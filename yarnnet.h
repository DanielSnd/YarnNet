#ifndef GODOT_YARNNET_H
#define GODOT_YARNNET_H

#include "core/object/ref_counted.h"
#include "modules/websocket/websocket_peer.h"
#include "core/error/error_list.h"
#include "scene/main/node.h"
#include "core/io/json.h"
#include "modules/regex/regex.h"
#include "scene/main/window.h"
#include "scene/main/multiplayer_api.h"
#include "scene/main/multiplayer_peer.h"
#include <cstring>

#include "modules/multiplayer/scene_multiplayer.h"
#include "scene/resources/packed_scene.h"
#include "scene/3d/node_3d.h"
#include "scene/2d/node_2d.h"


class YNetPropertySyncer : public RefCounted {
    GDCLASS(YNetPropertySyncer, RefCounted);

protected:
    static void _bind_methods();

public:
    YNetPropertySyncer();

    uint8_t property_syncer_index;
    ObjectID target;
    Vector<StringName> property;
    int net_id;
    Variant current_val;
    int authority;
    bool sync_always;

    static uint32_t get_property_syncer_id_from_property_stringnames(const Vector<StringName> &p_property);
    void set_current_val(const Variant &new_value);
    bool check_for_changed_value();

    YNetPropertySyncer(int p_net_id, Object *p_target, const Vector<StringName> &p_property, const Variant &p_val, int authority, bool p_sync_always);
    ~YNetPropertySyncer();
};

class YNet : public Node {
    GDCLASS(YNet, Node);
    
protected:
    static void _bind_methods();

    Ref<SceneMultiplayer> scene_multiplayer;
    Ref<YNetPropertySyncer> register_sync_property(Node *p_target, const NodePath &p_property, int authority, bool p_always_sync);

    struct NetworkSpawnedObjectInfo {
        int network_instance_id;
        uint32_t spawnable_scene_id;
        String spawned_name;
        ObjectID SpawnedNodeId;
        NodePath desired_parent;
        int authority;
        int spawn_pos_x;
        int spawn_pos_y;
        int spawn_pos_z;
    };

    Error _send_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Error _send_and_receive_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);

    StringName receive_yrpc_stringname;
    StringName receive_yrpc_also_local_stringname;
    StringName rpc_spawn_stringname;
    StringName rpc_despawn_stringname;
    StringName rpc_request_spawned_nodes_stringname;
    StringName rpc_respond_with_spawned_nodes_stringname;
    StringName rpc_rec_synced_vars_stringname;

    int count_to_check_should_spawn = 0;


    void _notification(int p_what);

    void spawned_network_node_exited_tree(int p_nid);

    void clear_all_spawned_network_nodes();

    void on_connected_to_server();

    void on_received_peer_packet(int packet_from, const Vector<uint8_t> &packet_received);

    void received_sync_packet(int received_from, const Vector<uint8_t> &_packet);

    Dictionary create_rpc_dictionary_config(MultiplayerAPI::RPCMode p_rpc_mode,
                                            MultiplayerPeer::TransferMode p_transfer_mode, bool p_call_local,
                                            int p_channel);

    bool ynet_settings_enabled=false;
    inline static const String slash_namespace = "/";
    bool process_packets();

    void socketio_connect(String name_space = slash_namespace);
    void socketio_disconnect(String name_space = slash_namespace);

    bool socketio_parse_packet(String &payload);

    void update_last_engine_state();

    void do_process();

    void update_networked_property_syncers();

    void setup_node();

    int get_queued_spawn_count() const {
        return static_cast<int>(queued_networked_spawned_objects.size());
    }

    int get_spawned_obj_count() const {
        return static_cast<int>(networked_spawned_objects.size());
    }

    bool already_setup_in_tree = false;
    static YNet* singleton;
    HashMap<int,ObjectID> yrpc_to_node_hash_map;

    void clear_unhandled_packets();

