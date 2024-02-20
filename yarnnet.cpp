#include "yarnnet.h"

#include "core/config/project_settings.h"
#include "core/object/script_language.h"

void YarnNet::_bind_methods() {
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

    ADD_SIGNAL(MethodInfo("engine_status_changed", PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("status_changed", PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("engine_connected", PropertyInfo(Variant::STRING, "sid")));
    ADD_SIGNAL(MethodInfo("engine_disconnected", PropertyInfo(Variant::INT, "code"),PropertyInfo(Variant::STRING, "reason")));
    ADD_SIGNAL(MethodInfo("engine_message", PropertyInfo(Variant::STRING, "payload")));
    ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "new_room_id")));
    ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "new_room_id")));
    ADD_SIGNAL(MethodInfo("room_error", PropertyInfo(Variant::STRING, "returned_error")));
    ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::STRING, "player_sid")));
    ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::STRING, "player_sid")));
    ADD_SIGNAL(MethodInfo("host_migration", PropertyInfo(Variant::STRING, "new_host_sid")));

    ADD_SIGNAL(MethodInfo("connected", PropertyInfo(Variant::OBJECT, "payload"), PropertyInfo(Variant::STRING, "name_space"), PropertyInfo(Variant::BOOL, "error")));
    ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::STRING, "name_space")));
    ADD_SIGNAL(MethodInfo("event", PropertyInfo(Variant::STRING, "event_name"),PropertyInfo(Variant::OBJECT, "payload"),PropertyInfo(Variant::STRING, "name_space")));

    ADD_SIGNAL(MethodInfo("rpc_received", PropertyInfo(Variant::STRING, "sender"),PropertyInfo(Variant::INT, "netnode_id"),PropertyInfo(Variant::INT, "rpc_id"),PropertyInfo(Variant::OBJECT, "payload")));

    // void YarnNet::on_rpc_event(const String &p_sender, const int &p_netnodeid, const int &p_rpc_id, Variant &p_data) {
    ClassDB::bind_method(D_METHOD("setup_node"), &YarnNet::setup_node);
    ClassDB::bind_method(D_METHOD("ynet_connect", "url"), &YarnNet::engineio_connect);
    ClassDB::bind_method(D_METHOD("ynet_disconnect"), &YarnNet::engineio_disconnect);

    ClassDB::bind_method(D_METHOD("socketio_send_event", "event_name","data","name_space"), &YarnNet::socketio_send,DEFVAL(Variant{}),DEFVAL(""));
    ClassDB::bind_method(D_METHOD("socketio_connect", "name_space"), &YarnNet::socketio_connect,DEFVAL(slash_namespace));
    ClassDB::bind_method(D_METHOD("socketio_disconnect", "name_space"), &YarnNet::socketio_disconnect,DEFVAL(slash_namespace));
    // Error socketio_send(String &event_name, Variant data = {} , String name_space = slash_namespace);
    // void socketio_connect(String name_space = slash_namespace);
    // void socketio_disconnect(String name_space = slash_namespace);

    ClassDB::bind_method(D_METHOD("set_debugging", "debugging_level"), &YarnNet::set_debugging);
    ClassDB::bind_method(D_METHOD("get_debugging"), &YarnNet::get_debugging);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "debugging", PROPERTY_HINT_ENUM, "None,MostMessages,MesssagesAndPings,All"), "set_debugging", "get_debugging");

    ClassDB::bind_method(D_METHOD("set_offline_mode", "status"), &YarnNet::set_offline_mode);
    ClassDB::bind_method(D_METHOD("get_offline_mode"), &YarnNet::get_offline_mode);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "offline_mode"), "set_offline_mode", "get_offline_mode");

    ClassDB::bind_method(D_METHOD("set_last_used_id", "id"), &YarnNet::set_last_used_id);
    ClassDB::bind_method(D_METHOD("get_last_used_id"), &YarnNet::get_last_used_id);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "last_used_id"), "set_last_used_id", "get_last_used_id");

    ClassDB::bind_method(D_METHOD("get_new_network_id"), &YarnNet::get_new_network_id);

    ClassDB::bind_method(D_METHOD("create_room"), &YarnNet::create_room);
    ClassDB::bind_method(D_METHOD("join_or_create_room", "roomcode"), &YarnNet::join_or_create_room);
    ClassDB::bind_method(D_METHOD("join_room", "roomcode"), &YarnNet::join_room);
    ClassDB::bind_method(D_METHOD("leave_room"), &YarnNet::leave_room);

    ClassDB::bind_method(D_METHOD("get_rpc_configs","node"), &YarnNet::DebugRPCs);

}

