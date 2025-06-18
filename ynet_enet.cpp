#include "ynet_enet.h"

void YNetEnet::_bind_methods() {
    // No additional bindings needed as all methods are inherited from YNetTransport
}

YNetEnet::YNetEnet() {
    connection = Ref<ENetConnection>();
    connection.instantiate();
    peer = nullptr;
    status = STATE_CLOSED;
    debugging = YNet::get_singleton()->debugging;
    
    if (debugging >= YNet::DebuggingLevel::MINIMAL) {
        print_line("[YNet] ENet transport initialized successfully");
    }
}

YNetEnet::~YNetEnet() {
    transport_disconnect();
    
    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }
    
    connections_map.clear();
    clear_unhandled_packets();
    connection = Ref<ENetConnection>();
}

String YNetEnet::MessageTypeToString(uint8_t type) {
    switch (type) {
        case 0: return "ERROR";
        case 1: return "ROOM_CREATE";
        case 2: return "ROOM_JOIN";
        case 3: return "ROOM_LEAVE";
        case 4: return "RPC_TO_SERVER";
        case 5: return "RPC_TO_CLIENTS";
        case 6: return "RPC_TO_CLIENT";
        case 7: return "MESSAGE";
        case 8: return "GET_ROOM_LIST";
        case 9: return "CONFIRM_ROOM_CREATION";
        case 10: return "CONFIRM_ROOM_JOIN";
        case 11: return "PLAYER_JOINED";
        case 12: return "ROOM_PEER_JOIN";
        case 13: return "ROOM_PEER_LEFT";
        case 14: return "SET_ROOM_PASSWORD";
        case 15: return "SET_MAX_PLAYERS";
        case 16: return "SET_ROOM_PRIVATE";
        case 17: return "SET_HOST_MIGRATION";
        case 18: return "SET_ROOM_NAME";
        case 19: return "SET_EXTRA_INFO";
        case 20: return "GET_ROOM_INFO";
        case 21: return "PACKET_TO_SERVER";
        case 22: return "PACKET_TO_CLIENTS";
        case 23: return "PACKET_TO_CLIENT";
        case 24: return "KICK_CLIENT";
        case 25: return "JOIN_ROOM_WITH_PASSWORD";
        case 26: return "JOIN_OR_CREATE_ROOM";
        case 27: return "JOIN_OR_CREATE_ROOM_RANDOM";
        case 28: return "PLAYER_LEFT";
        case 29: return "CREATE_ROOM_WITH_PASSWORD";
        case 30: return "HOST_LEFT";
        default: return "UNKNOWN";
    }
}

void YNetEnet::clear_unhandled_packets() {
    for (YNetTypes::Packet &E : unhandled_packets) {
        if (E.data != nullptr) {
            memfree(E.data);
            E.data = nullptr;
        }
    }
    unhandled_packets.clear();
}

Error YNetEnet::connect_to(const String &p_url) {
    if (debugging >= YNet::DebuggingLevel::MINIMAL) {
        print_line("[YNet] Connecting to ", p_url);
    }

    // Parse URL for address and port
    String address = p_url;
    uint16_t port = 7777; // Default port

    // Handle URL parsing
    if (p_url.contains(":")) {
        Vector<String> parts = p_url.split(":");
        address = parts[0];
        port = parts[1].to_int();
    }

    if (peer.is_valid()) {
        YNet::get_singleton()->emit_signal(SNAME("connected"),"FAILURE ALREADY CONNECTED",false);
        ERR_PRINT("[YNet] Tried to connect while already connected");
        transport_disconnect();
    }

    // Create client host
    Error err = connection->create_host(1, 2);
    if (err != OK) {
        YNet::get_singleton()->emit_signal(SNAME("connected"),"FAILURE",false);
        ERR_PRINT("[YNet] Failed to create ENet client host");
        return err;
    }

    // Enable compression
    connection->compress(ENetConnection::COMPRESS_RANGE_CODER);

    Ref<ENetPacketPeer> temp_peer = connection->connect_to_host(address, port, 2, YNet::get_singleton()->protocol_hash);
    
    if (temp_peer.is_null() ||!temp_peer.is_valid()) {
        YNet::get_singleton()->emit_signal(SNAME("connected"),"FAILURE",false);
        ERR_PRINT("[YNet] Failed to initiate ENet connection");
        connection->destroy();
        connection = Ref<ENetConnection>();
        return ERR_CANT_CONNECT;
    }
    
    peer = temp_peer;
    peer->ping_interval(2000); // Send ping every 2 seconds
    peer->set_timeout(32, 10000, 30000);

    // Configure ping and timeout settings for better compatibility with raw ENet servers
    set_current_state(STATE_CONNECTING);

    YNet::get_singleton()->set_process(true);

    // if (debugging >= YNet::DebuggingLevel::MINIMAL) {
        print_line("[YNet] Initiating connection to ", address, ":", port);
    // }

    return OK;
}

