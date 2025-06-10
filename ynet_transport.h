#ifndef YNET_TRANSPORT_H
#define YNET_TRANSPORT_H

#include "yarnnet.h"
#include "core/object/ref_counted.h"
#include "core/error/error_list.h"
#include "scene/main/node.h"
#include "core/io/json.h"
#include "ynet_types.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"

class YNet;

class YNetTransport : public RefCounted {
    GDCLASS(YNetTransport, RefCounted);

protected:
    static void _bind_methods();
public:
    enum State {
        STATE_CONNECTING,
        STATE_OPEN,
        STATE_CLOSING,
        STATE_CLOSED
    };

    virtual Error connect_to(const String &p_address) = 0;
    virtual void transport_disconnect() = 0;
    virtual Error send_packet(const uint8_t *p_data, int p_size) = 0;
    virtual State get_state() const = 0;
    virtual bool has_packet() const = 0;
	virtual int get_available_packet_count() const { return 0; }
    virtual Error get_packet(const uint8_t **r_packet, int &r_packet_size) = 0;
    virtual void set_max_queued_packets(int p_max_queued_packets) = 0;
    virtual int get_max_queued_packets() const = 0;
    virtual int get_max_packet_size() const = 0;
    virtual void set_inbound_buffer_size(int p_size) = 0;
    virtual void set_outbound_buffer_size(int p_size) = 0;
    virtual int get_close_code() const = 0;
    virtual String get_close_reason() const = 0;
    virtual void transport_process(YNet* ynet) = 0;
    virtual void transport_exit_tree() = 0;
    virtual void transport_app_close_request() = 0;
    virtual void set_current_state(State val) = 0;

    // Room management methods
    virtual void create_room() = 0;
    virtual void create_room_with_code(const String &room_code, const String &password = "") = 0;
    virtual void join_or_create_room(const String &room_code, const String &password = "") = 0;
    virtual void join_room(const String &room_code, const String &password = "") = 0;
    virtual void leave_room() = 0;
    virtual void set_password(const String &password) = 0;
    virtual void set_max_players(int max_players) = 0;
    virtual void set_private(bool is_private) = 0;
    virtual void set_can_host_migrate(bool can_migrate) = 0;
    virtual void set_room_name(const String &room_name) = 0;
    virtual void set_extra_info(const String &extra_info) = 0;
    virtual void get_room_info(const String &room_code) = 0;
    virtual void get_room_list() = 0;
    virtual void kick_peer(String p_peer, bool p_force) = 0;

    virtual uint32_t string_to_hash_id(const String &p_string);

    virtual int get_packet_peer() const = 0;
    virtual State get_current_state() const = 0;
    State status = STATE_CLOSED;
    float tick_started_connecting = 0.0;
    bool was_timeout = false;
    mutable Error last_get_error = OK;
    String target_peer;
    HashMap<String, int> connections_map;

    YNetTransport();
    virtual ~YNetTransport();
};

VARIANT_ENUM_CAST(YNetTransport::State);

#endif // YNET_TRANSPORT_H 