#include "yarnnet.h"


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
    BIND_ENUM_CONSTANT(MINIMAL);
    BIND_ENUM_CONSTANT(MOSTMESSAGES);
    BIND_ENUM_CONSTANT(MESSAGESANDPING);
    BIND_ENUM_CONSTANT(ALL);

    ADD_SIGNAL(MethodInfo("room_connection_result", PropertyInfo(Variant::STRING, "room_id"),PropertyInfo(Variant::BOOL, "result")));
    ADD_SIGNAL(MethodInfo("room_connected", PropertyInfo(Variant::INT, "id")));
    ADD_SIGNAL(MethodInfo("room_disconnected", PropertyInfo(Variant::INT, "id")));
    ADD_SIGNAL(MethodInfo("room_info", PropertyInfo(Variant::DICTIONARY, "info")));
    ADD_SIGNAL(MethodInfo("room_list", PropertyInfo(Variant::ARRAY, "list")));
    ADD_SIGNAL(MethodInfo("status_changed", PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::INT, "code"),PropertyInfo(Variant::STRING, "reason")));
    ADD_SIGNAL(MethodInfo("engine_message", PropertyInfo(Variant::STRING, "payload")));
    ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "new_room_id")));
    ADD_SIGNAL(MethodInfo("room_players", PropertyInfo(Variant::ARRAY, "players")));
    ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "new_room_id"), PropertyInfo(Variant::STRING, "new_room_host_id")));
    ADD_SIGNAL(MethodInfo("room_error", PropertyInfo(Variant::STRING, "returned_error")));
    ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::STRING, "player_sid")));
    ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::STRING, "player_sid")));
#ifdef HOST_MIGRATION
    ADD_SIGNAL(MethodInfo("host_migration", PropertyInfo(Variant::STRING, "new_host_sid")));
#endif

    ADD_SIGNAL(MethodInfo("connected", PropertyInfo(Variant::STRING, "name_space"), PropertyInfo(Variant::BOOL, "result")));
    ADD_SIGNAL(MethodInfo("event", PropertyInfo(Variant::STRING, "event_name"),PropertyInfo(Variant::OBJECT, "payload"),PropertyInfo(Variant::STRING, "name_space")));

    ClassDB::bind_method(D_METHOD("setup_node"), &YNet::setup_node);
    ClassDB::bind_method(D_METHOD("ynet_connect", "url"), &YNet::engineio_connect);
    ClassDB::bind_method(D_METHOD("ynet_disconnect"), &YNet::engineio_disconnect);

    ClassDB::bind_static_method("YNet",D_METHOD("string_to_hash_id","str"), &YNet::string_to_hash_id);

    ClassDB::bind_method(D_METHOD("socketio_send_event", "event_name","data","name_space"), &YNet::socketio_send,DEFVAL(Variant{}),DEFVAL(""));
    ClassDB::bind_method(D_METHOD("socketio_connect", "name_space"), &YNet::socketio_connect,DEFVAL(slash_namespace));
    ClassDB::bind_method(D_METHOD("socketio_disconnect", "name_space"), &YNet::socketio_disconnect,DEFVAL(slash_namespace));

    ClassDB::bind_method(D_METHOD("set_debugging", "debugging_level"), &YNet::set_debugging);
    ClassDB::bind_method(D_METHOD("get_debugging"), &YNet::get_debugging);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "debugging", PROPERTY_HINT_ENUM, "None,Minimal,MostMessages,MesssagesAndPings,All"), "set_debugging", "get_debugging");

    ClassDB::bind_method(D_METHOD("set_is_host", "status"), &YNet::set_is_host);
    ClassDB::bind_method(D_METHOD("get_is_host"), &YNet::get_is_host);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_host"), "set_is_host", "get_is_host");

    ClassDB::bind_method(D_METHOD("set_pause_receive_spawns", "status"), &YNet::set_pause_receive_spawns);
    ClassDB::bind_method(D_METHOD("get_pause_receive_spawns"), &YNet::get_pause_receive_spawns);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pause_receive_spawns"), "set_pause_receive_spawns", "get_pause_receive_spawns");

    ClassDB::bind_method(D_METHOD("set_hashed_socket_id", "status"), &YNet::set_hashed_sid);
    ClassDB::bind_method(D_METHOD("get_hashed_socket_id"), &YNet::get_hashed_sid);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hashed_socket_id"), "set_hashed_socket_id", "get_hashed_socket_id");

    ClassDB::bind_method(D_METHOD("set_real_hashed_socket_id", "status"), &YNet::set_real_hashed_sid);
    ClassDB::bind_method(D_METHOD("get_real_hashed_socket_id"), &YNet::get_real_hashed_sid);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "real_hashed_socket_id"), "set_real_hashed_socket_id", "get_real_hashed_socket_id");

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
    ClassDB::bind_method(D_METHOD("create_room_with_code", "roomcode"), &YNet::create_room_with_code);
    ClassDB::bind_method(D_METHOD("join_or_create_room", "roomcode"), &YNet::join_or_create_room, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("join_room", "roomcode"), &YNet::join_room);
    ClassDB::bind_method(D_METHOD("leave_room"), &YNet::leave_room);
    ClassDB::bind_method(D_METHOD("join_room_with_password", "roomCode", "password"), &YNet::join_room_with_password);

    ClassDB::bind_method(D_METHOD("set_password", "newPassword"), &YNet::set_password);
    ClassDB::bind_method(D_METHOD("set_max_players", "newMaxPlayers"), &YNet::set_max_players);
    ClassDB::bind_method(D_METHOD("set_private", "newPrivate"), &YNet::set_private);
    ClassDB::bind_method(D_METHOD("set_can_host_migrate", "newCanHostMigrate"), &YNet::set_can_host_migrate);
    ClassDB::bind_method(D_METHOD("set_room_name", "newRoomName"), &YNet::set_room_name);
    ClassDB::bind_method(D_METHOD("set_extra_info", "new_extra_info"), &YNet::set_extra_info);
    ClassDB::bind_method(D_METHOD("get_room_info", "roomCode"), &YNet::get_room_info, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("get_room_list"), &YNet::get_room_list);

    ClassDB::bind_method(D_METHOD("add_network_spawnable","spawnable_path"), &YNet::add_network_spawnable);
    ClassDB::bind_method(D_METHOD("get_network_spawnable_id","spawnable_path"), &YNet::get_network_spawnable_id);
    ClassDB::bind_method(D_METHOD("find_network_spawnable","spawnable_id"), &YNet::find_network_spawnable);
    ClassDB::bind_method(D_METHOD("is_network_spawnable","spawnable_path"), &YNet::is_network_spawnable);
    
    ClassDB::bind_method(D_METHOD("find_node_with_net_id","net_id"), &YNet::find_node_with_net_id);
    ClassDB::bind_method(D_METHOD("update_networked_property_syncers"), &YNet::update_networked_property_syncers);

    ClassDB::bind_method(D_METHOD("clear_all_spawned_network_nodes"), &YNet::clear_all_spawned_network_nodes);
    ClassDB::bind_method(D_METHOD("register_sync_property","networked_node","property_path", "authority", "always_sync"), &YNet::register_sync_property, DEFVAL(1), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("spawn","spawnable_scene","spawned_name","parent_path","global_pos","authority"), &YNet::spawn,DEFVAL(1));
    ClassDB::bind_method(D_METHOD("spawn_with_path","spawnable_scene_path","spawned_name","parent_path","global_pos","authority"), &YNet::spawn_with_path,DEFVAL(1));

    ClassDB::bind_method(D_METHOD("was_last_rpc_sender_host"), &YNet::was_last_rpc_sender_host);

    //spawn(Ref<PackedScene> p_spawnable_scene, Variant p_spawn_pos, String p_spawn_name, NodePath p_desired_parent)

    ClassDB::bind_method(D_METHOD("rpc_spawn","network_id","packedscene_path_id","spawn_name","desired_parent_absolute_path","spawn_pos","authority"), &YNet::rpc_spawn,DEFVAL(1));

    ClassDB::bind_method(D_METHOD("rpc_despawn","network_obj_id"), &YNet::rpc_despawn);
    ClassDB::bind_method(D_METHOD("despawn","network_obj_id"), &YNet::despawn);
    ClassDB::bind_method(D_METHOD("despawn_node","node"), &YNet::despawn_node);
    ClassDB::bind_method(D_METHOD("get_spawned_obj_count"), &YNet::get_spawned_obj_count);
    ClassDB::bind_method(D_METHOD("get_queued_spawn_count"), &YNet::get_queued_spawn_count);

    ClassDB::bind_method(D_METHOD("server_or_client_str"), &YNet::server_or_client_str);

    ClassDB::bind_method(D_METHOD("register_for_yrpc","node","yrpc_id"), &YNet::register_for_yrpcs);
    ClassDB::bind_method(D_METHOD("remove_from_yrpc","yrpc_id"), &YNet::remove_from_yrpc_receiving_map);

    ClassDB::bind_method(D_METHOD("rpc_recv_sync_vars","synced_vars_data"), &YNet::rpc_recv_sync_vars);
    ClassDB::bind_method(D_METHOD("rpc_respond_with_spawned_nodes","spawned_nodes_data"), &YNet::rpc_respond_with_spawned_nodes);
    ClassDB::bind_method(D_METHOD("rpc_request_spawned_nodes","requester_id"), &YNet::rpc_request_spawned_nodes);

    ClassDB::bind_static_method("YNet",D_METHOD("get_debug_run_multiple_instances"), &YNet::get_debug_run_multiple_instances);
    ClassDB::bind_static_method("YNet",D_METHOD("set_debug_run_multiple_instances","status"), &YNet::set_debug_run_multiple_instances);

    //test_send_sync()

    {
        MethodInfo mi;
        mi.name = "send_yrpc";
        mi.arguments.push_back(PropertyInfo(Variant::CALLABLE, "method"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "send_yrpc", &YNet::_send_yrpc, mi);
    }

    {
        MethodInfo mi;
        mi.name = "send_yrpc_to";
        mi.arguments.push_back(PropertyInfo(Variant::INT, "target_peer"));
        mi.arguments.push_back(PropertyInfo(Variant::CALLABLE, "method"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "send_yrpc_to", &YNet::_send_yrpc_to, mi);
    }

    {
        MethodInfo mi;
        mi.name = "receive_yrpc";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc", &YNet::_receive_yrpc, mi);
    }

    {
        MethodInfo mi;
        mi.name = "receive_yrpc_call_local";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc_call_local", &YNet::_receive_yrpc_also_local, mi);
    }

    {
        MethodInfo mi;
        mi.name = "receive_yrpc_reliable";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc_reliable", &YNet::_receive_yrpc, mi);
    }

    {
        MethodInfo mi;
        mi.name = "receive_yrpc_reliable_call_local";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc_reliable_call_local", &YNet::_receive_yrpc_also_local, mi);
    }


    {
        MethodInfo mi;
        mi.name = "receive_yrpc_unreliable";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc_unreliable", &YNet::_receive_yrpc, mi);
    }

    {
        MethodInfo mi;
        mi.name = "receive_yrpc_unreliable_call_local";
        mi.arguments.push_back(PropertyInfo(Variant::ARRAY, "yrpc_info"));

        ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "receive_yrpc_unreliable_call_local", &YNet::_receive_yrpc_also_local, mi);
    }
}

