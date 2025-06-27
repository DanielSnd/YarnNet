#include "ynet_socketio.h"

void YNetSocketIO::_bind_methods() {
}

YNetSocketIO::YNetSocketIO() {
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
        client = nullptr;
    }
    
    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }
    clear_unhandled_packets();
    connections_map.clear();
}

void YNetSocketIO::socketio_connect(String name_space) {
    socketio_send_packet_text(CONNECT, name_space);
}

void YNetSocketIO::socketio_disconnect(String name_space) {
    socketio_send_packet_text(DISCONNECT, name_space);
    YNet::get_singleton()->emit_signal(SNAME("disconnected"),name_space);
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
    regex_match = nullptr;


    regex->compile("(\\w),");
    regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        name_space = regex_match->get_string(1);
    }
    regex_match = nullptr;

    regex->compile("(\\d+)");
    regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        WARN_PRINT("[YNet] Ignoring acknowledgement ID "+regex_match->get_string(1));
    }
    regex_match = nullptr;

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
                YNet::get_singleton()->sid = sid;
                YNet::get_singleton()->hashed_sid = static_cast<int>(string_to_hash_id(sid));
                YNet::get_singleton()->real_hashed_sid = YNet::get_singleton()->hashed_sid;
                YNet::get_singleton()->transport_connected_successfully();
            } else {
                YNet::get_singleton()->emit_signal(SNAME("connection_result"),false);
                socketio_disconnect();
            }
        }
            break;
        case SocketIOPacketType::CONNECT_ERROR:
            {YNet::get_singleton()->emit_signal(SNAME("connection_result"),false);}
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
                YNet::get_singleton()->on_host_migrated(event_payload);
#endif
            } else if (event_hash == roomcreated_event) {
                YNet::get_singleton()->on_room_created(event_payload);
            } else if (event_hash == roomjoined_event) {
                YNet::get_singleton()->on_room_joined(array[0], array[1]);
            } else if (event_hash == roomplayers_event) {
                YNet::get_singleton()->on_room_players(array[0]);
            } else if (event_hash == roomerror_event) {
                YNet::get_singleton()->on_room_error(event_payload);
            } else if (event_hash == leftroom) {
                YNet::get_singleton()->on_left_room();
            } else if (event_hash == roominfo) {
                YNet::get_singleton()->on_room_info(event_payload);
            } else if (event_hash == roomlist) {
                YNet::get_singleton()->on_room_list(event_payload);
            } else if (event_hash == playerjoin_event) {
                YNet::get_singleton()->on_player_join(event_payload);
            } else if (event_hash == playerleft_event) {
                YNet::get_singleton()->on_player_left(event_payload);
            } else if (event_hash == pkt) {
                String received_from = array[0];
                String pkt_content = array[1];
                if(debugging > YNet::MOSTMESSAGES)
                    print_line("on_received_pkt from ",received_from," content: ",pkt_content);

                int strlen = pkt_content.length();
                CharString cstr = pkt_content.ascii();

                size_t arr_len = 0;
                Vector<uint8_t> buf;
                {
                    buf.resize(strlen / 4 * 3 + 1);
                    uint8_t *w = buf.ptrw();
                    Error result = CryptoCore::b64_decode(&w[0], buf.size(), &arr_len, (unsigned char *)cstr.get_data(), strlen);
                    if (result != OK) {
                        ERR_PRINT(vformat("Failed to decode packet %s",pkt_content));
                        return false;
                    }
                }
                buf.resize(arr_len);

                YNetTypes::Packet packet;
                packet.data = (uint8_t *)memalloc(arr_len);
                memcpy(packet.data, buf.ptrw(), arr_len);
                packet.size = arr_len;
                packet.source = string_to_hash_id(received_from);
                
                unhandled_packets.push_back(packet);
            }
            // YNet::get_singleton()->emit_signal(SNAME("event"),event_name,event_payload,name_space);
            if(debugging == YNet::ALL) {
                print_line(vformat("[YNet %s] Socket IO Event Received = %s = data: = %s =",sid,event_name,event_payload));
            }
        }
            break;
        case SocketIOPacketType::DISCONNECT:
            //{YNet::get_singleton()->emit_signal(SNAME("disconnected"),name_space);}
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

int YNetSocketIO::get_max_packet_size() const {
    return 65535; // ENet default max packet size
}

void YNetSocketIO::transport_process(YNet* ynet) {
    update_last_engine_state();

    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout=true;
            YNet::get_singleton()->last_error_message = "TIMEOUT";
            YNet::get_singleton()->emit_signal(SNAME("connection_result"),false);
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
        YNet::get_singleton()->emit_signal(SNAME("disconnected"),client->get_close_code(),client->get_close_reason());
        if (status == STATE_OPEN) {
            //emit_signal("disconnected",slash_namespace);
        }
        set_current_state(State::STATE_CLOSED);
        if (debugging == YNet::ALL) {
            print_line("[YNet] Closed connection, stoppping process");
        }
        YNet::get_singleton()->set_process(false);
    }
}