void YarnNet::setup_node() {
    add_setting("YarnNet/settings/protocol", "change_me", Variant::Type::STRING);
    //r_options->push_back(ImportOption(PropertyInfo(Variant::ARRAY, "fallbacks", PROPERTY_HINT_ARRAY_TYPE, PROPERTY_HINT_NODE_TYPE), Array()));
    add_setting("YarnNet/settings/networked_nodes", TypedArray<NodePath>(), Variant::Type::ARRAY, PROPERTY_HINT_ARRAY_TYPE,
            vformat("%s/%s:",Variant::Type::STRING, PROPERTY_HINT_FILE));

    if(!already_setup_in_tree && SceneTree::get_singleton() != nullptr) {
        SceneTree::get_singleton()->get_root()->add_child(this);
        already_setup_in_tree=true;

        protocol = GLOBAL_GET("YarnNet/settings/protocol");
    }
}
YarnNet* YarnNet::singleton = nullptr;

Dictionary YarnNet::DebugRPCs(Node *node) {
    Ref<Script> scr = node->get_script();
    if(scr.is_valid()) {
        return scr->get_rpc_config();
    }
    return node->get_node_rpc_config();
}

void YarnNet::engineio_disconnect() {
    if(client.is_valid()) {
        client->close();
        emit_signal(SNAME("engine_disconnected"),client->get_close_code(),was_timeout ? "Timed out" : client->get_close_reason());
        was_timeout=false;
        if (status == STATE_OPEN || status == STATE_CONNECTING) {
            emit_signal(SNAME("disconnected"),slash_namespace);
            set_current_state(STATE_CLOSED);
        }
        update_last_engine_state();
        set_process(false);
    }
}