uint32_t YNet::get_new_network_id() {
    uint32_t hash = 0;

	while (hash == 0 || hash == 1) {
		hash = hash_murmur3_one_32(
				(uint32_t)OS::get_singleton()->get_ticks_usec());
		hash = hash_murmur3_one_32(
				(uint32_t)OS::get_singleton()->get_unix_time(), hash);
		hash = hash_murmur3_one_32(
				(uint32_t)OS::get_singleton()->get_user_data_dir().hash64(), hash);
		hash = hash_murmur3_one_32(
				(uint32_t)((uint64_t)this), hash); // Rely on ASLR heap
		hash = hash_murmur3_one_32(
				(uint32_t)((uint64_t)&hash), hash); // Rely on ASLR stack

		hash = hash_fmix32(hash);
		hash = hash & 0x7FFFFFFF; // Make it compatible with unsigned, since negative ID is used for exclusion
	}

    if (yrpc_to_node_hash_map.has(hash)) {
        return get_new_network_id();
    }
	return hash;
}

Ref<YNetPropertySyncer> YNet::register_sync_property(Node *p_target, const NodePath &p_property, int authority, bool p_always_sync) {
    ERR_FAIL_NULL_V(p_target, nullptr);

    ERR_FAIL_COND_V_MSG(!p_target->has_meta("_net_id"), nullptr, "Registering a sync property with an object that doesn't have NetID");

    int p_net_id = p_target->get_meta("_net_id");
    if (networked_id_to_authority.has(p_net_id) && networked_id_to_authority[p_net_id] != authority)
        authority = networked_id_to_authority[p_net_id];
    //print_line(vformat("Registering sync property %s for netid %d authority %d",p_property.get_concatenated_subnames(), p_net_id, authority));
    Vector<StringName> property_subnames = p_property.get_as_property_path().get_subnames();

    const Variant &prop_value = p_target->get_indexed(property_subnames);

    Ref<YNetPropertySyncer> propsyncer;

    propsyncer.instantiate(p_net_id, p_target, property_subnames, prop_value, authority, p_always_sync);
    
    return propsyncer;
}

void YNet::remove_from_yrpc_receiving_map(int p_yrpc_id) {
    yrpc_to_node_hash_map.erase(p_yrpc_id);
}

Variant YNet::_receive_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
    if (p_argcount < 1) {
        r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.expected = 1;
        return ERR_INVALID_PARAMETER;
    }

    Variant::Type itype = p_args[0]->get_type();
    if (itype != Variant::ARRAY) {
        r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 0;
        r_error.expected = Variant::ARRAY;
        return ERR_INVALID_PARAMETER;
    }

    Array yrpc_info = p_args[0]->operator Array();
    int netid = yrpc_info[0];

    if (yrpc_to_node_hash_map.has(netid)) {
        auto node_found = ObjectDB::get_instance(yrpc_to_node_hash_map[netid]);
        if (node_found != nullptr) {
            Node *actual_node = Object::cast_to<Node>(node_found);
            if (actual_node != nullptr) {
                bool is_relative = yrpc_info[1];
                StringName method = yrpc_info[is_relative ? 3 : 2];
                if (is_relative) {
                    actual_node = actual_node->get_node(yrpc_info[2]);
                }
                if (actual_node != nullptr) {
                    // Get RPC config for this method
                    YNetRPCConfig rpc_config = _get_rpc_config(actual_node, method);
                    
                    // Check if we should receive this RPC
                    bool can_receive = false;
                    switch (rpc_config.rpc_mode) {
                        case MultiplayerAPI::RPC_MODE_ANY_PEER:
                            can_receive = true;
                            break;
                        case MultiplayerAPI::RPC_MODE_AUTHORITY:
                            can_receive = get_multiplayer()->get_remote_sender_id() == actual_node->get_multiplayer_authority();
                            break;
                        case MultiplayerAPI::RPC_MODE_DISABLED:
                        default:
                            can_receive = false;
                            break;
                    }

                    if (!can_receive) {
                        if (debugging >= DebuggingLevel::MINIMAL) {
                            print_line(vformat("[YNet] Blocked unauthorized RPC call to %s from peer %d", method, get_multiplayer()->get_remote_sender_id()));
                        }
                        return ERR_UNAUTHORIZED;
                    }

                    if (actual_node->has_method(method)) {
                        actual_node->callp(method, &p_args[1], p_argcount - 1, r_error);
                    }
                }
            }
        }
    }

    return 0;
}

Variant YNet::_receive_yrpc_also_local(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
    if (p_argcount < 1) {
        r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.expected = 1;
        return ERR_INVALID_PARAMETER;
    }

    Variant::Type itype = p_args[0]->get_type();
    if (itype != Variant::ARRAY) {
        r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 0;
        r_error.expected = Variant::ARRAY;
        return ERR_INVALID_PARAMETER;
    }

    Array yrpc_info = p_args[0]->operator Array();
    int netid = yrpc_info[0];

    if (yrpc_to_node_hash_map.has(netid)) {
        auto node_found = ObjectDB::get_instance(yrpc_to_node_hash_map[netid]);
        if (node_found != nullptr) {
            Node *actual_node = Object::cast_to<Node>(node_found);
            if (actual_node != nullptr) {
                bool is_relative = yrpc_info[1];
                StringName method = yrpc_info[is_relative ? 3 : 2];
                if (is_relative) {
                    actual_node = actual_node->get_node(yrpc_info[2]);
                }
                if (actual_node != nullptr) {
                    if (actual_node->has_method(method)) {
                        actual_node->callp(method,&p_args[1], p_argcount - 1, r_error);
                    }
                }
                //print_line(vformat("Received yrpc id %d method %s",objid,method));
            }
        }
    }

    return 0;
}