public:
    void remove_from_yrpc_receiving_map(int p_yrpc_id);

    Variant _receive_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Variant _receive_yrpc_also_local(const Variant **p_args, int p_argcount, Callable::CallError &r_error);

    struct Packet {
        int source = 0;
        uint8_t *data = nullptr;
        uint32_t size = 0;
    };

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
        MINIMAL = 1,
        MOSTMESSAGES = 2,
        MESSAGESANDPING = 3,
        ALL = 4
    };

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

    List<Packet> unhandled_packets;

    bool engineio_decode_packet(const uint8_t *packet, int len);

    Error engineio_send_packet_charstring(const CharString *cs);

    Error engineio_send_packet_type(EngineIOPacketType packet_type);
    Error engineio_send_packet_binary(EngineIOPacketType packet_type, PackedByteArray &p_message);
    Error engineio_send_packet_text(EngineIOPacketType packet_type, String &p_text);
    Error socketio_send_packet_binary(SocketIOPacketType packet_type, PackedByteArray &p_message);
    Error socketio_send_packet_text(SocketIOPacketType packet_type, Variant p_text = {}, String name_space = slash_namespace);
    Error socketio_send(String event_name, Variant data = {} , String name_space = slash_namespace);

    int next_networked_spawn_id = 1;

    Vector<uint8_t> packet_cache;

    enum NetworkNodeIdCompression {
        NETWORK_NODE_ID_COMPRESSION_8 = 0,
        NETWORK_NODE_ID_COMPRESSION_16,
        NETWORK_NODE_ID_COMPRESSION_32,
    };
    HashMap<int,NetworkSpawnedObjectInfo> queued_networked_spawned_objects;
    HashMap<int,NetworkSpawnedObjectInfo> networked_spawned_objects;
    HashMap<int,int> networked_id_to_authority;
    HashMap<int,Vector<Ref<YNetPropertySyncer>>> networked_property_syncers;
    HashMap<int,HashMap<uint8_t,Variant>> queued_received_property_syncers;
    HashMap<int,HashMap<uint8_t,Variant>> queued_to_send_property_syncers;

    uint64_t last_sent_synced_vars;
    uint64_t last_watched_synced_vars;

    uint64_t watch_synced_vars_interval;
    uint64_t send_synced_vars_interval;

    HashMap<uint32_t,String> spawnables_dictionary;

    void add_network_spawnable(const String& new_spawnable) {
        spawnables_dictionary[string_to_hash_id(new_spawnable)] = new_spawnable;
    }


    Ref<PackedScene> find_network_spawnable(uint32_t new_spawnable_id);

    uint32_t get_network_spawnable_id(const String& new_spawnable) {
        return string_to_hash_id(new_spawnable);
    }

    bool is_network_spawnable(const String& new_spawnable) {
        return spawnables_dictionary.has(get_network_spawnable_id(new_spawnable));
    }

    String protocol ="change_me";
    String get_protocol() const {return protocol;}
    void set_protocol(String val) {protocol = val;}
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

    int real_hashed_sid{};
    int get_real_hashed_sid() const {return real_hashed_sid;}
    void set_real_hashed_sid(int val) {}
    int hashed_sid{};
    int get_hashed_sid() const {return hashed_sid;}
    void set_hashed_sid(int val) {}

    Node* find_node_with_net_id(int p_net_id);

    String room_id = "";
    String get_room_id() const {return room_id;}
    void set_room_id(String val) {}
    String get_room_id_without_protocol() const {
        if (room_id.contains(protocol)) {
            return room_id.replace(protocol,"");
        }
        return room_id;
    }

    void set_room_id_without_protocol(String val) {
        room_id = val + protocol;
    }

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

    bool has_room() const {return !room_id.is_empty();}

    bool offline_mode = false;
    bool get_offline_mode() const {return offline_mode;}
    void set_offline_mode(bool val) {offline_mode = val;}

    
    bool pause_receive_spawns = false;
    bool get_pause_receive_spawns() const {return pause_receive_spawns;}
    void set_pause_receive_spawns(bool val) {pause_receive_spawns = val;}
    
    int last_used_id = 1;
    int get_last_used_id() const {return last_used_id;}
    void set_last_used_id(int val) {last_used_id = val;}

    bool is_host = false;
    bool get_is_host() const {return is_host;}
    void set_is_host(bool val) {}

    Node *internal_spawn(int network_id, const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name,
                         const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority);

    void set_authority_after_entered(Node *node_entered_tree, const Variant &p_spawn_pos, int authority);

    Node *spawn_with_path(const String &p_spawnable_scene_path, const String &p_spawn_name,
                          const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority);

    void rpc_spawn(int network_id, const uint32_t &p_spawnable_path_id, const String &p_spawn_name, const String &p_desired_parent,
                   const Variant &p_spawn_pos, int authority);

    void internal_spawn_with_queued_struct(const NetworkSpawnedObjectInfo &p_nsoi);

    Variant create_spawned_lists_variant();

    void unpack_property_syncer_received_value(const Array &received_property_syncer_value);

    void unpack_spawned_list_variants(const Array &received_spawned_list);

    Variant convert_nsoi_to_variant(const NetworkSpawnedObjectInfo &p_nsoi);

    void unpack_spawninfo_from_variant(const Array &received_spawn_info);

    Node *spawn(const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name, const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority);

    void rpc_request_spawned_nodes(int p_id_requesting);

    void rpc_respond_with_spawned_nodes(const Array &spawned_nodes_info);

    void rpc_despawn(int p_network_id);

    void despawn(int network_id);

    void despawn_node(Node *node_to_despawn);

    String server_or_client_str();

    void test_send_sync();

    void send_sync_vars(uint64_t p_cur_usec);
    void rpc_recv_sync_vars(Variant p_synced_vars_data);

    String host_id;
    String get_host_id() const {return host_id;}
    void set_host_id(String val) {host_id = val;}

    int host_id_hashed{};

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
                room_id = "";
            }
            if (debugging >= 2) {
                switch ((State)status) {
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
                    default: ;
                }
            }
            emit_signal("status_changed",val);
        }
    }

    enum WriteMode {
        WRITE_MODE_TEXT,
        WRITE_MODE_BINARY,
    };

    enum {
        DEFAULT_BUFFER_SIZE = 65535,
    };
    YNet* engineio_connect(String url);

    void create_client();

    void engineio_disconnect();


    int max_queued_packets;
    void set_max_queued_packets(int p_max_queued_packets);
    int get_max_queued_packets();

    YNet* create_room();

    YNet *create_room_with_code(const String &create_room);

    YNet* join_or_create_room(const String &join_room);

    YNet* join_room(const String &p_join_room);
    YNet* leave_room();

    YNet* join_room_with_password(const String &roomCode, const String &password);

    Error set_password(const String &newPassword);

    Error set_max_players(int newMaxPlayers);

    Error set_private(bool newPrivate);

    Error set_can_host_migrate(bool newCanHostMigrate);

    Error set_room_name(const String &newRoomName);

    Error set_extra_info(const String &new_extra_info);

    YNet* get_room_info(const String &roomCode);

    YNet* get_room_list();

    void on_room_created(const String &p_new_room_id);
    void on_room_joined(const String &p_new_room_id, const String &p_host_id);
    void on_room_error(const String &p_room_id);

    void on_left_room();

    void on_room_info(const Variant &p_room_info);

    void on_room_list(const Variant &p_room_list);

    void on_room_players(const Array &players_array);

    void on_player_join(const String &p_player);

    void on_received_pkt(const String &received_from, const String &pkt_content);

    void on_player_left(const String &p_player);

#ifdef HOST_MIGRATION
    void on_host_migrated(const String &p_new_host);
#endif

    void register_for_yrpcs(Node* p_registering_node, int registering_id);

    HashMap<String,int> connections_map;

    bool received_any_packet = false;
    static uint32_t string_to_hash_id(const String &p_string);

    static YNet* get_singleton();
    YNet();

    ~YNet();

    void add_setting(const String& name, const Variant& default_value, Variant::Type type,
            PropertyHint hint = PROPERTY_HINT_NONE, const String& hint_string = "",
            int usage = PROPERTY_USAGE_DEFAULT, bool restart_if_changed = false);
};

VARIANT_ENUM_CAST(YNet::EngineIOPacketType);
VARIANT_ENUM_CAST(YNet::SocketIOPacketType);
VARIANT_ENUM_CAST(YNet::DebuggingLevel);
VARIANT_ENUM_CAST(YNet::WriteMode);
VARIANT_ENUM_CAST(YNet::State);
#endif