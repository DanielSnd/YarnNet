#ifndef YNET_ENET_H
#define YNET_ENET_H

#include "ynet_transport.h"
#include "enet/enet.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"

class YNetENet : public YNetTransport {
    GDCLASS(YNetENet, YNetTransport);

private:
    ENetPeer* client;
    String ip_address;
    int port;
    State status;
    int max_queued_packets;
    int debugging;

    static const int DEFAULT_BUFFER_SIZE = 65535;

protected:
    static void _bind_methods();

public:
    enum DebugLevel {
        NONE = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        ALL = 5
    };

    virtual Error connect_to(const String &p_address) override;
    virtual void disconnect() override;
    virtual Error send_packet(const uint8_t *p_data, int p_size) override;
    virtual Error poll() override;
    virtual State get_state() const override;
    virtual bool has_packet() const override;
    virtual Error get_packet(const uint8_t **r_packet, int &r_packet_size) override;
    virtual void set_max_queued_packets(int p_max_queued_packets) override;
    virtual int get_max_queued_packets() const override;
    virtual void set_inbound_buffer_size(int p_size) override;
    virtual void set_outbound_buffer_size(int p_size) override;
    virtual int get_close_code() const override;
    virtual String get_close_reason() const override;
    virtual void transport_process(YNet* ynet) override;
    virtual void transport_exit_tree() override;
    virtual void transport_app_close_request() override;

    void set_debugging(int p_debugging);
    int get_debugging() const;
    int get_port() const;

    YNetENet();
    virtual ~YNetENet();
};

VARIANT_ENUM_CAST(YNetENet::DebugLevel);

#endif // YNET_ENET_H 