Error YNet::_send_yrpc_direct(Node *p_node, const StringName &p_method, const Variant **p_args, int p_argcount) {
    ERR_FAIL_COND_V(!is_inside_tree(), ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(p_node == nullptr, ERR_DOES_NOT_EXIST);

    // Get RPC config for this method
    YNet::YNetRPCConfig rpc_config = _get_rpc_config(p_node, p_method);
    
    // Check if we can send this RPC
    bool can_send = false;
    switch (rpc_config.rpc_mode) {
        case MultiplayerAPI::RPC_MODE_ANY_PEER:
            can_send = true;
            break;
        case MultiplayerAPI::RPC_MODE_AUTHORITY:
            can_send = get_multiplayer()->get_unique_id() == p_node->get_multiplayer_authority();
            break;
        case MultiplayerAPI::RPC_MODE_DISABLED:
        default:
            can_send = false;
            break;
    }

    if (!can_send) {
        print_error(vformat("Invalid call for function %s. Doesn't have authority.", p_method));
        return ERR_UNAUTHORIZED;
    }

    int net_id = -1;
    Node *current = p_node;
    while (current != nullptr) {
        if (current->has_meta(SNAME("_net_id"))) {
            net_id = current->get_meta(SNAME("_net_id"),-1);
            break;
        }
        current = current->get_parent();
    }

    ERR_FAIL_COND_V(net_id == -1, ERR_UNCONFIGURED);

    Array sending_rpc_array;
    sending_rpc_array.push_back(net_id);
    if (current == p_node) {
        sending_rpc_array.push_back(false);
    } else {
        sending_rpc_array.push_back(true);
        sending_rpc_array.push_back(current->get_path_to(p_node));
    }
    sending_rpc_array.push_back(p_method);

    // Create new args array with the RPC info as first argument
    const Variant **new_args = (const Variant **)alloca(sizeof(Variant *) * (p_argcount + 1));
    new_args[0] = new Variant(sending_rpc_array);
    for (int i = 0; i < p_argcount; i++) {
        new_args[i + 1] = p_args[i];
    }

    if (scene_multiplayer.is_null()) {
        return ERR_UNCONFIGURED;
    }

    return scene_multiplayer->rpcp(this, 0, rpc_config.call_local ? 
        rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? 
            receive_yrpc_reliable_also_local_stringname : 
            receive_yrpc_unreliable_also_local_stringname : 
        rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? 
            receive_yrpc_reliable_stringname : 
            receive_yrpc_stringname, 
        new_args, p_argcount + 1);
}

Error YNet::_send_yrpc_to_direct(Node *p_node, int p_target_peer, const StringName &p_method, const Variant **p_args, int p_argcount) {
    ERR_FAIL_COND_V(!is_inside_tree(), ERR_UNCONFIGURED);
    ERR_FAIL_COND_V(p_node == nullptr, ERR_DOES_NOT_EXIST);

    // Get RPC config for this method
    YNet::YNetRPCConfig rpc_config = _get_rpc_config(p_node, p_method);
    
    // Check if we can send this RPC
    bool can_send = false;
    switch (rpc_config.rpc_mode) {
        case MultiplayerAPI::RPC_MODE_ANY_PEER:
            can_send = true;
            break;
        case MultiplayerAPI::RPC_MODE_AUTHORITY:
            can_send = get_multiplayer()->get_unique_id() == p_node->get_multiplayer_authority();
            break;
        case MultiplayerAPI::RPC_MODE_DISABLED:
        default:
            can_send = false;
            break;
    }

    if (!can_send) {
        print_error(vformat("Invalid call for function %s. Doesn't have authority.", p_method));
        return ERR_UNAUTHORIZED;
    }

    int net_id = -1;
    Node *current = p_node;
    while (current != nullptr) {
        if (current->has_meta(SNAME("_net_id"))) {
            net_id = current->get_meta(SNAME("_net_id"),-1);
            break;
        }
        current = current->get_parent();
    }

    ERR_FAIL_COND_V(net_id == -1, ERR_UNCONFIGURED);

    Array sending_rpc_array;
    sending_rpc_array.push_back(net_id);
    if (current == p_node) {
        sending_rpc_array.push_back(false);
    } else {
        sending_rpc_array.push_back(true);
        sending_rpc_array.push_back(current->get_path_to(p_node));
    }
    sending_rpc_array.push_back(p_method);

    // Create new args array with the RPC info as first argument
    const Variant **new_args = (const Variant **)alloca(sizeof(Variant *) * (p_argcount + 1));
    new_args[0] = new Variant(sending_rpc_array);
    for (int i = 0; i < p_argcount; i++) {
        new_args[i + 1] = p_args[i];
    }

    if (scene_multiplayer.is_null()) {
        return ERR_UNCONFIGURED;
    }

    // If target peer is the same as the sender, call the method locally
    if (p_target_peer == get_multiplayer()->get_unique_id()) {
        if (p_node->has_method(p_method)) {
            Callable::CallError r_error;
            p_node->callp(p_method, p_args, p_argcount, r_error);
            if (r_error.error == Callable::CallError::CALL_ERROR_INVALID_ARGUMENT) {
                return ERR_INVALID_PARAMETER;
            } else if (r_error.error == Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS) {
                return ERR_INVALID_DATA;
            } else if (r_error.error == Callable::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS) {
                return ERR_INVALID_DATA;
            } else if (r_error.error == Callable::CallError::CALL_ERROR_INVALID_METHOD) {
                return ERR_METHOD_NOT_FOUND;
            }
            return OK;
        }
        return ERR_METHOD_NOT_FOUND;
    }

    return scene_multiplayer->rpcp(this, p_target_peer, rpc_config.call_local ? 
        rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? 
            receive_yrpc_reliable_also_local_stringname : 
            receive_yrpc_unreliable_also_local_stringname : 
        rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? 
            receive_yrpc_reliable_stringname : 
            receive_yrpc_stringname, 
        new_args, p_argcount + 1);
}

Error YNet::_send_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
    if (p_argcount < 1) {
        r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.expected = 1;
        return ERR_INVALID_PARAMETER;
    }

    Variant::Type type = p_args[0]->get_type();
    if (type != Variant::CALLABLE) {
        r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 0;
        r_error.expected = Variant::CALLABLE;
        return ERR_INVALID_PARAMETER;
    }

    Callable p_callable = p_args[0]->operator Callable();
    ERR_FAIL_COND_V(!is_inside_tree(), ERR_UNCONFIGURED);

    Node* callable_object = Object::cast_to<Node>(p_callable.get_object());
    ERR_FAIL_COND_V(callable_object == nullptr, ERR_DOES_NOT_EXIST);

    // Get RPC config for this method
    YNet::YNetRPCConfig rpc_config = _get_rpc_config(callable_object, p_callable.get_method());
    
    // Check if we can send this RPC
    bool can_send = false;
    switch (rpc_config.rpc_mode) {
        case MultiplayerAPI::RPC_MODE_ANY_PEER:
            can_send = true;
            break;
        case MultiplayerAPI::RPC_MODE_AUTHORITY:
            can_send = get_multiplayer()->get_unique_id() == callable_object->get_multiplayer_authority();
            break;
        case MultiplayerAPI::RPC_MODE_DISABLED:
        default:
            can_send = false;
            break;
    }

    if (!can_send) {
        print_error(vformat("Invalid call for function %s. Doesn't have authority.", p_callable.get_method()));
        return ERR_UNAUTHORIZED;
    }

    int net_id = -1;
    Node *current = callable_object;
    while (current != nullptr) {
        if (current->has_meta(SNAME("_net_id"))) {
            net_id = current->get_meta(SNAME("_net_id"),-1);
            break;
        }
        current = current->get_parent();
    }

    ERR_FAIL_COND_V(callable_object == nullptr || net_id == -1, ERR_UNCONFIGURED);

    Array sending_rpc_array;
    sending_rpc_array.push_back(net_id);
    if (current == callable_object) {
        sending_rpc_array.push_back(false);
    } else {
        sending_rpc_array.push_back(true);
        sending_rpc_array.push_back(current->get_path_to(callable_object));
    }
    sending_rpc_array.push_back(p_callable.get_method());
    p_args[0] = new Variant(sending_rpc_array);

    if (scene_multiplayer.is_null()) {
        return ERR_UNCONFIGURED;
    }

    return scene_multiplayer->rpcp(this, 0, rpc_config.call_local ? rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? receive_yrpc_reliable_also_local_stringname : receive_yrpc_unreliable_also_local_stringname : rpc_config.transfer_mode == MultiplayerPeer::TRANSFER_MODE_RELIABLE ? receive_yrpc_reliable_stringname : receive_yrpc_stringname, p_args, p_argcount);
}

Error YNet::_send_yrpc_to(const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
    if (p_argcount < 2) {
        r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.expected = 2;
        return ERR_INVALID_PARAMETER;
    }

    Variant::Type type = p_args[0]->get_type();
    if (type != Variant::INT) {
        r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 0;
        r_error.expected = Variant::INT;
        return ERR_INVALID_PARAMETER;
    }

    type = p_args[1]->get_type();
    if (type != Variant::CALLABLE) {
        r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = 1;
        r_error.expected = Variant::CALLABLE;
        return ERR_INVALID_PARAMETER;
    }

    int target_peer = p_args[0]->operator int();
    Callable p_callable = p_args[1]->operator Callable();
    ERR_FAIL_COND_V(!is_inside_tree(), ERR_UNCONFIGURED);

    Node* callable_object = Object::cast_to<Node>(p_callable.get_object());
    ERR_FAIL_COND_V(callable_object == nullptr, ERR_DOES_NOT_EXIST);

    // Get RPC config for this method
    YNetRPCConfig rpc_config = _get_rpc_config(callable_object, p_callable.get_method());
    
    // Check if we can send this RPC
    bool can_send = false;
    switch (rpc_config.rpc_mode) {
        case MultiplayerAPI::RPC_MODE_ANY_PEER:
            can_send = true;
            break;
        case MultiplayerAPI::RPC_MODE_AUTHORITY:
            can_send = get_multiplayer()->get_unique_id() == callable_object->get_multiplayer_authority();
            break;
        case MultiplayerAPI::RPC_MODE_DISABLED:
        default:
            can_send = false;
            break;
    }

    if (!can_send) {
        print_error(vformat("Invalid call for function %s. Doesn't have authority.", p_callable.get_method()));
        return ERR_INVALID_PARAMETER;
    }

    int net_id = -1;
    Node *current = callable_object;
    while (current != nullptr) {
        if (current->has_meta(SNAME("_net_id"))) {
            net_id = current->get_meta(SNAME("_net_id"),-1);
            break;
        }
        current = current->get_parent();
    }

    ERR_FAIL_COND_V(callable_object == nullptr || net_id == -1, ERR_UNCONFIGURED);

    Array sending_rpc_array;
    sending_rpc_array.push_back(net_id);
    if (current == callable_object) {
        sending_rpc_array.push_back(false);
    } else {
        sending_rpc_array.push_back(true);
        sending_rpc_array.push_back(current->get_path_to(callable_object));
    }
    sending_rpc_array.push_back(p_callable.get_method());
    p_args[1] = new Variant(sending_rpc_array);

    if (scene_multiplayer.is_null()) {
        return ERR_UNCONFIGURED;
    }

    // If target peer is the same as the sender, call the method locally
    if (target_peer == get_multiplayer()->get_unique_id()) {
        if (callable_object->has_method(p_callable.get_method())) {
            callable_object->callp(p_callable.get_method(), &p_args[2], p_argcount - 2, r_error);
            return OK;
        }
        return ERR_METHOD_NOT_FOUND;
    }

    return scene_multiplayer->rpcp(this, target_peer, receive_yrpc_stringname, &p_args[1], p_argcount - 1);
}