void YNetEnet::transport_disconnect() {
    if (peer.is_valid()) {
        peer->peer_disconnect(0);
        peer = Ref<ENetPacketPeer>();
    }

    if (connection.is_valid()) {
        connection->flush();
        connection->destroy();
        connection = Ref<ENetConnection>();
    }

    set_current_state(STATE_CLOSED);
    clear_unhandled_packets();
}

void YNetEnet::transport_process(YNet* ynet) {
    if (!connection.is_valid()) {
        return;
    }

    if (status == STATE_CONNECTING) {
        const float current_time = OS::get_singleton()->get_ticks_msec() * 0.001f;
        if (current_time > tick_started_connecting + 6.0f) {
            was_timeout=true;
            YNet::get_singleton()->emit_signal(SNAME("connected"),"TIMEOUT",false);
            transport_disconnect();
            return;
        }
    }

    ENetConnection::Event event;
    ENetConnection::EventType ret = connection->service(0, event);
    
    do {
        switch (ret) {
            case ENetConnection::EVENT_CONNECT: {
                if (debugging >= YNet::DebuggingLevel::MINIMAL) {
                    print_line("[YNet] Connection established, pending receiving SID");
                }
                // Don't set peer again - we already have it from connect_to_host
                set_current_state(STATE_OPEN);
            } break;

            case ENetConnection::EVENT_RECEIVE: {
                // Handle the message
                if (event.packet != nullptr) {
                    handle_received_message(event.packet->data, event.packet->dataLength);
                }
            } break;

            case ENetConnection::EVENT_DISCONNECT: {
                if (debugging >= YNet::DebuggingLevel::MINIMAL) {
                    print_line("[YNet] Disconnected from server");
                }
                peer = Ref<ENetPacketPeer>();
                if (status == STATE_CONNECTING) {
                    YNet::get_singleton()->emit_signal(SNAME("connected"),"CONNECTION CLOSED BY SERVER",false);
                }
                set_current_state(STATE_CLOSED);
                YNet::get_singleton()->emit_signal(SNAME("disconnected"), "", "Connection closed by server");
            } break;

            case ENetConnection::EVENT_ERROR: {
                if (debugging >= YNet::DebuggingLevel::MINIMAL) {
                    print_line("[YNet] Connection error occurred");
                }
                if (status == STATE_CONNECTING) {
                    YNet::get_singleton()->emit_signal(SNAME("connected"),"CONNECTION ERROR",false);
                }
                set_current_state(STATE_CLOSED);
            } break;

            case ENetConnection::EVENT_NONE: {
            } break;
        }
    } while (connection.is_valid() && connection->check_events(ret, event) > 0);

    if (get_current_state() == STATE_OPEN) {
        YNet::get_singleton()->update_networked_property_syncers();
    }
}

void YNetEnet::send_message(Ref<YNetMessage> p_message) {
    if (!peer.is_valid() || get_state() != STATE_OPEN) return;
    PackedByteArray serialized = p_message->serialize();
    
    // Create ENet packet with reliable flag
    ENetPacket *packet = enet_packet_create(
        serialized.ptr(),
        serialized.size(),
        ENetPacketPeer::FLAG_RELIABLE
    );
    
    int err = peer->send(0, packet);
    connection->flush();
    if (err < 0) {
        ERR_PRINT("[YNet] Failed to send message");
    }
}

