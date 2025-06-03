#ifndef YNET_TRANSPORT_H
#define YNET_TRANSPORT_H

#include "core/object/ref_counted.h"
#include "core/error/error_list.h"
#include "scene/main/node.h"
#include "core/io/json.h"
#include "ynet_types.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "yarnnet.h"

class YNetTransport : public RefCounted {
    GDCLASS(YNetTransport, RefCounted);

public:
    enum State {
        STATE_CLOSED,
        STATE_CONNECTING,
        STATE_OPEN,
        STATE_CLOSING
    };
    State status = STATE_CLOSED;
    State last_transport_state = STATE_CLOSED;
    float tick_started_connecting = 0.0;
    bool was_timeout = false;
    mutable Error last_get_error = OK;
    
    virtual Error connect_to(const String &p_address) = 0;
    virtual void disconnect() = 0;
    virtual Error send_packet(const uint8_t *p_data, int p_size) = 0;
    virtual Error poll() = 0;
    virtual State get_state() const = 0;
    virtual bool has_packet() const = 0;
    virtual Error get_packet(const uint8_t **r_packet, int &r_packet_size) = 0;
    virtual bool process_packets() = 0;
    virtual void set_max_queued_packets(int p_max_queued_packets) = 0;
    virtual int get_max_queued_packets() const = 0;
    virtual void set_inbound_buffer_size(int p_size) = 0;
    virtual void set_outbound_buffer_size(int p_size) = 0;
    virtual int get_close_code() const = 0;
    virtual String get_close_reason() const = 0;
    virtual void transport_process(YNet* ynet) = 0;
    virtual void transport_exit_tree() = 0;
    virtual void transport_app_close_request() = 0;

protected:
    static void _bind_methods();

public:

    State get_current_state() const;
    virtual void set_current_state(State val) = 0;

    YNetTransport();
    virtual ~YNetTransport();
};

VARIANT_ENUM_CAST(YNetTransport::State);

#endif // YNET_TRANSPORT_H 