void YNet::cleanup_node() {
    if (is_inside_tree()) {
        queue_free();
    }
}

void YNet::setup_node() {
    add_setting("YNet/settings/enabled", false, Variant::Type::BOOL);
    add_setting("YNet/settings/protocol", "change_me", Variant::Type::STRING);
    //r_options->push_back(ImportOption(PropertyInfo(Variant::ARRAY, "fallbacks", PROPERTY_HINT_ARRAY_TYPE, PROPERTY_HINT_NODE_TYPE), Array()));
    add_setting("YNet/settings/network_spawnable_scenes", TypedArray<String>(), Variant::Type::ARRAY, PROPERTY_HINT_ARRAY_TYPE,
            vformat("%s/%s:",Variant::Type::STRING, PROPERTY_HINT_FILE));
    add_setting("YNet/settings/send_synced_properties_interval", 0.025f, Variant::Type::FLOAT, PROPERTY_HINT_RANGE,"0.001,0.5,0.001");
    add_setting("YNet/settings/watch_synced_properties_interval", 0.025f, Variant::Type::FLOAT, PROPERTY_HINT_RANGE,"0.001,0.5,0.001");

    if(!already_setup_in_tree && SceneTree::get_singleton() != nullptr) {
        bool is_enabled = GLOBAL_GET("YNet/settings/enabled");
        if(!is_enabled) {
            ynet_settings_enabled = false;
            queue_free();
            return;
        }
        ynet_settings_enabled=true;
        SceneTree::get_singleton()->get_root()->call_deferred("add_child",this);
        SceneTree::get_singleton()->get_root()->connect(SceneStringName(tree_exiting), callable_mp(this, &YNet::cleanup_node));
        set_name("YNet");
        already_setup_in_tree=true;
    }

    TypedArray<String> network_spawnables_paths = GLOBAL_GET("YNet/settings/network_spawnable_scenes");
    for (String network_spawnables_path: network_spawnables_paths) {
        add_network_spawnable(network_spawnables_path);
    }

    protocol = GLOBAL_GET("YNet/settings/protocol");
    watch_synced_vars_interval = static_cast<uint64_t>((static_cast<double>(GLOBAL_GET("YNet/settings/watch_synced_properties_interval")) * 1000 * 1000));
    send_synced_vars_interval = static_cast<uint64_t>((static_cast<double>(GLOBAL_GET("YNet/settings/send_synced_properties_interval")) * 1000 * 1000));
}

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
        client.unref();
    }
}

void YNet::rpc_spawn(const int p_network_id, const uint32_t &p_spawnable_path_id, const String &p_spawn_name, const String &p_desired_parent, const Variant &p_spawn_pos, const int authority) {
    ERR_FAIL_COND_MSG(!spawnables_dictionary.has(p_spawnable_path_id), "ERROR: Received a spawn rpc with a scene id that wasn't present as a spawnable scene in YNet");
    Ref<PackedScene> ps = ResourceLoader::load(spawnables_dictionary[p_spawnable_path_id]);
    ERR_FAIL_COND_MSG(ps.is_null() || !ps.is_valid(), "ERROR: Received a spawn rpc with a invalid or null packed scene");
    internal_spawn(p_network_id,ps,p_spawn_name,NodePath{p_desired_parent},p_spawn_pos,authority);
}

void YNet::internal_spawn_with_queued_struct(const NetworkSpawnedObjectInfo &p_nsoi) {
    const int p_nid = p_nsoi.network_instance_id;
    const Vector3 spawn_pos = Vector3{static_cast<float>(p_nsoi.spawn_pos_x) * 0.01f, static_cast<float>(p_nsoi.spawn_pos_y) * 0.01f, static_cast<float>(p_nsoi.spawn_pos_z) * 0.01f};
    const Ref<PackedScene> ps = ResourceLoader::load(spawnables_dictionary[p_nsoi.spawnable_scene_id]);
    internal_spawn(p_nid,ps,p_nsoi.spawned_name,p_nsoi.desired_parent,spawn_pos,p_nsoi.authority);
}

Variant YNet::create_spawned_lists_variant() {
    Array variant_array;

    for (const auto& _spawn_queued_list: queued_networked_spawned_objects) {
        variant_array.push_back(convert_nsoi_to_variant(_spawn_queued_list.value));
    }
    for (const auto& _spawned_list: networked_spawned_objects) {
        variant_array.push_back(convert_nsoi_to_variant(_spawned_list.value));
    }
    int amount_property_syncers_added = 0;
    // Iterate over the HashMap
    for (auto itr = networked_property_syncers.begin(); itr != networked_property_syncers.end(); ++itr) {
        // Get the Vector associated with the current key
        Vector<Ref<YNetPropertySyncer>> &property_syncers = itr->value;
        // Iterate over the Vector
        for (int i = 0; i < property_syncers.size(); ++i) {
            Array property_syncer_info;
            property_syncer_info.push_back(property_syncers[i]->net_id);
            property_syncer_info.push_back(property_syncers[i]->property_syncer_index);
            property_syncer_info.push_back(property_syncers[i]->current_val);
            variant_array.push_back(property_syncer_info);
            amount_property_syncers_added++;
        }
    }

    variant_array.push_front(Vector3i{static_cast<int>(queued_networked_spawned_objects.size()),static_cast<int>(networked_spawned_objects.size()),amount_property_syncers_added});

    return variant_array;
}

void YNet::unpack_property_syncer_received_value(const Array& received_property_syncer_value) {
    HashMap<uint8_t, Variant>& inner_map = queued_received_property_syncers[received_property_syncer_value[0]];
    inner_map[received_property_syncer_value[1]] = received_property_syncer_value[2];
}

void YNet::unpack_spawned_list_variants(const Array& received_spawned_list) {
    const Vector3i received_info_amounts = received_spawned_list[0];
    int received_property_values_amount = 0;
    for (int i = 1; i < received_spawned_list.size(); ++i) {
        if (i <= received_info_amounts.x + received_info_amounts.y) {
            unpack_spawninfo_from_variant (received_spawned_list[i]);
        } else {
            unpack_property_syncer_received_value(received_spawned_list[i]);
            received_property_values_amount += 1;
        }
    }

    if (debugging >= DebuggingLevel::MINIMAL)
        print_line(vformat("[YNet] Received spawn list. [%d] Network Spawns [%d] Queued Network Spawns [%d obj %d prop] Queued Sync Properties for objects",networked_spawned_objects.size(), queued_networked_spawned_objects.size(), queued_received_property_syncers.size(), received_property_values_amount));
}

bool YNet::was_last_rpc_sender_host() const {
    return get_multiplayer()->get_remote_sender_id() == 1;
}

Variant YNet::convert_nsoi_to_variant(const NetworkSpawnedObjectInfo &p_nsoi) {
    Array variant_array;
    variant_array.push_back(p_nsoi.network_instance_id);
    variant_array.push_back(p_nsoi.spawnable_scene_id);
    variant_array.push_back(p_nsoi.spawned_name);
    variant_array.push_back(String(p_nsoi.desired_parent));
    variant_array.push_back(p_nsoi.authority);
    bool has_spawned = false;
    bool has_z = false;
    if (p_nsoi.SpawnedNodeId.is_valid()) {
        const auto find_instance = ObjectDB::get_instance(p_nsoi.SpawnedNodeId);
        const auto spawned_node2D = Object::cast_to<Node2D>(find_instance);
        if (spawned_node2D != nullptr) {
            const Vector2 _current_pos = spawned_node2D->get_global_position();
            has_spawned = true;
            variant_array.push_back(_current_pos.x * 100.0);
            variant_array.push_back(_current_pos.y * 100.0);
        } else {
            const auto spawned_node3D = Object::cast_to<Node3D>(find_instance);
            if (spawned_node3D != nullptr) {
                const Vector3 _current_pos = spawned_node3D->get_global_position();
                has_spawned=true;
                has_z = true;
                variant_array.push_back(_current_pos.x * 100.0);
                variant_array.push_back(_current_pos.y * 100.0);
                variant_array.push_back(_current_pos.z * 100.0);
            }
        }
    }

    if (!has_spawned) {
        variant_array.push_back(p_nsoi.spawn_pos_x);
        variant_array.push_back(p_nsoi.spawn_pos_y);
    }
    if (!has_z) {
        variant_array.push_back(p_nsoi.spawn_pos_z);
    }
    return variant_array;
}

void YNet::unpack_spawninfo_from_variant(const Array& received_spawn_info) {
    ERR_FAIL_COND_MSG(received_spawn_info.size() < 8, "ERROR: Attempted to unpack a spawninfo variable with wrong number of arguments");
    int _n_id = received_spawn_info[0];
    if (networked_spawned_objects.has(_n_id)) {
        return;
    }
    networked_id_to_authority[_n_id] = received_spawn_info[4];
    queued_networked_spawned_objects[_n_id] = NetworkSpawnedObjectInfo{_n_id, received_spawn_info[1],
        received_spawn_info[2], ObjectID{}, NodePath{received_spawn_info[3]}, received_spawn_info[4]
    ,received_spawn_info[5], received_spawn_info[6], received_spawn_info[7]};
}