void YNetEnet::handle_received_message(const uint8_t* data, size_t dataLength) {
    if (dataLength < 1) return;

    uint8_t type = data[0];
    
    if (debugging >= YNet::ALL) {
        print_line("[YNet] Received message type: ", MessageTypeToString(type));
    }

    PackedByteArray packet_data;
    packet_data.resize(dataLength);
    memcpy(packet_data.ptrw(), data, dataLength);

    switch (type) {
        case YNetMessage::MessageType::CONFIRM_CONNECTION: { // CONFIRM_CONNECTION
            Ref<YNetConfirmConnectionMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                YNet::get_singleton()->sid = vformat("%d", msg->clientId);
                YNet::get_singleton()->hashed_sid = msg->clientId;
                YNet::get_singleton()->real_hashed_sid = msg->clientId;
                set_current_state(STATE_OPEN);
                if (debugging >= YNet::DebuggingLevel::MINIMAL) {
                    print_line(vformat("[YNet] Connection established, SID received %d (Hashed: %d)", msg->clientId, YNet::get_singleton()->hashed_sid));
                }
                YNet::get_singleton()->emit_signal(SNAME("connected"), "", true);
            }
        } break;
        case YNetMessage::MessageType::CONFIRM_ROOM_CREATION: { // CONFIRM_ROOM_CREATION
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                YNet::get_singleton()->on_room_created(msg->data);
            }
        } break;

        case YNetMessage::MessageType::CONFIRM_ROOM_JOIN: { // CONFIRM_ROOM_JOIN
            Ref<YNetConfirmRoomJoinMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                Array players = JSON::parse_string(msg->jsonRoomPlayers);
                YNet::get_singleton()->on_room_joined(msg->roomCode, players[0]);
                YNet::get_singleton()->on_room_players(players);
            }
        } break;

        case YNetMessage::MessageType::PLAYER_JOINED: { // PLAYER_JOINED
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                YNet::get_singleton()->on_player_join(msg->data);
            }
        } break;

        case YNetMessage::MessageType::PLAYER_LEFT: { // PLAYER_LEFT
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                YNet::get_singleton()->on_player_left(msg->data);
            }
        } break;

        case YNetMessage::MessageType::HOST_LEFT: { // HOST_LEFT
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                // YNet::get_singleton()->on_host_migrated(msg->data);
            }
        } break;

        case YNetMessage::MessageType::MESSAGE_ERROR: { // ERROR
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                YNet::get_singleton()->on_room_error(msg->data);
            }
        } break;

        case 8: { // GET_ROOM_LIST
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                Array room_list = JSON::parse_string(msg->data);
                YNet::get_singleton()->on_room_list(room_list);
            }
        } break;

        case 20: { // GET_ROOM_INFO
            Ref<YNetMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                Dictionary room_info = JSON::parse_string(msg->data);
                YNet::get_singleton()->on_room_info(room_info);
            }
        } break;

        case YNetMessage::MessageType::PACKET_TO_CLIENTS: // PACKET_TO_CLIENTS
        case YNetMessage::MessageType::PACKET_TO_CLIENT:
        case YNetMessage::MessageType::PACKET_TO_SERVER: { // PACKET_TO_CLIENT
            Ref<YNetPacketMessage> msg;
            msg.instantiate();
            if (msg->deserialize(packet_data)) {
                if (msg->targetClientId == YNet::get_singleton()->host_id_hashed) {
                    msg->targetClientId = 1;
                }
                if (debugging >= YNet::DebuggingLevel::ALL) {
                    print_line(vformat("Received ynetpacketmmessage from %d type %d data %s", msg->targetClientId, msg->type, msg->packetData));
                }
                YNetTypes::Packet packet;
                packet.data = (uint8_t *)memalloc(msg->packetData.size());
                memcpy(packet.data, msg->packetData.ptr(), msg->packetData.size());
                packet.size = msg->packetData.size();
                packet.source = msg->targetClientId;
                unhandled_packets.push_back(packet);
            } else {
                print_error("Couldn't deserialize packet");
            }
        } break;
    }
}

