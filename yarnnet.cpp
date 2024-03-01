#include "yarnnet.h"

#include "core/config/project_settings.h"
#include "core/crypto/crypto_core.h"
#include "core/io/marshalls.h"
#include "core/object/script_language.h"

void YNet::_bind_methods() {
    BIND_ENUM_CONSTANT(open);
    BIND_ENUM_CONSTANT(close);
    BIND_ENUM_CONSTANT(ping);
    BIND_ENUM_CONSTANT(pong);
    BIND_ENUM_CONSTANT(message);
    BIND_ENUM_CONSTANT(upgrade);
    BIND_ENUM_CONSTANT(noop);
    BIND_ENUM_CONSTANT(CONNECT);
    BIND_ENUM_CONSTANT(DISCONNECT);
    BIND_ENUM_CONSTANT(EVENT);
    BIND_ENUM_CONSTANT(ACK);
    BIND_ENUM_CONSTANT(CONNECT_ERROR);
    BIND_ENUM_CONSTANT(BINARY_EVENT);
    BIND_ENUM_CONSTANT(BINARY_ACK);

    BIND_ENUM_CONSTANT(NONE);
    BIND_ENUM_CONSTANT(MOSTMESSAGES);
    BIND_ENUM_CONSTANT(MESSAGESANDPING);
    BIND_ENUM_CONSTANT(ALL);

    ADD_SIGNAL(MethodInfo("room_connection_result", PropertyInfo(Variant::STRING, "room_id"),PropertyInfo(Variant::BOOL, "result")));
    ADD_SIGNAL(MethodInfo("room_connected", PropertyInfo(Variant::INT, "id")));
    ADD_SIGNAL(MethodInfo("room_disconnected", PropertyInfo(Variant::INT, "id")));
    //ADD_SIGNAL(MethodInfo("engine_status_changed", PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("status_changed", PropertyInfo(Variant::INT, "status")));
    //ADD_SIGNAL(MethodInfo("engine_connection_result", PropertyInfo(Variant::STRING, "sid"), PropertyInfo(Variant::BOOL, "result")));
    ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::INT, "code"),PropertyInfo(Variant::STRING, "reason")));
    ADD_SIGNAL(MethodInfo("engine_message", PropertyInfo(Variant::STRING, "payload")));
    ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "new_room_id")));
    ADD_SIGNAL(MethodInfo("room_players", PropertyInfo(Variant::ARRAY, "players")));
    ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "new_room_id"), PropertyInfo(Variant::STRING, "new_room_host_id")));
    ADD_SIGNAL(MethodInfo("room_error", PropertyInfo(Variant::STRING, "returned_error")));
    ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::STRING, "player_sid")));
    ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::STRING, "player_sid")));
    ADD_SIGNAL(MethodInfo("host_migration", PropertyInfo(Variant::STRING, "new_host_sid")));

    ADD_SIGNAL(MethodInfo("connected", PropertyInfo(Variant::STRING, "name_space"), PropertyInfo(Variant::BOOL, "result")));
    //ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::STRING, "name_space")));
    ADD_SIGNAL(MethodInfo("event", PropertyInfo(Variant::STRING, "event_name"),PropertyInfo(Variant::OBJECT, "payload"),PropertyInfo(Variant::STRING, "name_space")));

    ClassDB::bind_method(D_METHOD("setup_node"), &YNet::setup_node);
    ClassDB::bind_method(D_METHOD("ynet_connect", "url"), &YNet::engineio_connect);
    ClassDB::bind_method(D_METHOD("ynet_disconnect"), &YNet::engineio_disconnect);

    ClassDB::bind_method(D_METHOD("string_to_hash_id","str"), &YNet::string_to_hash_id);

    ClassDB::bind_method(D_METHOD("socketio_send_event", "event_name","data","name_space"), &YNet::socketio_send,DEFVAL(Variant{}),DEFVAL(""));
    ClassDB::bind_method(D_METHOD("socketio_connect", "name_space"), &YNet::socketio_connect,DEFVAL(slash_namespace));
    ClassDB::bind_method(D_METHOD("socketio_disconnect", "name_space"), &YNet::socketio_disconnect,DEFVAL(slash_namespace));
    // Error socketio_send(String &event_name, Variant data = {} , String name_space = slash_namespace);
    // void socketio_connect(String name_space = slash_namespace);
    // void socketio_disconnect(String name_space = slash_namespace);

    ClassDB::bind_method(D_METHOD("set_debugging", "debugging_level"), &YNet::set_debugging);
    ClassDB::bind_method(D_METHOD("get_debugging"), &YNet::get_debugging);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "debugging", PROPERTY_HINT_ENUM, "None,MostMessages,MesssagesAndPings,All"), "set_debugging", "get_debugging");

    ClassDB::bind_method(D_METHOD("set_is_host", "status"), &YNet::set_is_host);
    ClassDB::bind_method(D_METHOD("get_is_host"), &YNet::get_is_host);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_host"), "set_is_host", "get_is_host");

    ClassDB::bind_method(D_METHOD("set_offline_mode", "status"), &YNet::set_offline_mode);
    ClassDB::bind_method(D_METHOD("get_offline_mode"), &YNet::get_offline_mode);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "offline_mode"), "set_offline_mode", "get_offline_mode");

    ClassDB::bind_method(D_METHOD("set_hashed_socket_id", "status"), &YNet::set_hashed_sid);
    ClassDB::bind_method(D_METHOD("get_hashed_socket_id"), &YNet::get_hashed_sid);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hashed_socket_id"), "set_hashed_socket_id", "get_hashed_socket_id");

    ClassDB::bind_method(D_METHOD("set_real_hashed_socket_id", "status"), &YNet::set_real_hashed_sid);
    ClassDB::bind_method(D_METHOD("get_real_hashed_socket_id"), &YNet::get_real_hashed_sid);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "real_hashed_socket_id"), "set_real_hashed_socket_id", "get_real_hashed_socket_id");

    ClassDB::bind_method(D_METHOD("set_last_used_id", "id"), &YNet::set_last_used_id);
    ClassDB::bind_method(D_METHOD("get_last_used_id"), &YNet::get_last_used_id);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "last_used_id"), "set_last_used_id", "get_last_used_id");

    ClassDB::bind_method(D_METHOD("set_socket_id", "id"), &YNet::set_sid);
    ClassDB::bind_method(D_METHOD("get_socket_id"), &YNet::get_sid);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "socket_id"), "set_socket_id", "get_socket_id");

    ClassDB::bind_method(D_METHOD("set_room_id", "id"), &YNet::set_room_id);
    ClassDB::bind_method(D_METHOD("get_room_id"), &YNet::get_room_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "room_id"), "set_room_id", "get_room_id");

    ClassDB::bind_method(D_METHOD("set_host_id", "id"), &YNet::set_host_id);
    ClassDB::bind_method(D_METHOD("get_host_id"), &YNet::get_host_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_id"), "set_host_id", "get_host_id");

    ClassDB::bind_method(D_METHOD("set_room_id_without_protocol", "id"), &YNet::set_room_id_without_protocol);
    ClassDB::bind_method(D_METHOD("get_room_id_without_protocol"), &YNet::get_room_id_without_protocol);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "room_id_without_protocol"), "set_room_id_without_protocol", "get_room_id_without_protocol");

    ClassDB::bind_method(D_METHOD("set_protocol", "protocol"), &YNet::set_protocol);
    ClassDB::bind_method(D_METHOD("get_protocol"), &YNet::get_protocol);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "protocol"), "set_protocol", "get_protocol");

    ClassDB::bind_method(D_METHOD("get_new_network_id"), &YNet::get_new_network_id);

    ClassDB::bind_method(D_METHOD("create_room"), &YNet::create_room);
    ClassDB::bind_method(D_METHOD("join_or_create_room", "roomcode"), &YNet::join_or_create_room);
    ClassDB::bind_method(D_METHOD("join_room", "roomcode"), &YNet::join_room);
    ClassDB::bind_method(D_METHOD("leave_room"), &YNet::leave_room);
}