Node *YNet::internal_spawn(int p_network_id, const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name, const NodePath &p_desired_parent, const Variant &p_spawn_pos, const int authority) {
    ERR_FAIL_COND_V_MSG(p_spawnable_scene.is_null() || !p_spawnable_scene.is_valid(), nullptr, "ERROR: Spawnable scene is not valid");
    uint32_t desired_spawn_path_id = string_to_hash_id(p_spawnable_scene->get_path());
    ERR_FAIL_COND_V_MSG(!spawnables_dictionary.has(desired_spawn_path_id), nullptr, "ERROR: Provided scene wasn't added as a spawnable scene to YNet");

    networked_id_to_authority[p_network_id] = authority;

    int _spawn_x = -1;
    int _spawn_y = -1;
    int _spawn_z = -1;

    if (p_spawn_pos.get_type() == Variant::Type::VECTOR3) {
        const Vector3 _desired_spawn_pos = p_spawn_pos;
        _spawn_x = static_cast<int>(_desired_spawn_pos.x * 100);
        _spawn_y = static_cast<int>(_desired_spawn_pos.y * 100);
        _spawn_z = static_cast<int>(_desired_spawn_pos.z * 100);
    }
    else if (p_spawn_pos.get_type() == Variant::Type::VECTOR2) {
        const Vector2 _desired_spawn_pos = p_spawn_pos;
        _spawn_x = static_cast<int>(_desired_spawn_pos.x * 100);
        _spawn_y = static_cast<int>(_desired_spawn_pos.y * 100);
    }

    Node* p_desired_parent_node = this->get_node_or_null(p_desired_parent);

    if (p_desired_parent_node == nullptr || pause_receive_spawns) {
        // Parent node isn't here yet, try again later.
        if (!queued_networked_spawned_objects.has(p_network_id))
            queued_networked_spawned_objects[p_network_id] = NetworkSpawnedObjectInfo{p_network_id, desired_spawn_path_id, p_spawn_name, ObjectID{}, p_desired_parent, authority, _spawn_x, _spawn_y, _spawn_z};
        return nullptr;
    }

    if (get_multiplayer()->is_server()) {
        Array p_arguments;
        p_arguments.push_back(p_network_id);
        p_arguments.push_back(desired_spawn_path_id);
        p_arguments.push_back(p_spawn_name);
        p_arguments.push_back(String(p_desired_parent));
        p_arguments.push_back(p_spawn_pos);
        p_arguments.push_back(authority);
        int argcount = p_arguments.size();
        const Variant **argptrs = (const Variant **)alloca(sizeof(Variant *) * argcount);
        for (int i = 0; i < argcount; i++) {
            argptrs[i] = &p_arguments[i];
        }
        rpcp(0,rpc_spawn_stringname,argptrs,argcount);
    }

    auto spawned_instance = p_spawnable_scene->instantiate();

    ERR_FAIL_COND_V_MSG(spawned_instance == nullptr, nullptr, "ERROR: Spawned Instance is null");

    spawned_instance->set_name(p_spawn_name);
    spawned_instance->set_meta(SNAME("_net_id"),p_network_id);
    // print_line(vformat("[%s] Internal spawn actually spawning net id %d with authority %d",get_multiplayer()->is_server() ? "SERVER" : "CLIENT", p_network_id,authority));
    //spawned_instance->connect("tree_entered",callable_mp(this,&YNet::set_authority_after_entered).bind(spawned_instance).bind(authority), CONNECT_ONE_SHOT);
    //spawned_instance->connect("ready",callable_mp(spawned_instance,&Node::set_multiplayer_authority).bind(authority).bind(true), CONNECT_ONE_SHOT);
    p_desired_parent_node->connect(SNAME("child_entered_tree"), callable_mp(this,&YNet::set_authority_after_entered).bind(p_spawn_pos,authority), CONNECT_ONE_SHOT);
    p_desired_parent_node->add_child(spawned_instance,true);
    spawned_instance->set_multiplayer_authority(authority);
    spawned_instance->connect(SceneStringName(tree_exited),callable_mp(this,&YNet::spawned_network_node_exited_tree).bind(p_network_id),CONNECT_ONE_SHOT);
    yrpc_to_node_hash_map[p_network_id] = spawned_instance->get_instance_id();
    networked_spawned_objects[p_network_id] = NetworkSpawnedObjectInfo{p_network_id, desired_spawn_path_id, p_spawn_name, spawned_instance->get_instance_id(), p_desired_parent, authority, _spawn_x, _spawn_y, _spawn_z};
    if (queued_networked_spawned_objects.has(p_network_id))
        queued_networked_spawned_objects.erase(p_network_id);
    return spawned_instance;
}

void YNet::set_authority_after_entered(Node* node_entered_tree, const Variant &p_spawn_pos, int authority ) {
    if (node_entered_tree != nullptr) {
        if (node_entered_tree->has_method("set_global_position")) {
            node_entered_tree->call("set_global_position",p_spawn_pos);
        }
        node_entered_tree->set_multiplayer_authority(authority,true);
    }
}

Node *YNet::spawn_with_path(const String &p_spawnable_scene_path, const String &p_spawn_name, const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority) {
    ERR_FAIL_COND_V_MSG(!get_multiplayer()->is_server(), nullptr, "ERROR: Attempted to spawn a networked node from a client");
    Ref<PackedScene> ps = ResourceLoader::load(p_spawnable_scene_path);
    ERR_FAIL_COND_V_MSG(ps.is_null() || !ps.is_valid(), nullptr, "ERROR: Attempted to spawn a invalid or null packed scene");
    return internal_spawn(get_new_network_id(),ps, p_spawn_name, p_desired_parent, p_spawn_pos, authority);
}
Node *YNet::spawn(const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name, const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority) {
    ERR_FAIL_COND_V_MSG(!get_multiplayer()->is_server(), nullptr, "ERROR: Attempted to spawn a networked node from a client");
    ERR_FAIL_COND_V_MSG(p_spawnable_scene.is_null() || !p_spawnable_scene.is_valid(), nullptr, "ERROR: Attempted to spawn a invalid or null packed scene");
    return internal_spawn(get_new_network_id(),p_spawnable_scene, p_spawn_name, p_desired_parent, p_spawn_pos, authority);
}

void YNet::rpc_request_spawned_nodes(int id_requesting) {
    Array p_arguments;
    p_arguments.push_back(create_spawned_lists_variant());
    int argcount = p_arguments.size();
    const Variant **argptrs = (const Variant **)alloca(sizeof(Variant *) * argcount);
    for (int i = 0; i < argcount; i++) {
        argptrs[i] = &p_arguments[i];
    }
    rpcp(id_requesting,rpc_respond_with_spawned_nodes_stringname,argptrs,argcount);
    //print_line(vformat("[%s] Received spawned nodes request from %d",get_multiplayer()->is_server() ? "SERVER" : "CLIENT",id_requesting));
}

void YNet::rpc_respond_with_spawned_nodes(const Array& spawned_nodes_info) {
    unpack_spawned_list_variants(spawned_nodes_info);
    //print_line(vformat("[%s] Received resulting spawned nodes response from server. In queue now: %d",get_multiplayer()->is_server() ? "SERVER" : "CLIENT",get_queued_spawn_count()));
    count_to_check_should_spawn = 999;
}

void YNet::rpc_despawn(int p_network_id) {
    if (yrpc_to_node_hash_map.has(p_network_id))
        yrpc_to_node_hash_map.erase(p_network_id);
    if (networked_property_syncers.has(p_network_id)) {
        networked_property_syncers.erase(p_network_id);
        if (queued_received_property_syncers.has(p_network_id)) {
            queued_received_property_syncers.erase(p_network_id);
        }
        if (queued_to_send_property_syncers.has(p_network_id)) {
            queued_to_send_property_syncers.erase(p_network_id);
        }
    }
    if (queued_networked_spawned_objects.has(p_network_id)) {
        queued_networked_spawned_objects.erase(p_network_id);
    }
    if(!networked_spawned_objects.has(p_network_id)) {
        return;
    }
    auto spawned_obj_info = networked_spawned_objects[p_network_id];
    if (spawned_obj_info.SpawnedNodeId.is_valid()) {
        auto find_instance = ObjectDB::get_instance(spawned_obj_info.SpawnedNodeId);
        if (find_instance != nullptr) {
            auto spawned_node = Object::cast_to<Node>(find_instance);
            if (spawned_node != nullptr) {
                networked_spawned_objects.erase(p_network_id);
                spawned_node->queue_free();
                return;
            }
        }
    }
    networked_spawned_objects.erase(p_network_id);
}

void YNet::despawn(int p_network_id) {
    ERR_FAIL_COND_MSG(!get_multiplayer()->is_server(), "ERROR: Attempted to despawn a networked node from a client");
    ERR_FAIL_COND_MSG(!queued_networked_spawned_objects.has(p_network_id) && !networked_spawned_objects.has(p_network_id), "ERROR: Attempted to despawn a networked node not present in the dictionary");
    rpc_despawn(p_network_id);
    Array p_arguments;
    p_arguments.push_back(p_network_id);
    int argcount = p_arguments.size();
    const Variant **argptrs = (const Variant **)alloca(sizeof(Variant *) * argcount);
    for (int i = 0; i < argcount; i++) {
        argptrs[i] = &p_arguments[i];
    }
    rpcp(0,rpc_despawn_stringname,argptrs,argcount);
}

void YNet::despawn_node(Node* node_to_despawn) {
    ERR_FAIL_COND_MSG(node_to_despawn == nullptr, "ERROR: Attempted to despawn a null node");
    if (!node_to_despawn->has_meta(SNAME("_net_id"))) {
        node_to_despawn->queue_free();
        return;
    }
    int net_id_despawning = node_to_despawn->get_meta(SNAME("_net_id"),0);
    despawn(net_id_despawning);
}

