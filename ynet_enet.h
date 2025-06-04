// ynet_enet.h
#ifndef YNET_ENET_H
#define YNET_ENET_H

#include "ynet_transport.h"
#include <enet/enet.h>
#include "core/io/json.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "yarnnet.h"
#include "ynet_messages.h"

class YNetEnet : public YNetTransport {
    GDCLASS(YNetEnet, YNetTransport);

protected:
    static void _bind_methods();

public:
    virtual Error connect_to(const String& p_url) override;
    virtual void transport_disconnect() override;
    virtual Error send_packet(const uint8_t* p_data, int p_size) override;
    virtual State get_state() const override;
    virtual bool has_packet() const override;
    virtual int get_packet_peer() const override;
    virtual Error get_packet(const uint8_t** r_packet, int& r_packet_size) override;
    virtual uint32_t string_to_hash_id(const String &p_string) override;
    // Room management implementations
    virtual void create_room() override;
    virtual void create_room_with_code(const String& room_code, const String& password = "") override;
    virtual void join_or_create_room(const String& room_code, const String& password = "") override;
    virtual void join_room(const String& room_code, const String& password = "") override;
    virtual void leave_room() override;
    virtual void set_password(const String& password) override;
    virtual void set_max_players(int max_players) override;
    virtual void set_private(bool is_private) override;
    virtual void set_can_host_migrate(bool can_migrate) override;
    virtual void set_room_name(const String& room_name) override;
    virtual void set_extra_info(const String& extra_info) override;
    virtual void get_room_info(const String& room_code) override;
    virtual void get_room_list() override;
    virtual void kick_peer(String p_peer, bool p_force) override;
    virtual void set_current_state(State val) override;
    virtual State get_current_state() const override;
    virtual int get_max_packet_size() const override;
    virtual void set_max_queued_packets(int p_max_queued_packets) override;
    virtual int get_max_queued_packets() const override;
    virtual void set_inbound_buffer_size(int p_size) override;
    virtual void set_outbound_buffer_size(int p_size) override;
    virtual int get_close_code() const override;
    virtual String get_close_reason() const override;
    virtual void transport_process(YNet* ynet) override;
    virtual void transport_exit_tree() override;
    virtual void transport_app_close_request() override;

    static String MessageTypeToString(uint8_t type);
    YNetEnet();
    ~YNetEnet();

private:
    ENetHost* client;
    ENetPeer* peer;
    String server_address;
    uint16_t server_port;
    int debugging;
    State status;
    
    // Message handling
    void handle_received_message(const uint8_t* data, size_t dataLength);
    void send_message(Ref<YNetMessage> p_message);
    
    // Packet management
    List<YNetTypes::Packet> unhandled_packets;
    YNetTypes::Packet current_packet;
    void clear_unhandled_packets();

    static const int max_packet_size = 65535;
};

#endif // YNET_ENET_H