uint32_t YNetEnet::string_to_hash_id(const String &p_string) {
    // I know that the string is a number that complies with uint16_t.   
    return p_string.to_int();
}
Error YNetEnet::send_packet(const uint8_t *p_data, int p_size) {
    if (!peer.is_valid() || get_state() != STATE_OPEN) {
        return ERR_UNCONFIGURED;
    }

    Ref<YNetPacketMessage> msg;
    msg.instantiate();
    msg->targetClientId = YNet::get_singleton()->get_target_peer();
    if (msg->targetClientId <= 0) {
        msg->type = (YNetMessage::MessageType::PACKET_TO_CLIENTS);
    } else if (msg->targetClientId == 1) {
        msg->type = (YNetMessage::MessageType::PACKET_TO_SERVER);
    } else {
        msg->type = (YNetMessage::MessageType::PACKET_TO_CLIENT);
    }
    msg->packetData.resize(p_size);
    msg->channel = YNet::get_singleton()->get_transfer_channel();
    auto reliability = YNet::get_singleton()->get_transfer_mode();
    msg->reliability = (reliability == MultiplayerPeer::TransferMode::TRANSFER_MODE_RELIABLE) ? 
                ENetPacketPeer::FLAG_RELIABLE : ENetPacketPeer::FLAG_UNSEQUENCED;
    memcpy(msg->packetData.ptrw(), p_data, p_size);
    PackedByteArray serialized = msg->serialize();
    
    // Create ENet packet with appropriate flags
    int flags = (msg->reliability == MultiplayerPeer::TransferMode::TRANSFER_MODE_RELIABLE) ? 
                ENetPacketPeer::FLAG_RELIABLE : ENetPacketPeer::FLAG_UNSEQUENCED;
    
    ENetPacket *packet = enet_packet_create(
        serialized.ptr(),
        serialized.size(),
        flags
    );
    
    int err = peer->send(msg->channel, packet);
    connection->flush();
    if (err < 0) {
        return ERR_CANT_CREATE;
    }

    return OK;
}

void YNetEnet::create_room() {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::ROOM_CREATE); // ROOM_CREATE
    msg->data = "";
    send_message(msg);
}

void YNetEnet::create_room_with_code(const String &room_code, const String &password) {
    if (get_state() != STATE_OPEN) return;
    if (!password.is_empty()) {
        Ref<YNetJoinRoomWithPasswordMessage> msg;
        msg.instantiate();
        msg->type = (YNetMessage::MessageType::CREATE_ROOM_WITH_PASSWORD); // ROOM_CREATE
        msg->roomCode = room_code;
        msg->password = password;
        send_message(msg);
    } else {
        Ref<YNetMessage> msg;
        msg.instantiate();
        msg->type = (YNetMessage::MessageType::ROOM_CREATE); // ROOM_CREATE
        msg->data = room_code;
        send_message(msg);
    }
}

void YNetEnet::join_room(const String &room_code, const String &password) {
    if (get_state() != STATE_OPEN) return;
    Ref<YNetJoinRoomWithPasswordMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::JOIN_ROOM_WITH_PASSWORD); // ROOM_JOIN
    msg->roomCode = room_code;
    msg->password = password;
    send_message(msg);
}

void YNetEnet::leave_room() {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::ROOM_LEAVE); // ROOM_LEAVE
    send_message(msg);

    clear_unhandled_packets();
    YNet::get_singleton()->room_id = "";
}

void YNetEnet::join_or_create_room(const String &room_code, const String &password) {
    if (get_state() != STATE_OPEN) return;

    if (room_code.is_empty()){
        Ref<YNetMessage> msg;
        msg->type = (YNetMessage::MessageType::JOIN_OR_CREATE_ROOM_RANDOM);
        send_message(msg);
    } else {
        Ref<YNetJoinRoomWithPasswordMessage> msg;
        msg.instantiate();
        msg->type = (YNetMessage::MessageType::JOIN_OR_CREATE_ROOM); // JOIN_OR_CREATE_ROOM
        msg->roomCode = room_code;
        msg->password = password;
        send_message(msg);
    }
}

void YNetEnet::set_password(const String &password) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_ROOM_PASSWORD); // SET_ROOM_PASSWORD
    msg->settingValue = password;
    send_message(msg);
}