String YNet::server_or_client_str() {
    if (scene_multiplayer == nullptr)
        return "NULL";
    return scene_multiplayer->is_server() ? "SERVER" : "CLIENT";
}

void YNet::send_sync_vars(uint64_t p_cur_usec) {
	// Create base packet, lots of hardcode because it must be tight.
	int ofs = 0;

#define MAKE_ROOM(m_amount)             \
	if (packet_cache.size() < m_amount) \
		packet_cache.resize(m_amount);

	MAKE_ROOM(3);
	// The meta is composed along the way, so just set 0 for now.
	packet_cache.write[0] = 0;
    packet_cache.write[1] = 0;
    packet_cache.write[2] = 72;
	ofs += 3;

    uint8_t node_id_compression = UINT8_MAX;
    const auto amount_to_send = queued_to_send_property_syncers.size();

	if (amount_to_send >= 0 && amount_to_send <= 255) {
		// We can encode the id in 1 byte
		node_id_compression = NETWORK_NODE_ID_COMPRESSION_8;
		MAKE_ROOM(ofs + 1);
		packet_cache.write[ofs] = static_cast<uint8_t>(amount_to_send);
		ofs += 1;
	} else if (amount_to_send >= 0 && amount_to_send <= 65535) {
		// We can encode the id in 2 bytes
		node_id_compression = NETWORK_NODE_ID_COMPRESSION_16;
		MAKE_ROOM(ofs + 2);
		encode_uint16(static_cast<uint16_t>(amount_to_send), &(packet_cache.write[ofs]));
		ofs += 2;
	} else {
		// Too big, let's use 4 bytes.
		node_id_compression = NETWORK_NODE_ID_COMPRESSION_32;
		MAKE_ROOM(ofs + 4);
		encode_uint32(amount_to_send, &(packet_cache.write[ofs]));
		ofs += 4;
	}


    //This is Godot 4 C++ Code
    //I want to start a byte array for sending these variables for syncing across the network.
    for (const auto& propertysyncers: queued_to_send_property_syncers) {
        MAKE_ROOM(ofs + 4);
        encode_uint32(propertysyncers.key, &(packet_cache.write[ofs]));
        ofs += 4;

        MAKE_ROOM(ofs + 1);
        packet_cache.write[ofs] = static_cast<uint8_t>(propertysyncers.value.size());
        ofs += 1;

        // This key int is used to find the object that the variants belong to.
        // The value in this keyvalue is a hashmap<int,variant>, the int there identifies the index of the property and will never be bigger than a byte. the variant can be any type of variant.
        for (const auto& _property_info: propertysyncers.value) {

            MAKE_ROOM(ofs + 1);
            packet_cache.write[ofs] = static_cast<uint8_t>(_property_info.key);
            ofs += 1;

            int len;
            Error err = MultiplayerAPI::encode_and_compress_variant(_property_info.value, nullptr, len, false);
            ERR_FAIL_COND_MSG(err != OK, "Unable to encode sync value. THIS IS LIKELY A BUG IN THE ENGINE!");
            MAKE_ROOM(ofs + len);
            if (len) {
                MultiplayerAPI::encode_and_compress_variant(_property_info.value, &packet_cache.write[ofs], len, false);
                ofs += len;
            }
            // Here I need to store these values as byte to send it to sync as small as possible.
        }
    }

	ERR_FAIL_COND(node_id_compression > 3);

	// We can now set the meta
    packet_cache.write[0] = scene_multiplayer->NETWORK_COMMAND_RAW;
	packet_cache.write[1] = node_id_compression;

    if (debugging >= ALL)
        print_line(vformat("%s sending %d bytes",server_or_client_str(), ofs));

    scene_multiplayer->send_command(0,packet_cache.ptr(), ofs);
    queued_to_send_property_syncers.clear();
}

void YNet::on_received_peer_packet(int packet_from, const Vector<uint8_t> &packet_received) {
    int _packet_size = static_cast<int>(packet_received.size());
    if (debugging >= ALL)
        print_line(vformat("%s On Received peer packet from [%d] size [%d] packet byte 0 [%d]", server_or_client_str() ,packet_from,_packet_size,packet_received[0]));
    if (_packet_size > 4 && packet_received[1] == 72) {
        if (debugging >= ALL)
            print_line(vformat("%s Received packet with size %d, with %d as the first byte and %d as the second byte", server_or_client_str(), _packet_size, packet_received[2], packet_received[3]));
        received_sync_packet(packet_from, packet_received);
    }
}

void YNet::received_sync_packet(int received_from, const Vector<uint8_t> &p_buffer) {
    const int received_packet_size = static_cast<int>(p_buffer.size());
    uint8_t amount_type = p_buffer[0];
    int ofs = 2; // The spot where amount received starts.
    uint32_t amount_received = 0;
    if (amount_type == NETWORK_NODE_ID_COMPRESSION_8) {
        amount_received = p_buffer[ofs];
        ofs += 1;
    } else if (amount_type == NETWORK_NODE_ID_COMPRESSION_16) {
        amount_received = decode_uint16(&p_buffer[ofs]);
        ofs += 2;
    } else if (amount_type == NETWORK_NODE_ID_COMPRESSION_32) {
        amount_received = decode_uint32(&p_buffer[ofs]);
        ofs += 4;
    }
    for (int i = 0; i < static_cast<int>(amount_received); ++i) {
        int net_id = static_cast<int>(decode_uint32(&p_buffer[ofs]));
        if (debugging >= ALL)
            print_line(vformat("[%s] Received sync packet netid %d has network id to authority? %s %d received from %d",get_multiplayer()->is_server() ? "SERVER" : "CLIENT", net_id, networked_id_to_authority.has(net_id), networked_id_to_authority.has(net_id) ? networked_id_to_authority[net_id] : 0, received_from));
        bool can_receive_net_id = networked_id_to_authority.has(net_id) && networked_id_to_authority[net_id] == received_from;
        ofs += 4;
        uint8_t amount_of_properties_received = p_buffer[ofs];
        ofs += 1;
        for (int y = 0; y < amount_of_properties_received; ++y) {
            uint8_t property_index_received = p_buffer[ofs];
            ofs += 1;
            int vlen;
            Variant v;
            Error err = MultiplayerAPI::decode_and_decompress_variant(v, &p_buffer[ofs], received_packet_size - ofs, &vlen, false);
            ERR_FAIL_COND_MSG(err != OK, "Invalid packet received. Unable to decode state variable.");
            ofs += vlen;

            if (debugging >= ALL)
                print_line(vformat("Received for netid %d property %d value %s. From %d Can receive? %s",net_id,property_index_received,v,received_from, can_receive_net_id));
            if (can_receive_net_id) {
                if (networked_property_syncers.has(net_id) && property_index_received < networked_property_syncers[net_id].size()) {
                    // if (debugging >= ALL)
                    //     print_line("Setting current value for property index");
                    networked_property_syncers[net_id][property_index_received]->set_current_val(v);
                } else {
                    HashMap<uint8_t, Variant>& inner_map = queued_received_property_syncers[net_id];
                    inner_map[property_index_received] = v;
                }
            }
        }
    }
}

void YNet::rpc_recv_sync_vars(Variant p_synced_vars_data) {

}

YNet* YNet::engineio_connect(String _url) {
    if(!ynet_settings_enabled) {
        ERR_PRINT("[YNet] Attempting to connect while YNet is disabled on project settings");
    }
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
        print_line("[YNet] Connecting... url in ",_url," url out ",url," valid websockets client? ",client.is_valid());

    if (!client.is_valid()) {
        YNet::create_client();
        if(debugging == ALL)
            print_line("[YNet] WebSocket created? ",client.is_valid());
    } else {
        if (static_cast<State>(client->get_ready_state()) != STATE_CLOSED) {
            engineio_disconnect();
            ERR_PRINT("[YNet] Called connect when already connected");
            emit_signal(SNAME("connected"),"",false);
            return this;
        }
    }

    ERR_FAIL_COND_V(client.is_null(), this);

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
        emit_signal(SNAME("connected"),"",false);
    }

    ERR_FAIL_COND_V(err != OK, this);
    set_process(true);
    return this;
}

void YNet::create_client() {
    if (!client.is_valid()) {
        client = Ref<WebSocketPeer>(WebSocketPeer::create());
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
            scene_multiplayer = get_multiplayer();
            scene_multiplayer->connect(SNAME("peer_packet"), callable_mp(this, &YNet::on_received_peer_packet));
            scene_multiplayer->connect(SNAME("connected_to_server"),callable_mp(this,&YNet::on_connected_to_server));
            this->rpc_config(receive_yrpc_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED, false, 0));
            this->rpc_config(receive_yrpc_also_local_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED, true, 0));
            this->rpc_config(receive_yrpc_reliable_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_RELIABLE, false, 0));
            this->rpc_config(receive_yrpc_unreliable_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_UNRELIABLE, false, 0));
            this->rpc_config(receive_yrpc_unreliable_also_local_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_UNRELIABLE, true, 0));
            this->rpc_config(receive_yrpc_reliable_also_local_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_RELIABLE, true, 0));
            this->rpc_config(rpc_spawn_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_AUTHORITY, MultiplayerPeer::TRANSFER_MODE_RELIABLE, false, 0));
            this->rpc_config(rpc_despawn_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_AUTHORITY, MultiplayerPeer::TRANSFER_MODE_RELIABLE, false, 0));
            this->rpc_config(rpc_request_spawned_nodes_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_ANY_PEER, MultiplayerPeer::TRANSFER_MODE_RELIABLE, false, 0));
            this->rpc_config(rpc_respond_with_spawned_nodes_stringname,create_rpc_dictionary_config(MultiplayerAPI::RPC_MODE_AUTHORITY, MultiplayerPeer::TRANSFER_MODE_RELIABLE, false, 0));
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