Error YarnNet::engineio_connect(String _url) {
    if(protocol == "change_me") {
        WARN_PRINT("[YarnNet] Make sure to change the protocol string in YarnNet settings");
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
        print_line("[YarnNet] Connecting... url in ",_url," url out ",url);

    if (!client.is_valid()) {
        YarnNet::create_client();
        if(debugging == ALL)
            print_line("[YarnNet] WebSocket created");
    } else {
        if (client->get_ready_state() != STATE_CLOSED) {
            engineio_disconnect();
            ERR_PRINT("[YarnNet] Called connect when already connected");
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

void YarnNet::create_client() {
    if (!client.is_valid()) {
        client = Ref<WebSocketPeer>(Object::cast_to<WebSocketPeer>(ClassDB::instantiate("WebSocketPeer")));
    }
}
void YarnNet::set_max_queued_packets(int p_max_queued_packets) {
    client->set_max_queued_packets(p_max_queued_packets);
}

void YarnNet::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
        } break;
        // case NOTIFICATION_ENTER_WORLD: {
        // } break;
        case NOTIFICATION_EXIT_TREE: {
            if(status == STATE_OPEN) {
                if(!offline_mode) {
                    engineio_send_packet_type(EngineIOPacketType::close);
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
                    engineio_send_packet_type(EngineIOPacketType::close);
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

bool YarnNet::process_packets() {
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

bool YarnNet::engineio_decode_packet(const uint8_t* packet, int len){
    String packet_payload;
    packet_payload.parse_utf8((const char *)packet, len);
    auto packetType = packet_payload.substr(0,1).to_int();
    packet_payload = packet_payload.substr(1);
    switch (packetType) {
        case open: {
            if(debugging>0) print_line("[YarnNet] Received engine.IO open package");
            JSON json;
            ERR_FAIL_COND_V_MSG(json.parse(packet_payload) != OK, false, "[YarnNet] Malformed open message!");
            Dictionary params = json.get_data();
            String key_sid = "sid";
            String key_pingTimeout = "pingTimeout";
            String key_pingInterval = "pingInterval";
            ERR_FAIL_COND_V_MSG(!params.has(key_sid) || !params.has(key_pingTimeout) || !params.has(key_pingInterval), false, "[YarnNet] Open message was missing one of the variables needed.");
            sid = params.get(key_sid,key_sid);
            pingInterval = params.get(key_pingInterval,500);
            pingTimeout = params.get(key_pingTimeout,3000);
            set_current_state(State::STATE_OPEN);
            if(debugging>0) print_line("[YarnNet] Connected to engine.io, SID ",sid," ping interval ",pingInterval, " ping timeout ",pingTimeout);
            emit_signal(SNAME("engine_connected"),sid);
            socketio_connect();
        }
            break;
        case close: {
            if(debugging>0) print_line("[YarnNet] Received engine.IO close package");
        }
            break;
        case ping: {
            if(debugging>1) print_line("[YarnNet] Received engine.IO ping package");
            engineio_send_packet_type(pong);
        }
            break;
        case pong: {
            if(debugging>1) print_line("[YarnNet] Received engine.IO pong package");
        }
            break;
        case message: {
            if(debugging>2) print_line("[YarnNet] Received engine.IO message package",packet_payload);
            socketio_parse_packet(packet_payload);
            emit_signal(SNAME("engine_message"));
        }
            break;
        case upgrade: {
            if(debugging>0) print_line("[YarnNet] Received engine.IO upgrade package");
        }
            break;
        case noop: {
            if(debugging>0) print_line("[YarnNet] Received noop message");
        }
            break;
        default:
            break;
    }
    return true;
}

Error YarnNet::engineio_send_packet_type(const EngineIOPacketType packet_type) {
    const CharString cs = vformat("%d",packet_type).utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}


Error YarnNet::engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    return client->send(p_message.ptr(), p_message.size(), WebSocketPeer::WRITE_MODE_BINARY);
}

Error YarnNet::engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text) {
    if(p_text.is_empty()) {
        return engineio_send_packet_type(packet_type);
    }
    p_text = vformat("%d%s",packet_type,p_text);
    const CharString cs = p_text.utf8();
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YarnNet::socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message) {
    p_message.insert(0,static_cast<uint8_t>(packet_type));
    WARN_PRINT("BINARY SENDING NOT IMPLEMENTED");
    return ERR_BUG;
}

Error YarnNet::socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text, String name_space) {
    String _payload = vformat("%d",packet_type);
    if (!name_space.is_empty() && name_space != slash_namespace) {
        _payload += vformat("%s,",name_space);
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
    return client->send((const uint8_t *)cs.ptr(), cs.length(), WebSocketPeer::WRITE_MODE_TEXT);
}

Error YarnNet::socketio_send(String event_name, Variant _data, String name_space) {
    Array payload;
    payload.append(event_name);
    if (_data.get_type() != Variant::Type::NIL)
        payload.append(_data);
    return socketio_send_packet_text(EVENT,payload,name_space);
}

void YarnNet::socketio_connect(String name_space) {
    socketio_send_packet_text(CONNECT, name_space);
}

void YarnNet::socketio_disconnect(String name_space) {
    socketio_send_packet_text(DISCONNECT, name_space);
    emit_signal(SNAME("disconnect"),name_space);
}

bool YarnNet::socketio_parse_packet(String& payload) {
    auto packetType = (EngineIOPacketType)payload.substr(0,1).to_int();
    payload = payload.substr(1);

    auto name_space= slash_namespace;

    auto regex = RegEx::create_from_string("(\\d+)-");
    auto regex_match = regex->search(payload);
    if (regex_match.is_valid() && regex_match->get_start(0) == 0) {
        payload = payload.substr(regex_match->get_end(0));
        ERR_PRINT("[YarnNet] Binary data payload not supported! "+regex_match->get_string(1));
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
        WARN_PRINT("[YarnNet] Ignoring acknowledgement ID "+regex_match->get_string(1));
    }
    regex_match.unref();

    Variant _data {};
    if(payload.length() > 0) {
        JSON json;
        ERR_FAIL_COND_V_MSG(json.parse(payload) != OK, false, vformat("[YarnNet] Malformed socketio event %d namespace: %s",packetType,name_space));
        _data = json.get_data();
    }
    switch (packetType) {
        case SocketIOPacketType::CONNECT: {
            emit_signal(SNAME("connected"),name_space,false);
        }
            break;
        case SocketIOPacketType::CONNECT_ERROR:
            {emit_signal(SNAME("connected"),name_space,true);}
            break;
        case SocketIOPacketType::EVENT: {
            ERR_FAIL_COND_V_MSG(_data.is_array() != true, false, vformat("[YarnNet] Invalid socketio event format %s",_data.to_json_string()));
            Array typedArray = _data;
            const String event_name = typedArray[0];
            const auto event_hash = event_name.hash();
            const Variant event_payload = typedArray.size() > 1 ? typedArray[1] : Variant{};
            if (event_hash == newhost_event) {
                on_host_migrated(event_payload);
            } else if (event_hash == roomcreated_event) {
                on_room_created(event_payload);
            } else if (event_hash == roomjoined_event) {
                on_room_joined(event_payload);
            } else if (event_hash == roomerror_event) {
                on_room_error(event_payload);
            } else if (event_hash == playerjoin_event) {
                on_player_join(event_payload);
            } else if (event_hash == playerleft_event) {
                on_player_left(event_payload);
            } else if (event_hash == rpc_event) {
                print_line("[YarnNet] Need to implement rpc event... This is the Payload received:", event_payload);
            }
            emit_signal(SNAME("event"),event_name,event_payload,name_space);
            if(debugging >0) {
                print_line(vformat("[YarnNet] Socket IO Event Received [%s] data: [%s]",event_name,event_payload));
            }
        }
            break;
        case SocketIOPacketType::DISCONNECT:
            {emit_signal(SNAME("disconnected"),name_space);}
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

void YarnNet::update_last_engine_state() {
    if(offline_mode) {
        return;
    }
    if (last_engine_state != client->get_ready_state()) {
        last_engine_state = static_cast<State>(static_cast<int>(client->get_ready_state()));
        if(debugging >= 1) {
            switch ((State)last_engine_state) {
                case STATE_CONNECTING:
                    print_line("[YarnNet] Engine.io status is now: Connecting");
                    break;
                case STATE_OPEN:
                    print_line("[YarnNet] Engine.io status is now: Connected");
                    break;
                case STATE_CLOSING:
                    print_line("[YarnNet] Engine.io status is now: Closing");
                    break;
                case STATE_CLOSED:
                    print_line("[YarnNet] Engine.io status is now: Closed");
                    break;
                default: ;
            }
        }
    }
}

void YarnNet::do_process() {
    update_last_engine_state();
    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout=true;
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
        emit_signal(SNAME("engine_disconnected"),client->get_close_code(),client->get_close_reason());
        if (status == STATE_OPEN) {
            emit_signal("disconnected",slash_namespace);
        }
        set_current_state(STATE_CLOSED);
        set_process(false);
    }
}

int YarnNet::get_max_queued_packets() {
    if (!client.is_valid()) {
        YarnNet::create_client();
    }
    return client->get_max_queued_packets();
}

Error YarnNet::create_room() {
    socketio_send("requestroom");
    return OK;
}

Error YarnNet::join_or_create_room(const String &join_room) {
    socketio_send("joinOrCreateRoom",join_room);
    return OK;
}
Error YarnNet::join_room(const String &join_room) {
    socketio_send("joinroom",join_room);
    return OK;
}

Error YarnNet::leave_room() {
    socketio_send("leaveroom");
    return OK;
}

void YarnNet::on_room_created(const String &p_new_room_id) {
    if(debugging > 0)
        print_line("room_created ",p_new_room_id);
    emit_signal(SNAME("room_created"),p_new_room_id);
}

// ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "new_room_id")));
// ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "new_room_id")));
// ADD_SIGNAL(MethodInfo("room_error", PropertyInfo(Variant::STRING, "returned_error")));
// ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::STRING, "player_sid")));
// ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::STRING, "player_sid")));
// ADD_SIGNAL(MethodInfo("host_migration", PropertyInfo(Variant::STRING, "new_host_sid")));

void YarnNet::on_room_joined(const String &p_new_room_id) {
    if(debugging > 0)
        print_line("on_room_joined ",p_new_room_id);
    emit_signal(SNAME("room_joined"),p_new_room_id);
}

void YarnNet::on_room_error(const String &p_room_error) {
    if(debugging > 0)
        print_line("on_room_error ",p_room_error);
    emit_signal(SNAME("room_error"),p_room_error);
}

void YarnNet::on_player_join(const String &p_player) {
    if(debugging > 0)
        print_line("on_player_join ",p_player);
    emit_signal(SNAME("player_joined"),p_player);
}

void YarnNet::on_player_left(const String &p_player) {
    if(debugging > 0)
        print_line("on_player_left ",p_player);
    emit_signal(SNAME("player_left"),p_player);
}

void YarnNet::on_host_migrated(const String &p_new_host) {
    if(debugging > 0)
        print_line("on_host_migrated ",p_new_host);
    emit_signal(SNAME("host_migration"),p_new_host);
}

void YarnNet::on_rpc_event(const String &p_sender, const int &p_netnodeid, const int &p_rpc_id, Variant &p_data) {
    if(debugging > 0)
        print_line("on_rpc_event sender:",p_sender," p_netnodeid:",p_netnodeid," p_rpc_id:",p_rpc_id," data:",p_data);
    emit_signal(SNAME("rpc_received"),p_sender,p_netnodeid,p_rpc_id,p_data);
}

YarnNet * YarnNet::get_singleton() {
    return singleton;
}

YarnNet::YarnNet() {
    singleton = this;
    max_queued_packets = 2048;
    call_deferred("setup_node");
}

YarnNet::~YarnNet() {
    if (singleton != nullptr && singleton == this) {
        singleton = nullptr;
    }
    if (client.is_valid()) {
        client.unref();
    }
}

void YarnNet::add_setting(const String& p_name, const Variant& p_default_value, Variant::Type p_type,
PropertyHint p_hint, const String& p_hint_string, int p_usage,bool restart_if_changed) {
    if (!ProjectSettings::get_singleton()->has_setting(p_name)) {
        ProjectSettings::get_singleton()->set_setting(p_name, p_default_value);
    }
    ProjectSettings::get_singleton()->set_custom_property_info(PropertyInfo(p_type, p_name, p_hint, p_hint_string,p_usage));
    ProjectSettings::get_singleton()->set_initial_value(p_name, p_default_value);
    ProjectSettings::get_singleton()->set_restart_if_changed(p_name, restart_if_changed);
}