void YNetEnet::set_max_players(int max_players) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_MAX_PLAYERS); // SET_MAX_PLAYERS
    msg->settingValue = String::num(max_players);
    send_message(msg);
}

void YNetEnet::set_private(bool is_private) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_ROOM_PRIVATE); // SET_ROOM_PRIVATE
    msg->settingValue = is_private ? "true" : "false";
    send_message(msg);
}

void YNetEnet::set_can_host_migrate(bool can_migrate) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_HOST_MIGRATION); // SET_HOST_MIGRATION
    msg->settingValue = can_migrate ? "true" : "false";
    send_message(msg);
}

void YNetEnet::set_room_name(const String &room_name) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_ROOM_NAME); // SET_ROOM_NAME
    msg->settingValue = room_name;
    send_message(msg);
}

void YNetEnet::set_extra_info(const String &extra_info) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetRoomSettingMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::SET_EXTRA_INFO); // SET_EXTRA_INFO
    msg->settingValue = extra_info;
    send_message(msg);
}

void YNetEnet::get_room_info(const String &room_code) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::GET_ROOM_INFO); // GET_ROOM_INFO
    msg->data = room_code;
    send_message(msg);
}

void YNetEnet::get_room_list() {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::GET_ROOM_LIST); // GET_ROOM_LIST
    send_message(msg);
}

void YNetEnet::kick_peer(String p_peer, bool p_force) {
    if (get_state() != STATE_OPEN) return;

    Ref<YNetMessage> msg;
    msg.instantiate();
    msg->type = (YNetMessage::MessageType::KICK_CLIENT); // KICK_CLIENT
    msg->data = p_peer;
    send_message(msg);
}

YNetTransport::State YNetEnet::get_state() const {
    if (YNet::get_singleton()->get_offline_mode()) {
        return STATE_OPEN;
    }
    if (peer.is_valid() && peer->get_state() == ENetPacketPeer::STATE_CONNECTED) {
        return STATE_OPEN;
    }
    return status;
}

YNetTransport::State YNetEnet::get_current_state() const {
    return status;
}

void YNetEnet::set_current_state(State val) {
    if (status != val) {
        status = val;
        if (status != STATE_OPEN) {
            YNet::get_singleton()->room_id = "";
        }
        if (status == STATE_CONNECTING) {
            tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
        }
        if (debugging >= YNet::DebuggingLevel::MINIMAL) {
            switch (status) {
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
                default: break;
            }
        }
        YNet::get_singleton()->emit_signal(SNAME("status_changed"), val);
    }
}

bool YNetEnet::has_packet() const {
    return !unhandled_packets.is_empty();
}

int YNetEnet::get_available_packet_count() const {
    if (!peer.is_valid() || get_state() != STATE_OPEN) {
        return 0;
    }
    return unhandled_packets.size();
}

int YNetEnet::get_packet_peer() const {
    if (!has_packet()) return 1;
    auto source = unhandled_packets.front()->get().source;
    if (source == YNet::get_singleton()->host_id_hashed)
        source = 1;
    return source;
}

Error YNetEnet::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
    if (!has_packet()) return ERR_UNAVAILABLE;

    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }

    current_packet = unhandled_packets.front()->get();
    unhandled_packets.pop_front();

    *r_buffer = current_packet.data;
    r_buffer_size = current_packet.size;

    return OK;
}

void YNetEnet::transport_exit_tree() {
    transport_disconnect();
}

void YNetEnet::transport_app_close_request() {
    transport_disconnect();
}

void YNetEnet::set_max_queued_packets(int p_max_queued_packets) {
    // ENet handles this internally
}

int YNetEnet::get_max_queued_packets() const {
    return 2048; // Default ENet value
}

void YNetEnet::set_inbound_buffer_size(int p_size) {
    // ENet handles buffer sizes internally
}

void YNetEnet::set_outbound_buffer_size(int p_size) {
    // ENet handles buffer sizes internally
}

int YNetEnet::get_close_code() const {
    return 0; // ENet doesn't use close codes like WebSocket
}

String YNetEnet::get_close_reason() const {
    return String(); // ENet doesn't provide close reasons
}

int YNetEnet::get_max_packet_size() const {
    return max_packet_size;
}