void YNet::spawned_network_node_exited_tree(int p_nid) {
    if (networked_property_syncers.has(p_nid)) {
        networked_property_syncers.erase(p_nid);
    }
    if (yrpc_to_node_hash_map.has(p_nid)) {
        // Clear RPC config cache for this node
        ObjectID node_id = yrpc_to_node_hash_map[p_nid];
        if (node_id.is_valid()) {
            rpc_config_cache.erase(node_id);
        }
        yrpc_to_node_hash_map.erase(p_nid);
    }
    if (networked_spawned_objects.has(p_nid)) {
        networked_spawned_objects.erase(p_nid);
        const auto _multiplayer = get_multiplayer();
        if (_multiplayer.is_valid() && get_multiplayer()->is_server()) {
            Array p_arguments;
            p_arguments.push_back(p_nid);
            int argcount = p_arguments.size();
            const Variant **argptrs = (const Variant **)alloca(sizeof(Variant *) * argcount);
            for (int i = 0; i < argcount; i++) {
                argptrs[i] = &p_arguments[i];
            }
            rpcp(0,rpc_despawn_stringname,argptrs,argcount);
        }
    }
}

void YNet::clear_all_spawned_network_nodes() {
    Vector<ObjectID> nodes_to_despawn;
    for (const auto& nsob: networked_spawned_objects) {
        if (nsob.value.SpawnedNodeId.is_valid()) {
            nodes_to_despawn.push_back(nsob.value.SpawnedNodeId);
        }
    }
    networked_spawned_objects.clear();
    for (auto to_despawn: nodes_to_despawn) {
        Node *actual_node = Object::cast_to<Node>(ObjectDB::get_instance(to_despawn));
        if (actual_node != nullptr) {
            actual_node->queue_free();
        }
    }
}

void YNet::on_connected_to_server() {
    //print_line("on connect to server");
    if (!get_multiplayer()->is_server()) {
        rpc(rpc_request_spawned_nodes_stringname,get_multiplayer()->get_unique_id());
    }
}


Dictionary YNet::create_rpc_dictionary_config(MultiplayerAPI::RPCMode p_rpc_mode, MultiplayerPeer::TransferMode p_transfer_mode, bool p_call_local, int p_channel) {
    Dictionary _dict_config;
    _dict_config["rpc_mode"] = p_rpc_mode;
    _dict_config["transfer_mode"] = p_transfer_mode;
    _dict_config["call_local"] = p_call_local;
    _dict_config["channel"] = p_channel;
    return _dict_config;
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

bool YNet::engineio_decode_packet(const uint8_t *packet, int len) {
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

Ref<PackedScene> YNet::find_network_spawnable(uint32_t new_spawnable_id) {
    if (spawnables_dictionary.has(new_spawnable_id)) {
        return ResourceLoader::load(spawnables_dictionary[new_spawnable_id]);
    }
    return nullptr;
}

Node * YNet::find_node_with_net_id(int p_net_id) {
    if (yrpc_to_node_hash_map.has(p_net_id)) {
        const ObjectID desiredId = yrpc_to_node_hash_map[p_net_id];
        if (!desiredId.is_null() && desiredId.is_valid()) {
            Node *actual_node = Object::cast_to<Node>(ObjectDB::get_instance(desiredId));
            if (actual_node != nullptr) {
                return actual_node;
            }
        }
    }
    return nullptr;
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

void YNet::update_last_engine_state() {
    if(offline_mode) {
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

    State client_state = static_cast<State>(client->get_ready_state());

    if (client_state == STATE_OPEN) {
        process_packets();
        update_networked_property_syncers();
    } else if(client_state == STATE_CLOSING) {
    } else if(client_state == STATE_CLOSED) {
        emit_signal(SNAME("disconnected"),client->get_close_code(),client->get_close_reason());
        if (status == STATE_OPEN) {
            //emit_signal("disconnected",slash_namespace);
        }
        set_current_state(STATE_CLOSED);
        if (debugging == ALL) {
            print_line("[YNet] Closed connection, stoppping process");
        }
        set_process(false);
    }
}

void YNet::update_networked_property_syncers() {
    count_to_check_should_spawn += 1;
    if (count_to_check_should_spawn > 30) {
        count_to_check_should_spawn = 0;
        if (!queued_networked_spawned_objects.is_empty() && !pause_receive_spawns) {
            Vector<NetworkSpawnedObjectInfo> infos_to_spawn;
            for (const auto& queued_spawnables: queued_networked_spawned_objects) {
                auto _get_node_or_null = get_node_or_null(queued_spawnables.value.desired_parent);
                if (_get_node_or_null != nullptr) {
                    infos_to_spawn.push_back(queued_spawnables.value);
                }
            }
            for (const auto& to_spawn: infos_to_spawn) {
                internal_spawn_with_queued_struct(to_spawn);
            }
        }
    }

    if (!scene_multiplayer.is_null() && scene_multiplayer.is_valid() && scene_multiplayer->has_multiplayer_peer() && !networked_property_syncers.is_empty()) {
        uint64_t usec = OS::get_singleton()->get_ticks_usec();
        if (last_watched_synced_vars > usec) last_watched_synced_vars = usec;
        if (last_sent_synced_vars > usec) last_sent_synced_vars = usec;
        if (debugging >= ALL)
            print_line(vformat("%s last watched sync vars %d + interval %d < usec %d     [ %d < %d ]    %s",server_or_client_str(),last_watched_synced_vars,watch_synced_vars_interval,usec,last_watched_synced_vars + watch_synced_vars_interval,usec,last_watched_synced_vars + watch_synced_vars_interval < usec));
        if (last_watched_synced_vars + watch_synced_vars_interval < usec) {
            last_watched_synced_vars = usec;
            int my_unique_id = scene_multiplayer->get_unique_id();
            for (const auto& netpropsync: networked_property_syncers) {
                // if (debugging >= ALL)
                //     print_line(vformat("%s my id %d, networked id to authority has %d? %s is my id? %s", server_or_client_str(), my_unique_id, netpropsync.key, networked_id_to_authority.has(netpropsync.key), networked_id_to_authority.has(netpropsync.key) && networked_id_to_authority[netpropsync.key] == my_unique_id));
                if (networked_id_to_authority.has(netpropsync.key) && networked_id_to_authority[netpropsync.key] == my_unique_id) {
                    auto syncproperties = netpropsync.value;
                    for (int i = 0; i < syncproperties.size(); ++i) {
                        Variant value_before = syncproperties[i]->current_val;
                        if(syncproperties[i]->check_for_changed_value() || syncproperties[i]->sync_always) {
                            // If the outer_key doesn't exist in the outer HashMap, it would default construct an inner HashMap
                            HashMap<uint8_t, Variant>& inner_map = queued_to_send_property_syncers[netpropsync.key];

                            // Add (or modify) the value in the inner HashMap
                            inner_map[syncproperties[i]->property_syncer_index] = syncproperties[i]->current_val;
                        }
                        // if (debugging >= ALL)
                        //     print_line(vformat("%s my id %d. netid %d property index %d value now %s value after %s, is different? %s",server_or_client_str(),my_unique_id, syncproperties[i]->net_id,syncproperties[i]->property_syncer_index, value_before, syncproperties[i]->current_val, value_before != syncproperties[i]->current_val));
                    }
                }
            }
           // print_line("Watching synced properties");
        }
        if (last_sent_synced_vars + send_synced_vars_interval < usec && !queued_to_send_property_syncers.is_empty()) {
            last_sent_synced_vars = usec;
            send_sync_vars(usec);
            if (debugging >= ALL)
                print_line(server_or_client_str(), "Sent sync vars maybe");
        }
    }
}

int YNet::get_max_queued_packets() {
    if (!client.is_valid()) {
        YNet::create_client();
    }
    return client->get_max_queued_packets();
}

YNet* YNet::create_room() {
    socketio_send("requestroom",protocol);
    return this;
}

YNet* YNet::create_room_with_code(const String &create_room) {
    Array payload;
    payload.push_back(create_room);
    payload.push_back(protocol);
    socketio_send("createroomwithcode",payload);
    return this;
}

YNet* YNet::join_or_create_room(const String &join_room) {
    if (join_room.is_empty()) {
        socketio_send("joinOrCreateRoomRandom",protocol);
    } else {
        socketio_send("joinOrCreateRoom",join_room + protocol);
    }
    return this;
}

YNet* YNet::join_room(const String &join_room) {
    socketio_send("joinroom",join_room + protocol);
    return this;
}

YNet* YNet::leave_room() {
    socketio_send("leaveroom");
    clear_unhandled_packets();
    room_id = "";
    return this;
}

YNet* YNet::join_room_with_password(const String &roomCode, const String &password) {
    Array payload;
    payload.push_back(roomCode);
    payload.push_back(password);
    socketio_send("joinroomwithpassword", payload);
    return this;
}

Error YNet::set_password(const String &newPassword) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(newPassword);
    socketio_send("set_password",  payload);
    return OK;
}

Error YNet::set_max_players(int newMaxPlayers) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(newMaxPlayers);
    socketio_send("set_max_players", payload);
    return OK;
}

Error YNet::set_private(bool newPrivate) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(newPrivate);
    socketio_send("set_private", payload);
    return OK;
}

Error YNet::set_can_host_migrate(bool newCanHostMigrate) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(newCanHostMigrate);
    socketio_send("set_can_host_migrate", payload);
    return OK;
}

