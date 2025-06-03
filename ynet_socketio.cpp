#include "ynet_socketio.h"

void YNetSocketIO::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to", "url"), &YNetSocketIO::connect_to);
    ClassDB::bind_method(D_METHOD("disconnect"), &YNetSocketIO::disconnect);
    ClassDB::bind_method(D_METHOD("send_packet", "data", "size"), &YNetSocketIO::send_packet);
    ClassDB::bind_method(D_METHOD("poll"), &YNetSocketIO::poll);
    ClassDB::bind_method(D_METHOD("get_state"), &YNetSocketIO::get_state);
    ClassDB::bind_method(D_METHOD("has_packet"), &YNetSocketIO::has_packet);
    ClassDB::bind_method(D_METHOD("get_packet"), &YNetSocketIO::get_packet);
    ClassDB::bind_method(D_METHOD("set_max_queued_packets", "max_queued_packets"), &YNetSocketIO::set_max_queued_packets);
    ClassDB::bind_method(D_METHOD("get_max_queued_packets"), &YNetSocketIO::get_max_queued_packets);
    ClassDB::bind_method(D_METHOD("set_inbound_buffer_size", "size"), &YNetSocketIO::set_inbound_buffer_size);
    ClassDB::bind_method(D_METHOD("set_outbound_buffer_size", "size"), &YNetSocketIO::set_outbound_buffer_size);
    ClassDB::bind_method(D_METHOD("get_close_code"), &YNetSocketIO::get_close_code);
    ClassDB::bind_method(D_METHOD("get_close_reason"), &YNetSocketIO::get_close_reason);
    ClassDB::bind_method(D_METHOD("set_supported_protocols", "protocols"), &YNetSocketIO::set_supported_protocols);
    ClassDB::bind_method(D_METHOD("set_debugging", "debugging"), &YNetSocketIO::set_debugging);
    ClassDB::bind_method(D_METHOD("get_debugging"), &YNetSocketIO::get_debugging);
    ClassDB::bind_method(D_METHOD("get_sid"), &YNetSocketIO::get_sid);
    ClassDB::bind_method(D_METHOD("was_connection_timeout"), &YNetSocketIO::was_connection_timeout);

    // ClassDB::bind_method(D_METHOD("socketio_send_event", "event_name","data","name_space"), &YNetSocketIO::socketio_send,DEFVAL(Variant{}),DEFVAL(""));
    ClassDB::bind_method(D_METHOD("socketio_connect", "name_space"), &YNetSocketIO::socketio_connect,DEFVAL(slash_namespace));
    ClassDB::bind_method(D_METHOD("socketio_disconnect", "name_space"), &YNetSocketIO::socketio_disconnect,DEFVAL(slash_namespace));

}

YNetSocketIO::YNetSocketIO() {
    client = Ref<WebSocketPeer>(WebSocketPeer::create());
    status = STATE_CLOSED;
    last_engine_state = STATE_CLOSED;
    tick_started_connecting = 0.0;
    was_timeout = false;
    max_queued_packets = 2048;
    debugging = YNet::get_singleton()->debugging;
}


void YNetSocketIO::clear_unhandled_packets() {
    for (YNetTypes::Packet &E : unhandled_packets) {
        memfree(E.data);
        E.data = nullptr;
    }
    unhandled_packets.clear();
}

YNetSocketIO::~YNetSocketIO() {
    if (client.is_valid()) {
        client.unref();
    }
    
    clear_unhandled_packets();
    connections_map.clear();
}

void YNetSocketIO::socketio_connect(String name_space) {
    socketio_send_packet_text(CONNECT, name_space);
}

void YNetSocketIO::socketio_disconnect(String name_space) {
    socketio_send_packet_text(DISCONNECT, name_space);
    emit_signal(SNAME("disconnect"),name_space);
}

