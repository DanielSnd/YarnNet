#include "ynet_enet.h"

void YNetENet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to_url", "url"), &YNetENet::connect_to_url);
    ClassDB::bind_method(D_METHOD("disconnect"), &YNetENet::disconnect);
    ClassDB::bind_method(D_METHOD("send_packet", "data", "size"), &YNetENet::send_packet);
    ClassDB::bind_method(D_METHOD("poll"), &YNetENet::poll);
    ClassDB::bind_method(D_METHOD("get_state"), &YNetENet::get_state);
    ClassDB::bind_method(D_METHOD("has_packet"), &YNetENet::has_packet);
    ClassDB::bind_method(D_METHOD("get_packet"), &YNetENet::get_packet);
    ClassDB::bind_method(D_METHOD("set_max_queued_packets", "max_queued_packets"), &YNetENet::set_max_queued_packets);
    ClassDB::bind_method(D_METHOD("get_max_queued_packets"), &YNetENet::get_max_queued_packets);
    ClassDB::bind_method(D_METHOD("set_inbound_buffer_size", "size"), &YNetENet::set_inbound_buffer_size);
    ClassDB::bind_method(D_METHOD("set_outbound_buffer_size", "size"), &YNetENet::set_outbound_buffer_size);
    ClassDB::bind_method(D_METHOD("get_close_code"), &YNetENet::get_close_code);
    ClassDB::bind_method(D_METHOD("get_close_reason"), &YNetENet::get_close_reason);
    ClassDB::bind_method(D_METHOD("set_supported_protocols", "protocols"), &YNetENet::set_supported_protocols);
    ClassDB::bind_method(D_METHOD("set_debugging", "debugging"), &YNetENet::set_debugging);
    ClassDB::bind_method(D_METHOD("get_debugging"), &YNetENet::get_debugging);
    ClassDB::bind_method(D_METHOD("get_port"), &YNetENet::get_port);

    ADD_SIGNAL(MethodInfo("enet_packet_received", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "packet")));

    BIND_ENUM_CONSTANT(NONE);
    BIND_ENUM_CONSTANT(ERROR);
    BIND_ENUM_CONSTANT(WARN);
    BIND_ENUM_CONSTANT(INFO);
    BIND_ENUM_CONSTANT(DEBUG);
    BIND_ENUM_CONSTANT(ALL);
}

YNetENet::YNetENet() {
    client = nullptr;
    status = STATE_CLOSED;
    max_queued_packets = 2048;
    debugging = NONE;
    port = 0;
}

YNetENet::~YNetENet() {

}

Error YNetENet::connect_to(const String &p_url) {
    if (debugging == ALL) {
        print_line("[YNet] Connecting to ENet server... url: ", p_url);
    }

    if (!client) {
        client = enet_host_create(nullptr, 1, 2, 0, 0);
        if (debugging == ALL) {
            print_line("[YNet] ENet client created? ", client != nullptr);
        }
    } else {
        if (status != STATE_CLOSED) {
            disconnect();
            ERR_PRINT("[YNet] Called connect when already connected");
            return ERR_ALREADY_IN_USE;
        }
    }

    ERR_FAIL_COND_V(client == nullptr, ERR_UNCONFIGURED);

    // Parse URL to get host and port
    // String host = p_url;
    if (p_url.contains(":")) {
        // int pos = p_url.find_last(":");
        // host = p_url.substr(0, pos);
        // port = p_url.substr(pos + 1).to_int();
    } else {
        port = 7777; // Default port
    }

    // url = host;

    status = STATE_CONNECTING;
    // Error err = client->connect_to_host(host, port);

    // if (err != OK) {
    //     status = STATE_CLOSED;
    //     if (debugging >= ERROR) {
    //         print_line("[YNet] Failed to connect to ENet server: ", err);
    //     }
    // }

    return OK;
}

void YNetENet::disconnect() {
    if (client != nullptr) {
        enet_host_destroy(client);
        client = nullptr;
        status = STATE_CLOSED;
        if (debugging == ALL) {
            print_line("[YNet] Disconnected from ENet server");
        }
    }
}