void YNet::setup_node() {
    add_setting("YNet/settings/enabled", false, Variant::Type::BOOL);
    add_setting("YNet/settings/protocol", "change_me", Variant::Type::STRING);
    // //r_options->push_back(ImportOption(PropertyInfo(Variant::ARRAY, "fallbacks", PROPERTY_HINT_ARRAY_TYPE, PROPERTY_HINT_NODE_TYPE), Array()));
    // add_setting("YNet/settings/networked_nodes", TypedArray<NodePath>(), Variant::Type::ARRAY, PROPERTY_HINT_ARRAY_TYPE,
    //         vformat("%s/%s:",Variant::Type::STRING, PROPERTY_HINT_FILE));

    if(!already_setup_in_tree && SceneTree::get_singleton() != nullptr) {
        bool is_enabled = GLOBAL_GET("YNet/settings/enabled");
        if(!is_enabled) return;
        SceneTree::get_singleton()->get_root()->add_child(this);
        set_name("YNet");
        already_setup_in_tree=true;
    }

    protocol = GLOBAL_GET("YNet/settings/protocol");
}
YNet* YNet::singleton = nullptr;

void YNet::engineio_disconnect() {
    if(client.is_valid()) {
        clear_unhandled_packets();
        connections_map.clear();
        client->close();
        emit_signal(SNAME("disconnected"),client->get_close_code(),was_timeout ? "Timed out" : client->get_close_reason());
        was_timeout=false;
        if (status == STATE_OPEN || status == STATE_CONNECTING) {
            //emit_signal(SNAME("disconnected"),slash_namespace);
            set_current_state(STATE_CLOSED);
        }
        update_last_engine_state();
        set_process(false);
    }
}