bool YNetSocketIO::socketio_parse_packet(String& payload) {
    auto packetType = (EngineIOPacketType)payload.substr(0,1).to_int();
    payload = payload.substr(1);
    if(debugging == YNet::ALL)
        print_line(vformat("[YNet %s] PAYLOAD RECEIVED ",sid),payload);
    auto name_space= slash_namespace;

    auto regex = RegEx::create_from_string("(\\d+)-");
    auto regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        ERR_PRINT("[YNet] Binary data payload not supported! "+regex_match->get_string(1));
    }
    regex_match.unref();


    regex->compile("(\\w),");
    regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        name_space = regex_match->get_string(1);
    }
    regex_match.unref();

    regex->compile("(\\d+)");
    regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        WARN_PRINT("[YNet] Ignoring acknowledgement ID "+regex_match->get_string(1));
    }
    regex_match.unref();

    Variant _data {};
    if(payload.length() > 0) {
        JSON json;
        ERR_FAIL_COND_V_MSG(json.parse(payload) != OK, false, vformat("[YNet] Malformed socketio event %d namespace: %s",packetType,name_space));
        _data = json.get_data();
    }
    if(debugging == YNet::ALL)
        print_line("[YNet] Received socketio event ",packetType," data parsed: ",_data," payload parsed: ",payload);
    switch (static_cast<SocketIOPacketType>(packetType)) {
        case SocketIOPacketType::CONNECT: {
            Dictionary data_dict = _data;
            if (data_dict.has("sid")) {
                //print_line("Before my sid was (set by engine io) ",sid," now it will be ",data_dict["sid"]);
                sid = data_dict["sid"];
                hashed_sid = static_cast<int>(string_to_hash_id(sid));
                real_hashed_sid = hashed_sid;
                emit_signal(SNAME("connected"),name_space,true);
            } else {
                emit_signal(SNAME("connected"),name_space,false);
                socketio_disconnect();
            }
        }
            break;
        case SocketIOPacketType::CONNECT_ERROR:
            {emit_signal(SNAME("connected"),name_space,false);}
            break;
        case SocketIOPacketType::EVENT: {
            ERR_FAIL_COND_V_MSG(_data.is_array() != true, false, vformat("[YNet] Invalid socketio event format %s",_data.to_json_string()));
            Array array = _data;
            const String event_name = array[0];
            const auto event_hash = event_name.hash();
            array.remove_at(0);
            Variant event_payload;
            if (array.size() == 1) {
                event_payload = array[0];
            } else if (array.size() == 0) {
                event_payload = Variant{};
            } else {
                event_payload = array;
            }
            if (event_hash == newhost_event) {
#ifdef HOST_MIGRATION
                on_host_migrated(event_payload);
#endif
            } else if (event_hash == roomcreated_event) {
                on_room_created(event_payload);
            } else if (event_hash == roomjoined_event) {
                on_room_joined(array[0], array[1]);
            } else if (event_hash == roomplayers_event) {
                on_room_players(array[0]);
            } else if (event_hash == roomerror_event) {
                on_room_error(event_payload);
            } else if (event_hash == leftroom) {
                on_left_room();
            } else if (event_hash == roominfo) {
                on_room_info(event_payload);
            } else if (event_hash == roomlist) {
                on_room_list(event_payload);
            } else if (event_hash == playerjoin_event) {
                on_player_join(event_payload);
            } else if (event_hash == playerleft_event) {
                on_player_left(event_payload);
            } else if (event_hash == pkt) {
                on_received_pkt(array[0],array[1]);
            }
            emit_signal(SNAME("event"),event_name,event_payload,name_space);
            if(debugging == DebuggingLevel::ALL) {
                print_line(vformat("[YNet %s] Socket IO Event Received = %s = data: = %s =",sid,event_name,event_payload));
            }
        }
            break;
        case SocketIOPacketType::DISCONNECT:
            //{emit_signal(SNAME("disconnected"),name_space);}
            break;
        case SocketIOPacketType::ACK:
            break;
        case SocketIOPacketType::BINARY_ACK:
            break;
        case SocketIOPacketType::BINARY_EVENT:
            break;
    }
    return true;
}

void YNetSocketIO::update_last_engine_state() {
    if(YNet::get_singleton()->get_offline_mode()) {
        return;
    }
    if (last_engine_state != static_cast<State>(client->get_ready_state())) {
        last_engine_state = static_cast<State>(static_cast<int>(client->get_ready_state()));
        if(debugging > 1) {
            switch ((State)last_engine_state) {
                case STATE_CONNECTING:
                    print_line("[YNet] Engine.io status is now: Connecting");
                    break;
                case STATE_OPEN:
                    print_line("[YNet] Engine.io status is now: Connected");
                    break;
                case STATE_CLOSING:
                    print_line("[YNet] Engine.io status is now: Closing");
                    break;
                case STATE_CLOSED:
                    print_line("[YNet] Engine.io status is now: Closed");
                    break;
                default: ;
            }
        }
    }
}