YNetSocketIO::State YNetSocketIO::get_current_state() const {
    if (YNet::get_singleton()->get_offline_mode()) {
        return STATE_OPEN;
    }
    return status;
}

void YNetSocketIO::set_current_state(State val) {
        if (status != val) {
            status = val;
            if (status == STATE_CONNECTING) {
                tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
            }
            if (status != STATE_OPEN) {
                YNet::get_singleton()->room_id = "";
            }
            if (YNet::get_singleton()->get_debugging() >= 2) {
                switch ((State)status) {
                    case STATE_CONNECTING:
                        print_line("[YNet] Status is now: Connecting");
                        break;
                    case STATE_OPEN:
                        print_line("[YNet] Status is now: Connected");
                        break;
                    case STATE_CLOSING:
                        print_line("[YNet] Status is now: Closing");
                        break;
                    case STATE_CLOSED:
                        print_line("[YNet] Status is now: Closed");
                        break;
                    default: ;
                }
            }
        YNet::get_singleton()->emit_signal(SNAME("status_changed"),val);
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
            transport_disconnect();
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

    set_current_state(State::STATE_CONNECTING);

    tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
    Error err = client->connect_to_url(url);

    if (err != OK) {
        set_current_state(State::STATE_CLOSED);
        YNet::get_singleton()->last_error_message = vformat("CONNECTION ERROR %d", err);
        print_line("ERROR! ", YNet::get_singleton()->last_error_message); 
        YNet::get_singleton()->emit_signal(SNAME("connection_result"),false);
        return err;
    }
    
    if(debugging == YNet::ALL) print_line("[YNet] Connecting to url: ", url);

    YNet::get_singleton()->set_process(true);

        return err;
}

void YNetSocketIO::transport_disconnect() {
    if (client.is_valid()) {
        engineio_send_packet_charstring(&close_cs);
        client->close();
        status = STATE_CLOSED;
        was_timeout = false;
        update_last_engine_state();
    }
}

Error YNetSocketIO::send_packet(const uint8_t *p_buffer, int p_buffer_size) {
    ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(get_state() != STATE_OPEN, ERR_UNCONFIGURED);
    auto stringified_packet = CryptoCore::b64_encode_str(p_buffer,p_buffer_size);
    
    if (YNet::get_singleton()->is_host) {
        if (target_peer.is_empty()) {
            Array packet_data;
            packet_data.append(YNet::get_singleton()->room_id);
            packet_data.append(stringified_packet);
            socketio_send("pkt2clients",packet_data);
        } else {
            Array packet_data;
            packet_data.append(YNet::get_singleton()->room_id);
            packet_data.append(target_peer);
            packet_data.append(stringified_packet);
            socketio_send("pkt2cid",packet_data);
        }
        return OK;
    }
    //I'M CLIENT
    Array packet_data;
    packet_data.append(YNet::get_singleton()->room_id);
    packet_data.append(stringified_packet);
    socketio_send("pkt2serv",packet_data);
    return OK;
}

void YNetSocketIO::kick_peer(String p_peer, bool p_force) {
    if (client.is_valid()) {
        Array payload;
        payload.push_back(YNet::get_singleton()->room_id);
        payload.push_back(p_peer);
        socketio_send("kickid",payload);
    }
}

YNetTransport::State YNetSocketIO::get_state() const {
    if (!client.is_valid()) {
        return STATE_CLOSED;
    }
    return static_cast<State>(client->get_ready_state());
}

bool YNetSocketIO::has_packet() const {
    ERR_FAIL_COND_V(!client.is_valid(), false);
    return !unhandled_packets.is_empty();
}

int YNetSocketIO::get_available_packet_count() const {
    if (!client.is_valid()) {
        return 0;
    }
    return unhandled_packets.size();
}

int YNetSocketIO::get_packet_peer() const {
    if (!client.is_valid()) {
        return 1;
    }
    ERR_FAIL_COND_V_MSG(unhandled_packets.size() == 0, 1, "No packets to get!");
    auto source = unhandled_packets.front()->get().source;
    if (source == YNet::get_singleton()->host_id_hashed)
        source = 1;
    return source;
}

Error YNetSocketIO::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
    ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);

    r_buffer_size = 0;

    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }

    ERR_FAIL_COND_V(unhandled_packets.is_empty(), ERR_UNAVAILABLE);

    current_packet = unhandled_packets.front()->get();
    unhandled_packets.pop_front();

    *r_buffer = current_packet.data;
    r_buffer_size = current_packet.size;
    
    return OK;
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
    String packet_payload = String::utf8((const char *)packet, len);
    auto packetType = packet_payload.substr(0,1).to_int();
    packet_payload = packet_payload.substr(1);
    switch (packetType) {
        case EngineIOPacketType::OPEN: {
            if(debugging>1) print_line("[YNet] Received engine.IO open package");
            JSON json;
            ERR_FAIL_COND_V_MSG(json.parse(packet_payload) != OK, false, "[YNet] Malformed open message!");
            Dictionary params = json.get_data();
            ERR_FAIL_COND_V_MSG(!params.has(key_sid) || !params.has(key_pingTimeout) || !params.has(key_pingInterval), false, "[YNet] Open message was missing one of the variables needed.");
            sid = params.get(key_sid,key_sid);
            YNet::get_singleton()->sid = sid;
            YNet::get_singleton()->hashed_sid = static_cast<int>(string_to_hash_id(sid));
            YNet::get_singleton()->real_hashed_sid = YNet::get_singleton()->hashed_sid;
            pingInterval = params.get(key_pingInterval,500);
            pingTimeout = params.get(key_pingTimeout,3000);
            set_current_state(State::STATE_OPEN);
            if(debugging>1) print_line("[YNet] Connected to engine.io, SID ",sid," hashed ",YNet::get_singleton()->hashed_sid," ping interval ",pingInterval, " ping timeout ",pingTimeout);
            //(SNAME("engine_connection_result"),sid,true);
            socketio_connect();
        }
            break;
        case EngineIOPacketType::CLOSE: {
            if(debugging>1) print_line("[YNet] Received engine.IO close package");
        }
            break;
        case EngineIOPacketType::PING: {
            if(debugging>1) print_line("[YNet] Received engine.IO ping package");
            engineio_send_packet_charstring(&pong_cs);
        }
            break;
        case EngineIOPacketType::PONG: {
            if(debugging>1) print_line("[YNet] Received engine.IO pong package");
        }
            break;
        case EngineIOPacketType::MESSAGE: {
            if(debugging>2) print_line("[YNet] Received engine.IO message package",packet_payload);
            socketio_parse_packet(packet_payload);
        }
            break;
        case EngineIOPacketType::UPGRADE: {
            if(debugging>1) print_line("[YNet] Received engine.IO upgrade package");
        }
            break;
        case EngineIOPacketType::NOOP: {
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
    const CharString cs = vformat("%d%s",EngineIOPacketType::MESSAGE,_payload).utf8();
    //WARN_PRINT(vformat("Sending message %s",cs));
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNetSocketIO::socketio_send(String event_name, Variant data, String name_space) {
    if (data.get_type() == Variant::Type::ARRAY) {
        Array payload = data;
        payload.push_front(event_name);
        return socketio_send_packet_text(EVENT, payload, name_space);
    }
    Array payload;
    payload.append(event_name);
    if (data.get_type() != Variant::Type::NIL)
        payload.append(data);
    return socketio_send_packet_text(EVENT, payload, name_space);
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

void YNetSocketIO::set_max_players(int max_players) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(max_players);
    socketio_send("setmaxplayers", payload);
}

void YNetSocketIO::get_room_info(const String &room_code) {
    Array payload;
    payload.push_back(room_code);
    socketio_send("getroominfo", payload);
}

void YNetSocketIO::leave_room() {
    socketio_send("leaveroom");
    clear_unhandled_packets();
    YNet::get_singleton()->room_id = "";
}

void YNetSocketIO::set_can_host_migrate(bool can_migrate) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(can_migrate);
    socketio_send("sethostmigration", payload);
}

void YNetSocketIO::create_room_with_code(const String &room_code, const String &password) {
    Array payload;
    payload.push_back(room_code);
    payload.push_back(YNet::get_singleton()->protocol);
    socketio_send("createroomwithcode", payload);
}

void YNetSocketIO::set_password(const String &password) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(password);
    socketio_send("setpassword", payload);
}