Error YNetENet::send_packet(const uint8_t *p_data, int p_size) {
    ERR_FAIL_COND_V(client == nullptr, ERR_UNCONFIGURED);
    // return client->put_packet(p_data, p_size);
    return OK;
}

Error YNetENet::poll() {
    ERR_FAIL_COND_V(client == nullptr, ERR_UNCONFIGURED);

    // if (status == STATE_CONNECTING) {
    //     PacketPeerENet::ConnectionStatus conn_status = client->get_connection_status();
    //     if (conn_status == PacketPeerENet::CONNECTION_CONNECTED) {
    //         status = STATE_OPEN;
    //         if (debugging == ALL) {
    //             print_line("[YNet] Connected to ENet server");
    //         }
    //     } else if (conn_status == PacketPeerENet::CONNECTION_DISCONNECTED) {
    //         status = STATE_CLOSED;
    //         if (debugging >= ERROR) {
    //             print_line("[YNet] Failed to connect to ENet server");
    //         }
    //         return ERR_CONNECTION_ERROR;
    //     }
    // }

    // if (status == STATE_OPEN) {
    //     while (client->get_available_packet_count() > 0) {
    //         const uint8_t *packet;
    //         int len;
    //         Error err = client->get_packet(&packet, len);
    //         if (err == OK) {
    //             PackedByteArray packet_data;
    //             packet_data.resize(len);
    //             memcpy(packet_data.ptrw(), packet, len);
    //             emit_signal("enet_packet_received", packet_data);
    //         }
    //     }
    // }

    return OK;
}

YNetTransport::State YNetENet::get_state() const {
    // if (!client.is_valid()) {
    //     return STATE_CLOSED;
    // }

    // PacketPeerENet::ConnectionStatus conn_status = client->get_connection_status();
    // switch (conn_status) {
    //     case PacketPeerENet::CONNECTION_CONNECTED:
    //         return STATE_OPEN;
    //     case PacketPeerENet::CONNECTION_CONNECTING:
    //         return STATE_CONNECTING;
    //     case PacketPeerENet::CONNECTION_DISCONNECTED:
    //         return STATE_CLOSED;
    //     default:
    //         return STATE_CLOSED;
    // }
    return STATE_CLOSED;
}

bool YNetENet::has_packet() const {
    // ERR_FAIL_COND_V(!client.is_valid(), false);
    // return client->get_available_packet_count() > 0;
}

Error YNetENet::get_packet(const uint8_t **r_packet, int &r_packet_size) {
    // ERR_FAIL_COND_V(!client.is_valid(), ERR_UNCONFIGURED);
    // return client->get_packet(r_packet, r_packet_size);
}

void YNetENet::set_max_queued_packets(int p_max_queued_packets) {
    max_queued_packets = p_max_queued_packets;
    // if (client.is_valid()) {
    //     client->set_max_queued_packets(p_max_queued_packets);
    // }
}

int YNetENet::get_max_queued_packets() const {
    // if (!client.is_valid()) {
    //     return max_queued_packets;
    // }
    // return client->get_max_queued_packets();
}

void YNetENet::set_inbound_buffer_size(int p_size) {
    // if (client.is_valid()) {
    //     client->set_inbound_buffer_size(p_size);
    // }
}

void YNetENet::set_outbound_buffer_size(int p_size) {
    // if (client.is_valid()) {
    //     client->set_outbound_buffer_size(p_size);
    // }
}

int YNetENet::get_close_code() const {
    return 0; // ENet doesn't use close codes
}

String YNetENet::get_close_reason() const {
    return String(); // ENet doesn't use close reasons
}

void YNetENet::set_supported_protocols(const Vector<String> &p_protocols) {
    // ENet doesn't use protocols
}

void YNetENet::set_debugging(int p_debugging) {
    debugging = p_debugging;
}

int YNetENet::get_debugging() const {
    return debugging;
}

int YNetENet::get_port() const {
    return port;
} 