void YNetSocketIO::transport_exit_tree() {
    if(status == STATE_OPEN) {
        if(!YNet::get_singleton()->get_offline_mode()) {
            engineio_send_packet_charstring(&close_cs);
            client->close();
            status = STATE_CLOSED;
        }
    }
}

void YNetSocketIO::transport_app_close_request() {
    if(status == STATE_OPEN) {
        if(!YNet::get_singleton()->get_offline_mode()) {
            engineio_send_packet_charstring(&close_cs);
            client->close();
            status = STATE_CLOSED;
        }
    }
}

void YNetSocketIO::transport_process(YNet* ynet) {
    update_last_engine_state();

    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout=true;
            emit_signal(SNAME("connected"),"TIMEOUT",false);
            socketio_disconnect();
            return;
        }
    }
    client->poll();

    State client_state = static_cast<State>(client->get_ready_state());

    if (client_state == STATE_OPEN) {
        process_packets();
        YNet::get_singleton()->update_networked_property_syncers();
    } else if(client_state == STATE_CLOSING) {
    } else if(client_state == STATE_CLOSED) {
        emit_signal(SNAME("disconnected"),client->get_close_code(),client->get_close_reason());
        if (status == STATE_OPEN) {
            //emit_signal("disconnected",slash_namespace);
        }
        set_current_state(STATE_CLOSED);
        if (debugging == YNet::ALL) {
            print_line("[YNet] Closed connection, stoppping process");
        }
        YNet::get_singleton()->set_process(false);
    }
}

Error YNetSocketIO::connect_to(const String &p_url) {
    url = p_url;
    if (!p_url.ends_with("/")) {
        url = p_url + "/socket.io/";
    } else {
        url = p_url + "socket.io/";
    }

    if (url.begins_with("https")) {
        url = "wss" + url.erase(0, 5);
    } else if (url.begins_with("http")) {
        url = "ws" + url.erase(0, 4);
    }

    url = vformat("%s?EIO=4&transport=websocket", url);

    if (debugging == YNet::ALL) {
        print_line("[YNet] Connecting... url in ", p_url, " url out ", url, " valid websockets client? ", client.is_valid());
    }

    if (!client.is_valid()) {
        client = Ref<WebSocketPeer>(WebSocketPeer::create());
        if (debugging == YNet::ALL) {
            print_line("[YNet] WebSocket created? ", client.is_valid());
        }
    } else {
        if (static_cast<State>(client->get_ready_state()) != STATE_CLOSED) {
            disconnect();
            ERR_PRINT("[YNet] Called connect when already connected");
            return ERR_ALREADY_IN_USE;
        }
    }

    ERR_FAIL_COND_V(client.is_null(), ERR_UNCONFIGURED);

    Vector<String> protocols;
    protocols.push_back("binary");
    client->set_supported_protocols(protocols);
    client->set_max_queued_packets(max_queued_packets);
    client->set_inbound_buffer_size(DEFAULT_BUFFER_SIZE);
    client->set_outbound_buffer_size(DEFAULT_BUFFER_SIZE);

    status = STATE_CONNECTING;
    tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
    Error err = client->connect_to_url(url);

    if (err != OK) {
        status = STATE_CLOSED;
        print_line("ERROR! ", err);
    }

    return err;
}

void YNetSocketIO::disconnect() {
    if (client.is_valid()) {
        engineio_send_packet_charstring(&close_cs);
        client->close();
        status = STATE_CLOSED;
        was_timeout = false;
        update_last_engine_state();
    }
}

Error YNetSocketIO::send_packet(const uint8_t *p_data, int p_size) {
    ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);
    return client->send(p_data, p_size, WebSocketPeer::WRITE_MODE_BINARY);
}