Error YNet::engineio_connect(String _url) {
    if(protocol == "change_me") {
        WARN_PRINT("[YNet] Make sure to change the protocol string in YNet settings");
    }
    if (!_url.ends_with("/"))
        _url = _url+"/socket.io/";
    else {
        _url += "socket.io/";
    }
    if (_url.begins_with("https"))
        _url = "wss"+_url.erase(0,5);
    else if (_url.begins_with("http"))
        _url = "ws"+_url.erase(0,4);
    url = vformat("%s?EIO=4&transport=websocket",_url);
    if(debugging == ALL)
        print_line("[YNet] Connecting... url in ",_url," url out ",url);

    if (!client.is_valid()) {
        YNet::create_client();
        if(debugging == ALL)
            print_line("[YNet] WebSocket created");
    } else {
        if (client->get_ready_state() != STATE_CLOSED) {
            engineio_disconnect();
            ERR_PRINT("[YNet] Called connect when already connected");
            return ERR_BUG;
        }
    }

    ERR_FAIL_COND_V(client.is_null(), ERR_BUG);

    Vector<String> protocols;
    protocols.push_back("binary"); // Compatibility for emscripten TCP-to-WebSocket.
    client->set_supported_protocols(protocols);
    client->set_max_queued_packets(max_queued_packets);
    client->set_inbound_buffer_size(DEFAULT_BUFFER_SIZE);
    client->set_outbound_buffer_size(DEFAULT_BUFFER_SIZE);

    set_current_state(State::STATE_CONNECTING);
    Error err = client->connect_to_url(url);

    if (err != OK) {
        set_current_state(State::STATE_CLOSED);
        print_line("ERROR! ",err);
    }

    ERR_FAIL_COND_V(err != OK, err);
    set_process(true);
    return err;
}

void YNet::create_client() {
    if (!client.is_valid()) {
        client = Ref<WebSocketPeer>(Object::cast_to<WebSocketPeer>(ClassDB::instantiate("WebSocketPeer")));
    }
}
void YNet::set_max_queued_packets(int p_max_queued_packets) {
    client->set_max_queued_packets(p_max_queued_packets);
}

