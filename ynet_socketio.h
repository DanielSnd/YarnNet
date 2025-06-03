#ifndef YNET_SOCKETIO_H
#define YNET_SOCKETIO_H

#include "ynet_transport.h"
#include "modules/websocket/websocket_peer.h"
#include "core/io/json.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/io/json.h"
#include "modules/regex/regex.h"
#include "yarnnet.h"
#include <cstring>

class YNetSocketIO : public YNetTransport {
    GDCLASS(YNetSocketIO, YNetTransport);

private:
    Ref<WebSocketPeer> client;
    String url;
    String sid;
    float pingTimeout;
    float pingInterval;
    State status;
    State last_engine_state;
    float tick_started_connecting;
    bool was_timeout;
    int max_queued_packets;
    int debugging;

    inline static const char *close_cs = "1";
    static const int DEFAULT_BUFFER_SIZE = 65535;

    bool engineio_decode_packet(const uint8_t *p_packet, int p_size);
    void engineio_send_packet_charstring(const char **p_packet);
    void socketio_send(const String &p_event, const Array &p_args);
    void socketio_send_binary(const String &p_event, const Array &p_args);
    void socketio_send_ack(const String &p_event, const Array &p_args, int p_ack_id);
    void socketio_send_binary_ack(const String &p_event, const Array &p_args, int p_ack_id);
    inline static const String slash_namespace = "/";
    void socketio_connect(String name_space = slash_namespace);
    void socketio_disconnect(String name_space = slash_namespace);
    bool socketio_parse_packet(String& payload);
    void update_last_engine_state();
    bool process_packets();

protected:
    static void _bind_methods();

public:

    enum WriteMode {
        TEXT = 0,
        BINARY = 1
    };

    enum EngineIOPacketType {
        OPEN = 0,
        CLOSE = 1,
        PING = 2,
        PONG = 3,
        MESSAGE = 4,
        UPGRADE = 5,
        NOOP = 6
    };

    enum SocketIOPacketType {
        CONNECT = 0,
        DISCONNECT = 1,
        EVENT = 2,
        ACK = 3,
        ERROR = 4,
        BINARY_EVENT = 5,
        BINARY_ACK = 6
    };

    HashMap<String,int> connections_map;

    void clear_unhandled_packets();

    bool received_any_packet = false;
    const String key_sid = "sid";
    const String key_pingTimeout = "pingTimeout";
    const String key_pingInterval = "pingInterval";
    const CharString open_cs    = vformat("%d",0).utf8();
    const CharString close_cs   = vformat("%d",1).utf8();
    const CharString ping_cs    = vformat("%d",2).utf8();
    const CharString pong_cs    = vformat("%d",3).utf8();
    const CharString message_cs = vformat("%d",4).utf8();
    const CharString upgrade_cs = vformat("%d",5).utf8();
    const CharString noop_cs    = vformat("%d",6).utf8();
    uint32_t roomcreated_event = String{"roomcreated"}.hash();
    uint32_t roomjoined_event  = String{"roomjoined" }.hash();
    uint32_t roomplayers_event = String{"roomplayers"}.hash();
    uint32_t roomerror_event   = String{"roomerror"  }.hash();
    uint32_t playerjoin_event  = String{"playerjoin" }.hash();
    uint32_t playerleft_event  = String{"playerleft" }.hash();
    uint32_t newhost_event     = String{"newhost"    }.hash();
    uint32_t leftroom          = String{"leftroom"   }.hash();
    uint32_t pkt               = String{"pkt"        }.hash();
    uint32_t roomlist          = String{"roomlist"   }.hash();
    uint32_t roominfo          = String{"roominfo"   }.hash();

    List<YNetTypes::Packet> unhandled_packets;

    bool engineio_decode_packet(const uint8_t *packet, int len);

    Error engineio_send_packet_charstring(const CharString *cs);

    Error engineio_send_packet_type(EngineIOPacketType packet_type);
    Error engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message);
    Error engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text);
    Error socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message);
    Error socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text = {}, String name_space = slash_namespace);
    Error socketio_send(String event_name, Variant data = {} , String name_space = slash_namespace);

    virtual Error connect_to(const String &p_url) override;
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
    void set_supported_protocols(const Vector<String> &p_protocols);
    virtual void transport_process(YNet* ynet) override;
    virtual void transport_exit_tree() override;
    virtual void transport_app_close_request() override;

    Vector<uint8_t> packet_cache;

    void set_debugging(int p_debugging);
    int get_debugging() const;
    String get_sid() const;
    bool was_connection_timeout() const;

    YNetSocketIO();
    virtual ~YNetSocketIO();
};

VARIANT_ENUM_CAST(YNetSocketIO::EngineIOPacketType);
VARIANT_ENUM_CAST(YNetSocketIO::SocketIOPacketType);
VARIANT_ENUM_CAST(YNetSocketIO::WriteMode);
#endif // YNET_SOCKETIO_H 