Error YNetSocketIO::poll() {
    ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);
    client->poll();

    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout = true;
            disconnect();
            return ERR_TIMEOUT;
        }
    }

    State client_state = static_cast<State>(client->get_ready_state());

    if (client_state == STATE_OPEN) {
        process_packets();
    } else if (client_state == STATE_CLOSING) {
        // Handle closing state
    } else if (client_state == STATE_CLOSED) {
        status = STATE_CLOSED;
        if (debugging == YNet::ALL) {
            print_line("[YNet] Closed connection");
        }
    }

    return OK;
}

YNetTransport::State YNetSocketIO::get_state() const {
    if (!client.is_valid()) {
        return STATE_CLOSED;
    }
    return static_cast<State>(client->get_ready_state());
}

bool YNetSocketIO::has_packet() const {
    ERR_FAIL_COND_V(!client.is_valid(), false);
    return client->get_available_packet_count() > 0;
}

Error YNetSocketIO::get_packet(const uint8_t **r_packet, int &r_packet_size) {
    ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);
    return client->get_packet(r_packet, r_packet_size);
}

void YNetSocketIO::set_max_queued_packets(int p_max_queued_packets) {
    max_queued_packets = p_max_queued_packets;
    if (client.is_valid()) {
        client->set_max_queued_packets(p_max_queued_packets);
    }
}

int YNetSocketIO::get_max_queued_packets() const {
    if (!client.is_valid()) {
        return max_queued_packets;
    }
    return client->get_max_queued_packets();
}

void YNetSocketIO::set_inbound_buffer_size(int p_size) {
    if (client.is_valid()) {
        client->set_inbound_buffer_size(p_size);
    }
}

void YNetSocketIO::set_outbound_buffer_size(int p_size) {
    if (client.is_valid()) {
        client->set_outbound_buffer_size(p_size);
    }
}

int YNetSocketIO::get_close_code() const {
    if (!client.is_valid()) {
        return 0;
    }
    return client->get_close_code();
}

String YNetSocketIO::get_close_reason() const {
    if (!client.is_valid()) {
        return String();
    }
    return client->get_close_reason();
}

void YNetSocketIO::set_supported_protocols(const Vector<String> &p_protocols) {
    if (client.is_valid()) {
        client->set_supported_protocols(p_protocols);
    }
}

void YNetSocketIO::update_last_engine_state() {
    if (last_engine_state != static_cast<State>(client->get_ready_state())) {
        last_engine_state = static_cast<State>(static_cast<int>(client->get_ready_state()));
        if (debugging > 1) {
            switch ((State)last_engine_state) {
                case STATE_CONNECTING:
                    print_line("[YNet] Engine.io status is now: Connecting");
                    break;
                case STATE_OPEN:
                    print_line("[YNet] Engine.io status is now: Connected");
                    break;
                case STATE_CLOSING:
                    print_line("[YNet] Engine.io status is now: Closing");
                    break;
                case STATE_CLOSED:
                    print_line("[YNet] Engine.io status is now: Closed");
                    break;
                default:;
            }
        }
    }
}

bool YNetSocketIO::process_packets() {
    while (client->get_available_packet_count()) {
        const uint8_t *packet;
        int len;
        Error err = client->get_packet(&packet, len);
        ERR_FAIL_COND_V_MSG(err != OK, false, vformat("Error getting packet! %d", err));
        if (!engineio_decode_packet(packet, len)) {
            return false;
        }
    }
    return true;
}