Error YNet::set_room_name(const String &newRoomName) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(newRoomName);
    socketio_send("set_room_name", payload);
    return OK;
}

Error YNet::set_extra_info(const String &new_extra_info) {
    Array payload;
    payload.push_back(get_room_id());
    payload.push_back(new_extra_info);
    socketio_send("set_extra_info", payload);
    return OK;
}

YNet* YNet::get_room_info(const String &roomCode) {
    socketio_send("get_room_info", roomCode.is_empty() ? room_id : roomCode);
    return this;
}

YNet* YNet::get_room_list() {
    socketio_send("get_room_list", protocol);
    return this;
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

void YNet::on_room_info(const Variant &p_room_info) {
    emit_signal("room_info",p_room_info);
}

void YNet::on_room_list(const Variant &p_room_list) {
    emit_signal("room_list",p_room_list);
}

void YNet::on_room_players(const Array &players_array) {
    if(debugging > 1)
        print_line(vformat("on_room_players %s",players_array));
    for (int i = 0; i < players_array.size(); i++)
        if (const auto& _player = players_array[i]; _player.get_type() == Variant::STRING && !connections_map.has(_player))
            on_player_join(_player);

    emit_signal(SNAME("room_players"),players_array);
}

void YNet::on_player_join(const String &p_player) {
    if(debugging > 1)
        print_line("on_player_join ",p_player," ",string_to_hash_id(p_player));
    connections_map[p_player] = string_to_hash_id(p_player);
    emit_signal(SNAME("player_joined"),p_player);
}

void YNet::on_received_pkt(const String &received_from, const String &pkt_content) {
    if(debugging > DebuggingLevel::MOSTMESSAGES)
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

    YNetTypes::Packet packet;
    packet.data = (uint8_t *)memalloc(arr_len);
    memcpy(packet.data, buf.ptrw(), arr_len);
    packet.size = arr_len;
    packet.source = connections_map.has(received_from) ? static_cast<int>(connections_map[received_from]) : static_cast<int>(string_to_hash_id(received_from));
    unhandled_packets.push_back(packet);
}

void YNet::on_player_left(const String &p_player) {
    if(debugging > 1)
        print_line("on_player_left ",p_player);
    if (connections_map.has(p_player))
        connections_map.erase(p_player);
    emit_signal(SNAME("player_left"),p_player);
}

#ifdef HOST_MIGRATION
void YNet::on_host_migrated(const String &p_new_host) {
    if(debugging > 1)
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
#endif

void YNet::register_for_yrpcs(Node *p_registering_node, int registering_id) {
    if (!yrpc_to_node_hash_map.has(registering_id)) {
        yrpc_to_node_hash_map[registering_id] = p_registering_node->get_instance_id();
        Callable remove_callable = callable_mp(this,&YNet::remove_from_yrpc_receiving_map).bind(registering_id);
        if (!p_registering_node->is_connected(SceneStringName(tree_exiting), remove_callable)) {
            p_registering_node->connect(SceneStringName(tree_exiting), remove_callable, CONNECT_ONE_SHOT);
        }
        p_registering_node->set_meta(SNAME("_net_id"), registering_id);
    }
}

uint32_t YNet::string_to_hash_id(const String &p_string) {
        /* simple djb2 hashing */

        const char32_t *chr = p_string.get_data();
        uint32_t hashv = 5381;
        uint32_t c = *chr++;

        while (c) {
            hashv = (((hashv) << 5) + hashv) + c; /* hash * 33 + c */
            c = *chr++;
        }

        hashv = hash_fmix32(hashv);
        hashv = hashv & 0x7FFFFFFF; // Make it compatible with unsigned, since negative ID is used for exclusion
        return hashv;
}

YNet::YNet() {
    pause_receive_spawns = false;
    receive_yrpc_stringname = "receive_yrpc";
    receive_yrpc_also_local_stringname = "receive_yrpc_call_local";
    receive_yrpc_reliable_stringname = "receive_yrpc_reliable";
    receive_yrpc_reliable_also_local_stringname = "receive_yrpc_reliable_call_local";
    receive_yrpc_unreliable_stringname = "receive_yrpc_unreliable";
    receive_yrpc_unreliable_also_local_stringname = "receive_yrpc_unreliable_call_local";
    rpc_spawn_stringname = "rpc_spawn";
    rpc_despawn_stringname = "rpc_despawn";
    rpc_request_spawned_nodes_stringname = "rpc_request_spawned_nodes";
    rpc_respond_with_spawned_nodes_stringname = "rpc_respond_with_spawned_nodes";
    rpc_rec_synced_vars_stringname = "rpc_recv_sync_vars";
    singleton = this;
    max_queued_packets = 2048;

    last_sent_synced_vars = 0ul;
    last_watched_synced_vars = 0ul;

    call_deferred("setup_node");
}

void YNet::clear_unhandled_packets() {
    for (YNetTypes::Packet &E : unhandled_packets) {
        memfree(E.data);
        E.data = nullptr;
    }
    unhandled_packets.clear();
}

YNet::~YNet() {
    clear_unhandled_packets();
    connections_map.clear();
    if (client.is_valid()) {
        client.unref();
    }
    if (singleton != nullptr && singleton == this) {
        // print_line("Removing ynet singleton");
        if (Engine::get_singleton()->has_singleton("YNet")) {
            Engine::get_singleton()->remove_singleton("YNet");
        }
        singleton = nullptr;
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

// Add this helper function before _send_yrpc
struct YNetRPCConfig {
    MultiplayerAPI::RPCMode rpc_mode;
    MultiplayerPeer::TransferMode transfer_mode;
    bool call_local;
    int channel;
};

YNet::YNetRPCConfig YNet::_get_rpc_config(Node *p_node, const StringName &p_method) {
    const ObjectID oid = p_node->get_instance_id();
    
    // If we don't have a cache for this node yet, create it
    if (!rpc_config_cache.has(oid)) {
        YNetRPCConfigCache cache;
        
        // Parse node config
        Dictionary node_config = p_node->get_rpc_config();
        _parse_rpc_config(node_config, true, cache);
        
        // Parse script config if it exists
        if (p_node->get_script_instance()) {
            Dictionary script_config = p_node->get_script_instance()->get_rpc_config();
            _parse_rpc_config(script_config, false, cache);
        }
        
        rpc_config_cache[oid] = cache;
    }
    
    // Get the config from cache
    YNetRPCConfigCache &cache = rpc_config_cache[oid];
    if (cache.ids.has(p_method)) {
        uint16_t id = cache.ids[p_method];
        return cache.configs[id];
    }
    
    // Default config if not found
    YNetRPCConfig config;
    config.name = p_method;
    config.rpc_mode = MultiplayerAPI::RPC_MODE_DISABLED;
    config.transfer_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    config.call_local = false;
    config.channel = 0;
    return config;
}

void YNet::set_debug_run_multiple_instances(bool val) {
    #ifdef TOOLS_ENABLED
    if (RunInstancesDialog::get_singleton() != nullptr) {
        TypedArray<Node> checkbox_children = RunInstancesDialog::get_singleton()->find_children("*","CheckBox");
		for (int i = 0; i < checkbox_children.size(); i++) {
			Node *cb = cast_to<Node>(checkbox_children[i]);
            if (cb != nullptr) {
                cb->set("button_pressed",val);
            }
        }
    }
    #endif
}

bool YNet::get_debug_run_multiple_instances() {
    #ifdef TOOLS_ENABLED
    if (RunInstancesDialog::get_singleton() != nullptr) {
        TypedArray<Node> checkbox_children = RunInstancesDialog::get_singleton()->find_children("*","CheckBox");
		for (int i = 0; i < checkbox_children.size(); i++) {
			Node *cb = cast_to<Node>(checkbox_children[i]);
            if (cb != nullptr) {
                return cb->get("button_pressed").operator bool();
            }
        }
    }
    #endif
    return false;
}

void YNet::_parse_rpc_config(const Dictionary &p_config, bool p_for_node, YNetRPCConfigCache &r_cache) {
    Array names = p_config.keys();
    names.sort_custom(callable_mp_static(&StringLikeVariantOrder::compare)); // Ensure ID order
    for (int i = 0; i < names.size(); i++) {
        ERR_CONTINUE(!names[i].is_string());
        String name = names[i].operator String();
        ERR_CONTINUE(p_config[name].get_type() != Variant::DICTIONARY);
        ERR_CONTINUE(!p_config[name].operator Dictionary().has("rpc_mode"));
        Dictionary dict = p_config[name];
        YNetRPCConfig cfg;
        cfg.name = name;
        cfg.rpc_mode = ((MultiplayerAPI::RPCMode)dict.get("rpc_mode", MultiplayerAPI::RPC_MODE_AUTHORITY).operator int());
        cfg.transfer_mode = ((MultiplayerPeer::TransferMode)dict.get("transfer_mode", MultiplayerPeer::TRANSFER_MODE_RELIABLE).operator int());
        cfg.call_local = dict.get("call_local", false).operator bool();
        cfg.channel = dict.get("channel", 0).operator int();
        uint16_t id = ((uint16_t)i);
        if (p_for_node) {
            id |= (1 << 15);
        }
        r_cache.configs[id] = cfg;
        r_cache.ids[name] = id;
    }
}