void YNetSocketIO::set_extra_info(const String &extra_info) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(extra_info);
    socketio_send("setextrainfo", payload);
}

void YNetSocketIO::join_room(const String &join_room, const String &password) {
    if (!password.is_empty()) {
        Array payload;
        payload.push_back(join_room);
        payload.push_back(password);
        socketio_send("joinroomwithpassword", payload);
    } else {
        socketio_send("joinroom",join_room + YNet::get_singleton()->protocol);
    }
}

void YNetSocketIO::set_private(bool is_private) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(is_private);
    socketio_send("setprivate", payload);
}

void YNetSocketIO::create_room() {
    socketio_send("requestroom", YNet::get_singleton()->protocol);
}

void YNetSocketIO::get_room_list() {
    socketio_send("getroomlist");
}

void YNetSocketIO::set_room_name(const String &room_name) {
    Array payload;
    payload.push_back(YNet::get_singleton()->room_id);
    payload.push_back(room_name);
    socketio_send("setroomname", payload);
}

void YNetSocketIO::join_or_create_room(const String &join_room, const String &password) {
    if (join_room.is_empty()) {
        socketio_send("joinOrCreateRoomRandom", YNet::get_singleton()->protocol);
    } else {
        socketio_send("joinOrCreateRoom",join_room + YNet::get_singleton()->protocol);
    }
} 