bool YNetSocketIO::engineio_decode_packet(const uint8_t *p_packet, int p_size) {
    if (p_size == 0) {
        return true;
    }

    EngineIOPacketType packet_type = static_cast<EngineIOPacketType>(p_packet[0]);
    const uint8_t *data = p_packet + 1;
    int data_size = p_size - 1;

    switch (packet_type) {
        case OPEN: {
            if (data_size > 0) {
                String json_str;
                json_str.parse_utf8(reinterpret_cast<const char *>(data), data_size);
                JSON json;
                Error err = json.parse(json_str);
                if (err == OK) {
                    Dictionary dict = json.get_data();
                    if (dict.has("sid")) {
                        sid = dict["sid"];
                    }
                    if (dict.has("pingTimeout")) {
                        pingTimeout = dict["pingTimeout"];
                    }
                    if (dict.has("pingInterval")) {
                        pingInterval = dict["pingInterval"];
                    }
                }
            }
            status = STATE_OPEN;
            update_last_engine_state();
            break;
        }
        case CLOSE: {
            status = STATE_CLOSED;
            update_last_engine_state();
            break;
        }
        case PING: {
            engineio_send_packet_charstring(&pong_cs);
            break;
        }
        case PONG: {
            // Handle pong
            break;
        }
        case MESSAGE: {
            if (data_size > 0) {
                String message;
                message.parse_utf8(reinterpret_cast<const char *>(data), data_size);
                if (message.begins_with("0")) {
                    // Socket.IO packet
                    String packet_data = message.substr(1);
                    JSON json;
                    Error err = json.parse(packet_data);
                    if (err == OK) {
                        Array packet = json.get_data();
                        if (packet.size() > 0) {
                            SocketIOPacketType socket_packet_type = static_cast<SocketIOPacketType>(packet[0]);
                            switch (socket_packet_type) {
                                case CONNECT: {
                                    // Handle connect
                                    break;
                                }
                                case DISCONNECT: {
                                    // Handle disconnect
                                    break;
                                }
                                case EVENT: {
                                    if (packet.size() > 2) {
                                        String event_name = packet[1];
                                        Array args = packet[2];
                                        // Emit signal for event
                                        emit_signal("socketio_event", event_name, args);
                                    }
                                    break;
                                }
                                case ACK: {
                                    if (packet.size() > 2) {
                                        int ack_id = packet[1];
                                        Array args = packet[2];
                                        // Emit signal for ack
                                        emit_signal("socketio_ack", ack_id, args);
                                    }
                                    break;
                                }
                                case ERROR: {
                                    // Handle error
                                    break;
                                }
                                case BINARY_EVENT: {
                                    if (packet.size() > 2) {
                                        String event_name = packet[1];
                                        Array args = packet[2];
                                        // Emit signal for binary event
                                        emit_signal("socketio_binary_event", event_name, args);
                                    }
                                    break;
                                }
                                case BINARY_ACK: {
                                    if (packet.size() > 2) {
                                        int ack_id = packet[1];
                                        Array args = packet[2];
                                        // Emit signal for binary ack
                                        emit_signal("socketio_binary_ack", ack_id, args);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case UPGRADE: {
            // Handle upgrade
            break;
        }
        case NOOP: {
            // Handle noop
            break;
        }
    }

    return true;
}

void YNetSocketIO::socketio_send_ack(const String &p_event, const Array &p_args, int p_ack_id) {
    if (!client.is_valid() || status != STATE_OPEN) {
        return;
    }

    Array packet;
    packet.push_back(ACK);
    packet.push_back(p_ack_id);
    packet.push_back(p_args);

    JSON json;
    json.set_data(packet);
    String packet_str = "0" + json.stringify();
    client->send(packet_str.utf8(), packet_str.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

void YNetSocketIO::socketio_send_binary_ack(const String &p_event, const Array &p_args, int p_ack_id) {
    if (!client.is_valid() || status != STATE_OPEN) {
        return;
    }

    Array packet;
    packet.push_back(BINARY_ACK);
    packet.push_back(p_ack_id);
    packet.push_back(p_args);

    JSON json;
    json.set_data(packet);
    String packet_str = "0" + json.stringify();
    client->send(packet_str.utf8(), packet_str.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

bool YNetSocketIO::process_packets() {
    if(YNet::get_singleton()->get_offline_mode()) {
        return true;
    }
    while (client->get_available_packet_count()) {
        const uint8_t *packet;
        int len;
        last_get_error = client->get_packet(&packet, len);
        ERR_FAIL_COND_V_MSG(last_get_error != OK, last_get_error, vformat("Error getting packet! %d", last_get_error));
        if(!engineio_decode_packet(packet,len))
            return false;
    }
    return true;
}

bool YNetSocketIO::engineio_decode_packet(const uint8_t *packet, int len) {
    String packet_payload;
    packet_payload.parse_utf8((const char *)packet, len);
    auto packetType = packet_payload.substr(0,1).to_int();
    packet_payload = packet_payload.substr(1);
    switch (packetType) {
        case open: {
            if(debugging>1) print_line("[YNet] Received engine.IO open package");
            JSON json;
            ERR_FAIL_COND_V_MSG(json.parse(packet_payload) != OK, false, "[YNet] Malformed open message!");
            Dictionary params = json.get_data();
            ERR_FAIL_COND_V_MSG(!params.has(key_sid) || !params.has(key_pingTimeout) || !params.has(key_pingInterval), false, "[YNet] Open message was missing one of the variables needed.");
            sid = params.get(key_sid,key_sid);
            hashed_sid = string_to_hash_id(sid);
            real_hashed_sid = hashed_sid;
            pingInterval = params.get(key_pingInterval,500);
            pingTimeout = params.get(key_pingTimeout,3000);
            set_current_state(State::STATE_OPEN);
            if(debugging>1) print_line("[YNet] Connected to engine.io, SID ",sid," hashed ",hashed_sid," ping interval ",pingInterval, " ping timeout ",pingTimeout);
            //(SNAME("engine_connection_result"),sid,true);
            socketio_connect();
        }
            break;
        case close: {
            if(debugging>1) print_line("[YNet] Received engine.IO close package");
        }
            break;
        case ping: {
            if(debugging>1) print_line("[YNet] Received engine.IO ping package");
            engineio_send_packet_charstring(&pong_cs);
        }
            break;
        case pong: {
            if(debugging>1) print_line("[YNet] Received engine.IO pong package");
        }
            break;
        case message: {
            if(debugging>2) print_line("[YNet] Received engine.IO message package",packet_payload);
            socketio_parse_packet(packet_payload);
            emit_signal(SNAME("engine_message"));
        }
            break;
        case upgrade: {
            if(debugging>1) print_line("[YNet] Received engine.IO upgrade package");
        }
            break;
        case noop: {
            if(debugging>1) print_line("[YNet] Received noop message");
        }
            break;
        default:
            break;
    }
    return true;
}

Error YNetSocketIO::engineio_send_packet_charstring(const CharString *cs) {
    return client->send((const uint8_t *)cs->ptr(), cs->length(), WebSocketPeer::WRITE_MODE_TEXT);
}
Error YNetSocketIO::engineio_send_packet_type(const EngineIOPacketType packet_type) {
    const CharString cs = vformat("%d",packet_type).utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNetSocketIO::engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    return client->send(p_message.ptr(), p_message.size(), WebSocketPeer::WRITE_MODE_BINARY);
}

Error YNetSocketIO::engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text) {
    if(p_text.is_empty()) {
        return engineio_send_packet_type(packet_type);
    }
    p_text = vformat("%d%s",packet_type,p_text);
    const CharString cs = p_text.utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNetSocketIO::socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    WARN_PRINT("BINARY SENDING NOT IMPLEMENTED");
    return ERR_BUG;
}

Error YNetSocketIO::socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text, String name_space) {
    String _payload = vformat("%d",packet_type);
    if (!name_space.is_empty() && name_space != slash_namespace) {
        _payload += vformat("/%s,",name_space);
    }
    const auto varianttype = p_text.get_type();
    if (varianttype != Variant::Type::NIL) {
        if(varianttype == Variant::Type::ARRAY) {
            _payload += p_text.to_json_string();
        } else {
            _payload += p_text.stringify();
        }
    }
    const CharString cs = vformat("%d%s",EngineIOPacketType::message,_payload).utf8();
    //WARN_PRINT(vformat("Sending message %s",cs));
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNetSocketIO::socketio_send(String event_name, Variant _data, String name_space) {
    if (_data.get_type() == Variant::Type::ARRAY) {
        Array payload = _data;
        payload.push_front(event_name);
        return socketio_send_packet_text(EVENT,payload,name_space);
    }
    Array payload;
    payload.append(event_name);
    if (_data.get_type() != Variant::Type::NIL)
        payload.append(_data);
    return socketio_send_packet_text(EVENT,payload,name_space);
}

void YNetSocketIO::set_debugging(int p_debugging) {
    debugging = p_debugging;
}

int YNetSocketIO::get_debugging() const {
    return debugging;
}

String YNetSocketIO::get_sid() const {
    return sid;
}

bool YNetSocketIO::was_connection_timeout() const {
    return was_timeout;
} 