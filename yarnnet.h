#ifndef GODOT_YARNNET_H
#define GODOT_YARNNET_H

#include "core/object/ref_counted.h"
#include "modules/websocket/websocket_peer.h"
#include "core/error/error_list.h"
#include "scene/main/node.h"
#include "core/io/json.h"
#include "modules/regex/regex.h"
#include "scene/main/window.h"
#include <cstring>

class YarnNet : public Node {
    GDCLASS(YarnNet, Node);
    
protected:
    static void _bind_methods();
    void _notification(int p_what);

    inline static const String slash_namespace = "/";
    bool process_packets();

    void socketio_connect(String name_space = slash_namespace);
    void socketio_disconnect(String name_space = slash_namespace);

    bool socketio_parse_packet(String &payload);

    void update_last_engine_state();

    void do_process();

    void setup_node();
    bool already_setup_in_tree = false;
    static YarnNet* singleton;

public:
    
    enum EngineIOPacketType {
        open = 0,
        close = 1,
        ping = 2,
        pong = 3,
        message = 4,
        upgrade = 5,
        noop = 6,
    };

    enum SocketIOPacketType {
        CONNECT = 0,
        DISCONNECT = 1,
        EVENT = 2,
        ACK = 3,
        CONNECT_ERROR = 4,
        BINARY_EVENT = 5,
        BINARY_ACK = 6,
    };

    enum DebuggingLevel {
        NONE = 0,
        MOSTMESSAGES = 1,
        MESSAGESANDPING = 2,
        ALL = 3
    };

    
    uint32_t roomcreated_event = String{"roomcreated"}.hash();
    uint32_t roomjoined_event  = String{"roomjoined" }.hash();
    uint32_t roomerror_event   = String{"roomerror"  }.hash();
    uint32_t rpc_event         = String{"rpc"        }.hash();
    uint32_t playerjoin_event  = String{"playerjoin" }.hash();
    uint32_t playerleft_event  = String{"playerleft" }.hash();
    uint32_t newhost_event     = String{"newhost"    }.hash();
    uint32_t leftroom          = String{"leftroom"   }.hash();

    bool engineio_decode_packet(const uint8_t *packet, int len);

    Error engineio_send_packet_type(EngineIOPacketType packet_type);
    Error engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message);
    Error engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text);
    Error socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message);
    Error socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text = {}, String name_space = slash_namespace);
    Error socketio_send(String event_name, Variant data = {} , String name_space = slash_namespace);
    
    String protocol ="change_me";
    mutable Error last_get_error = OK;
    String url = "";
    String get_url() const {return url;}
    void set_url(String val) {url = val;}
    Ref<WebSocketPeer> client;
    Ref<WebSocketPeer> get_client() const {return client;}
    void set_client(Ref<WebSocketPeer> val) {client = val;}
    String sid = "";
    String get_sid() const {return sid;}
    void set_sid(String val) {sid = val;}
    int pingTimeout = 0;
    int get_ping_timeout() const {return pingTimeout;}
    void set_ping_timeout(int val) {pingTimeout = val;}
    int pingInterval = 0;
    int get_ping_interval() const {return pingInterval;}
    void set_ping_interval(int val) {pingInterval = val;}
    DebuggingLevel debugging = NONE;
    DebuggingLevel get_debugging() const {return debugging;}
    void set_debugging(DebuggingLevel val) {debugging = val;}
    enum State {
        STATE_CONNECTING,
        STATE_OPEN,
        STATE_CLOSING,
        STATE_CLOSED
    };
    State status = STATE_CLOSED;
    State last_engine_state = STATE_CLOSED;
    float tick_started_connecting = 0.0;
    bool was_timeout = false;
    String current_room = "";
    String get_current_room() const {return current_room;}

    bool has_room() const {return !current_room.is_empty();}

    bool offline_mode = false;
    bool get_offline_mode() const {return offline_mode;}
    void set_offline_mode(bool val) {offline_mode = val;}

    int last_used_id = 0;
    int get_last_used_id() const {return last_used_id;}
    void set_last_used_id(int val) {last_used_id = val;}

    int get_new_network_id() { last_used_id = last_used_id + 1; return last_used_id-1;}

    State get_current_state() const {
        if(offline_mode) {
            return STATE_OPEN;
        }
        return status;
    }
    void set_current_state(State val) {
        if (status != val) {
            status = val;
            if (status == STATE_CONNECTING) {
                tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
            }
            if (status != STATE_OPEN) {
                current_room = "";
            }
            if (debugging >= 1) {
                switch ((State)status) {
                    case STATE_CONNECTING:
                        print_line("[YarnNet] Status is now: Connecting");
                        break;
                    case STATE_OPEN:
                        print_line("[YarnNet] Status is now: Connected");
                        break;
                    case STATE_CLOSING:
                        print_line("[YarnNet] Status is now: Closing");
                        break;
                    case STATE_CLOSED:
                        print_line("[YarnNet] Status is now: Closed");
                        break;
                    default: ;
                }
            }
        }
    }

    enum WriteMode {
        WRITE_MODE_TEXT,
        WRITE_MODE_BINARY,
    };

    enum {
        DEFAULT_BUFFER_SIZE = 65535,
    };
    Error engineio_connect(String url);

    void create_client();

    Dictionary DebugRPCs(Node *node);

    void engineio_disconnect();

    int max_queued_packets;
    void set_max_queued_packets(int p_max_queued_packets);
    int get_max_queued_packets();

    Error create_room();

    Error join_or_create_room(const String &join_room);

    Error join_room(const String &p_join_room);
    Error leave_room();
    void on_room_created(const String &p_new_room_id);
    void on_room_joined(const String &p_new_room_id);
    void on_room_error(const String &p_room_id);
    void on_player_join(const String &p_player);
    void on_player_left(const String &p_player);
    void on_host_migrated(const String &p_new_host);
    void on_rpc_event(const String &p_sender, const int &p_netnodeid, const int &p_rpc_id, Variant& p_data);

    static YarnNet* get_singleton();
    YarnNet();
    ~YarnNet();

    void add_setting(const String& name, const Variant& default_value, Variant::Type type,
            PropertyHint hint = PROPERTY_HINT_NONE, const String& hint_string = "",
            int usage = PROPERTY_USAGE_DEFAULT, bool restart_if_changed = false);
};

VARIANT_ENUM_CAST(YarnNet::EngineIOPacketType);
VARIANT_ENUM_CAST(YarnNet::SocketIOPacketType);
VARIANT_ENUM_CAST(YarnNet::DebuggingLevel);
VARIANT_ENUM_CAST(YarnNet::WriteMode);
VARIANT_ENUM_CAST(YarnNet::State);
#endif