void YNet::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
        } break;
        // case NOTIFICATION_ENTER_WORLD: {
        // } break;
        case NOTIFICATION_EXIT_TREE: {
            if(status == STATE_OPEN) {
                if(!offline_mode) {
                    engineio_send_packet_charstring(&close_cs);
                    client->close();
                    set_current_state(STATE_CLOSED);
                }
            }
            break;
        }
        case NOTIFICATION_PARENTED: {
            if (Engine::get_singleton()->is_editor_hint()) {
            }
            break;
        }
        case NOTIFICATION_WM_CLOSE_REQUEST: {
            if(status == STATE_OPEN) {
                if(!offline_mode) {
                    engineio_send_packet_charstring(&close_cs);
                    client->close();
                    set_current_state(STATE_CLOSED);
                }
            }
        }break;
        case NOTIFICATION_READY: {
        } break;
        case NOTIFICATION_PROCESS: {
            if(client.is_valid()) {
                do_process();
            }
        } break;
        default:
            break;
    }
}

bool YNet::process_packets() {
    if(offline_mode) {
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

bool YNet::engineio_decode_packet(const uint8_t* packet, int len){
    String packet_payload;
    packet_payload.parse_utf8((const char *)packet, len);
    auto packetType = packet_payload.substr(0,1).to_int();
    packet_payload = packet_payload.substr(1);
    switch (packetType) {
        case open: {
            if(debugging>0) print_line("[YNet] Received engine.IO open package");
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
            if(debugging>0) print_line("[YNet] Connected to engine.io, SID ",sid," hashed ",hashed_sid," ping interval ",pingInterval, " ping timeout ",pingTimeout);
            //(SNAME("engine_connection_result"),sid,true);
            socketio_connect();
        }
            break;
        case close: {
            if(debugging>0) print_line("[YNet] Received engine.IO close package");
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
            if(debugging>0) print_line("[YNet] Received engine.IO upgrade package");
        }
            break;
        case noop: {
            if(debugging>0) print_line("[YNet] Received noop message");
        }
            break;
        default:
            break;
    }
    return true;
}

Error YNet::engineio_send_packet_charstring(const CharString *cs) {
    return client->send((const uint8_t *)cs->ptr(), cs->length(), WebSocketPeer::WRITE_MODE_TEXT);
}
Error YNet::engineio_send_packet_type(const EngineIOPacketType packet_type) {
    const CharString cs = vformat("%d",packet_type).utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNet::engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    return client->send(p_message.ptr(), p_message.size(), WebSocketPeer::WRITE_MODE_BINARY);
}

Error YNet::engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text) {
    if(p_text.is_empty()) {
        return engineio_send_packet_type(packet_type);
    }
    p_text = vformat("%d%s",packet_type,p_text);
    const CharString cs = p_text.utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YNet::socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    WARN_PRINT("BINARY SENDING NOT IMPLEMENTED");
    return ERR_BUG;
}

Error YNet::socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text, String name_space) {
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

Error YNet::socketio_send(String event_name, Variant _data, String name_space) {
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

void YNet::socketio_connect(String name_space) {
    socketio_send_packet_text(CONNECT, name_space);
}

void YNet::socketio_disconnect(String name_space) {
    socketio_send_packet_text(DISCONNECT, name_space);
    emit_signal(SNAME("disconnect"),name_space);
}

bool YNet::socketio_parse_packet(String& payload) {
    auto packetType = (EngineIOPacketType)payload.substr(0,1).to_int();
    payload = payload.substr(1);
    if(debugging == ALL)
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
    if(debugging == ALL)
        print_line("[YNet] Received socketio event ",packetType," data parsed: ",_data," payload parsed: ",payload);
    switch (packetType) {
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
                on_host_migrated(event_payload);
            } else if (event_hash == roomcreated_event) {
                on_room_created(event_payload);
            } else if (event_hash == roomjoined_event) {
                on_room_joined(array[0], array[1]);
            } else if (event_hash == roomplayers_event) {
                on_room_players(array[0]);
            } else if (event_hash == roomerror_event) {
                on_room_error(event_payload);
            }else if (event_hash == leftroom) {
                on_left_room();
            } else if (event_hash == playerjoin_event) {
                on_player_join(event_payload);
            } else if (event_hash == playerleft_event) {
                on_player_left(event_payload);
            } else if (event_hash == pkt) {
                on_received_pkt(array[0],array[1]);
            }
            emit_signal(SNAME("event"),event_name,event_payload,name_space);
            if(debugging >0) {
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

void YNet::update_last_engine_state() {
    if(offline_mode) {
        return;
    }
    if (last_engine_state != client->get_ready_state()) {
        last_engine_state = static_cast<State>(static_cast<int>(client->get_ready_state()));
        if(debugging >= 1) {
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

void YNet::do_process() {
    update_last_engine_state();
    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout=true;
            emit_signal(SNAME("connected"),"TIMEOUT",false);
            engineio_disconnect();
            return;
        }
    }
    client->poll();

    auto client_state = client->get_ready_state();

    if (client_state == STATE_OPEN) {
        process_packets();
    } else if(client_state == STATE_CLOSING) {
    } else if(client_state == STATE_CLOSED) {
        emit_signal(SNAME("disconnected"),client->get_close_code(),client->get_close_reason());
        if (status == STATE_OPEN) {
            //emit_signal("disconnected",slash_namespace);
        }
        set_current_state(STATE_CLOSED);
        set_process(false);
    }
}

int YNet::get_max_queued_packets() {
    if (!client.is_valid()) {
        YNet::create_client();
    }
    return client->get_max_queued_packets();
}

Error YNet::create_room() {
    socketio_send("requestroom",protocol);
    return OK;
}

Error YNet::join_or_create_room(const String &join_room) {
    socketio_send("joinOrCreateRoom",join_room);
    return OK;
}
Error YNet::join_room(const String &join_room) {
    socketio_send("joinroom",join_room);
    return OK;
}

Error YNet::leave_room() {
    socketio_send("leaveroom");
    clear_unhandled_packets();
    room_id = "";
    return OK;
}

void YNet::on_room_created(const String &p_new_room_id) {
    if(debugging > 0)
        print_line("room_created ",p_new_room_id);
    host_id = sid;
    host_id_hashed = string_to_hash_id(sid);
    is_host = host_id == sid;
    if (is_host) {
        hashed_sid = 1;
    }
    room_id = p_new_room_id;
    connections_map[sid] = 1;
    emit_signal(SNAME("room_created"),p_new_room_id);
    emit_signal(SNAME("room_connected"),hashed_sid);
    emit_signal(SNAME("room_connection_result"),p_new_room_id,true);
}

// ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "new_room_id")));
// ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "new_room_id")));
// ADD_SIGNAL(MethodInfo("room_error", PropertyInfo(Variant::STRING, "returned_error")));
// ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::STRING, "player_sid")));
// ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::STRING, "player_sid")));
// ADD_SIGNAL(MethodInfo("host_migration", PropertyInfo(Variant::STRING, "new_host_sid")));

void YNet::on_room_joined(const String &p_new_room_id,const String &p_host_id) {
    if(debugging > 0)
        print_line("on_room_joined ",p_new_room_id," host ",p_host_id," ",string_to_hash_id(p_host_id));
    host_id = p_host_id;
    host_id_hashed = string_to_hash_id(p_host_id);
    is_host = host_id == sid;
    if (is_host) {
        hashed_sid = 1;
    }
    room_id = p_new_room_id;
    connections_map[sid] = hashed_sid;
    emit_signal(SNAME("room_joined"),p_new_room_id,p_host_id);
    emit_signal(SNAME("room_connected"),hashed_sid);
    emit_signal(SNAME("room_connection_result"),p_new_room_id,true);
}

void YNet::on_room_error(const String &p_room_error) {
    if(debugging > 0)
        print_line("on_room_error ",p_room_error);
    emit_signal(SNAME("room_error"),p_room_error);
    emit_signal(SNAME("room_connection_result"),p_room_error,false);
    room_id = "";
}

void YNet::on_left_room() {
    if(debugging > 0)
        print_line("on_left_room ");
    emit_signal(SNAME("room_disconnected"),hashed_sid);
    room_id = "";
}

void YNet::on_room_players(const Array &players_array) {
    if(debugging > 0)
        print_line(vformat("on_room_players %s",players_array));
    for (int i = 0; i < players_array.size(); i++)
        if (const auto& _player = players_array[i]; _player.get_type() == Variant::STRING && !connections_map.has(_player))
            on_player_join(_player);

    emit_signal(SNAME("room_players"),players_array);
}

void YNet::on_player_join(const String &p_player) {
    if(debugging > 0)
        print_line("on_player_join ",p_player," ",string_to_hash_id(p_player));
    connections_map[p_player] = string_to_hash_id(p_player);
    emit_signal(SNAME("player_joined"),p_player);
}

void YNet::on_received_pkt(const String &received_from, const String &pkt_content) {
    if(debugging > 0)
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
            return;
        }
	}
	buf.resize(arr_len);

    Packet packet;
    packet.data = (uint8_t *)memalloc(arr_len);
    memcpy(packet.data, buf.ptrw(), arr_len);
    packet.size = arr_len;
    packet.source = connections_map.has(received_from) ? static_cast<int>(connections_map[received_from]) : static_cast<int>(string_to_hash_id(received_from));
    unhandled_packets.push_back(packet);
}

void YNet::on_player_left(const String &p_player) {
    if(debugging > 0)
        print_line("on_player_left ",p_player);
    if (connections_map.has(p_player))
        connections_map.erase(p_player);
    emit_signal(SNAME("player_left"),p_player);
}

void YNet::on_host_migrated(const String &p_new_host) {
    if(debugging > 0)
        print_line("on_host_migrated ",p_new_host);
    host_id = p_new_host;
    host_id_hashed = string_to_hash_id(p_new_host);
    is_host = host_id == sid;
    WARN_PRINT(vformat("HOST MIGRATION UNDERWAY NEW HOST IS %s I AM %s ... am I new host? %s",p_new_host,sid,host_id == sid));
    if (is_host) {
        hashed_sid = 1;
        connections_map[p_new_host] = 1;
    } else {
        connections_map[p_new_host] = 1;
    }
    emit_signal(SNAME("host_migration"),p_new_host);
}

YNet * YNet::get_singleton() {
    return singleton;
}

YNet::YNet() {
    singleton = this;
    max_queued_packets = 2048;
    call_deferred("setup_node");
}

void YNet::clear_unhandled_packets() {
    for (Packet &E : unhandled_packets) {
        memfree(E.data);
        E.data = nullptr;
    }
    unhandled_packets.clear();
}

YNet::~YNet() {
    clear_unhandled_packets();
    connections_map.clear();
    if (singleton != nullptr && singleton == this) {
        singleton = nullptr;
    }
    if (client.is_valid()) {
        client.unref();
    }
}

void YNet::add_setting(const String& p_name, const Variant& p_default_value, Variant::Type p_type,
PropertyHint p_hint, const String& p_hint_string, int p_usage,bool restart_if_changed) {
    if (!ProjectSettings::get_singleton()->has_setting(p_name)) {
        ProjectSettings::get_singleton()->set_setting(p_name, p_default_value);
    }
    ProjectSettings::get_singleton()->set_custom_property_info(PropertyInfo(p_type, p_name, p_hint, p_hint_string,p_usage));
    ProjectSettings::get_singleton()->set_initial_value(p_name, p_default_value);
    ProjectSettings::get_singleton()->set_restart_if_changed(p_name, restart_if_changed);
}
