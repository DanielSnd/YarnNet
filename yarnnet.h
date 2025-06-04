#ifndef GODOT_YARNNET_H
#define GODOT_YARNNET_H

#include "core/object/ref_counted.h"
#include "core/error/error_list.h"
#include "scene/main/node.h"
#include "scene/main/window.h"
#include "scene/main/multiplayer_api.h"
#include "scene/main/multiplayer_peer.h"
#include "ynet_types.h"
#include "ynet_transport.h"
#include "core/os/os.h"


#include "modules/multiplayer/scene_multiplayer.h"
#include "scene/resources/packed_scene.h"
#include "scene/3d/node_3d.h"
#include "scene/2d/node_2d.h"
#include "scene/scene_string_names.h"
#include "ynetsyncer.h"

#include "core/config/project_settings.h"
#include "core/crypto/crypto_core.h"
#include "core/io/marshalls.h"
#include "core/object/script_language.h"
#include "scene/2d/node_2d.h"

#ifdef TOOLS_ENABLED
#include "editor/run_instances_dialog.h"
#endif

class YNetPropertySyncer;
class YNetTransport;

class YNet : public Node {
    GDCLASS(YNet, Node);
    
private:
    struct YNetRPCConfig {
		StringName name;
		MultiplayerAPI::RPCMode rpc_mode = MultiplayerAPI::RPC_MODE_DISABLED;
		bool call_local = false;
		MultiplayerPeer::TransferMode transfer_mode = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
		int channel = 0;

		bool operator==(YNetRPCConfig const &p_other) const {
			return name == p_other.name;
		}
	};

    struct YNetRPCConfigCache {
        HashMap<uint16_t, YNetRPCConfig> configs;
        HashMap<StringName, uint16_t> ids;
    };

    // Cache for RPC configs, keyed by ObjectID
    HashMap<ObjectID, YNetRPCConfigCache> rpc_config_cache;
    
protected:

    Vector<uint8_t> packet_cache;
    
    static void _bind_methods();

    void _parse_rpc_config(const Dictionary &p_config, bool p_for_node, YNetRPCConfigCache &r_cache);
    YNetRPCConfig _get_rpc_config(Node *p_node, const StringName &p_method);
    
    Ref<SceneMultiplayer> scene_multiplayer;
    Ref<YNetPropertySyncer> register_sync_property(Node *p_target, const NodePath &p_property, int authority, bool p_always_sync);

    struct NetworkSpawnedObjectInfo {
        uint32_t network_instance_id;
        uint32_t spawnable_scene_id;
        String spawned_name;
        ObjectID SpawnedNodeId;
        NodePath desired_parent;
        int authority;
        int spawn_pos_x;
        int spawn_pos_y;
        int spawn_pos_z;
        bool cleanup_with_owner = false;
    };

    Variant _receive_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Variant _receive_yrpc_also_local(const Variant **p_args, int p_argcount, Callable::CallError &r_error);

    StringName receive_yrpc_stringname;
    StringName receive_yrpc_also_local_stringname;
    StringName receive_yrpc_unreliable_stringname;
    StringName receive_yrpc_unreliable_also_local_stringname;
    StringName receive_yrpc_reliable_stringname;
    StringName receive_yrpc_reliable_also_local_stringname;
    StringName rpc_spawn_stringname;
    StringName rpc_despawn_stringname;
    StringName rpc_request_spawned_nodes_stringname;
    StringName rpc_respond_with_spawned_nodes_stringname;
    StringName rpc_rec_synced_vars_stringname;

    int count_to_check_should_spawn = 0;


    void _notification(int p_what);

    void spawned_network_node_exited_tree(uint32_t p_nid);

    void clear_all_spawned_network_nodes();

    void on_connected_to_server();

    void on_received_peer_packet(int packet_from, const Vector<uint8_t> &packet_received);

    void received_sync_packet(int received_from, const Vector<uint8_t> &_packet);

    Dictionary create_rpc_dictionary_config(MultiplayerAPI::RPCMode p_rpc_mode,
                                            MultiplayerPeer::TransferMode p_transfer_mode, bool p_call_local,
                                            int p_channel);

    bool ynet_settings_enabled=false;

    void setup_node();

    void cleanup_node();

    int get_queued_spawn_count() const {
        return static_cast<int>(queued_networked_spawned_objects.size());
    }

    int get_spawned_obj_count() const {
        return static_cast<int>(networked_spawned_objects.size());
    }

    bool already_setup_in_tree = false;
    inline static YNet* singleton = nullptr;
    HashMap<uint32_t,ObjectID> yrpc_to_node_hash_map;

public:
    Ref<YNetTransport> transport;
    void set_transport(Ref<YNetTransport> p_transport);
    Ref<YNetTransport> get_transport() const { return transport; }

    void remove_from_yrpc_receiving_map(uint32_t p_yrpc_id);

    void update_networked_property_syncers();
    
    void attempt_despawn_nodes_from_peer_that_left(const uint32_t &p_peer_id);

    enum DebuggingLevel {
        NONE = 0,
        MINIMAL = 1,
        MOSTMESSAGES = 2,
        MESSAGESANDPING = 3,
        ALL = 4
    };

    Error _send_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Error _send_and_receive_yrpc(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Error _send_yrpc_to(const Variant **p_args, int p_argcount, Callable::CallError &r_error);
    Error _send_yrpc_direct(Node *p_node, const StringName &p_method, const Variant **p_args, int p_argcount);
    Error _send_yrpc_to_direct(Node *p_node, int p_target_peer, const StringName &p_method, const Variant **p_args, int p_argcount);
    
    int next_networked_spawn_id = 1;


    enum NetworkNodeIdCompression {
        NETWORK_NODE_ID_COMPRESSION_8 = 0,
        NETWORK_NODE_ID_COMPRESSION_16,
        NETWORK_NODE_ID_COMPRESSION_32,
    };
    HashMap<uint32_t,NetworkSpawnedObjectInfo> queued_networked_spawned_objects;
    HashMap<uint32_t,NetworkSpawnedObjectInfo> networked_spawned_objects;
    HashMap<uint32_t,uint32_t> networked_id_to_authority;
    HashMap<uint32_t,Vector<Ref<YNetPropertySyncer>>> networked_property_syncers;
    HashMap<uint32_t,HashMap<uint8_t,Variant>> queued_received_property_syncers;
    HashMap<uint32_t,HashMap<uint8_t,Variant>> queued_to_send_property_syncers;

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
    void set_protocol(String val) {protocol = val;
        protocol_hash = string_to_hash_id(protocol);
    }
    uint32_t protocol_hash = 0;

    mutable Error last_get_error = OK;

    String url = "";
    String get_url() const {return url;}
    void set_url(String val) {url = val;}

    String sid = "";
    String get_sid() const {return sid;}
    void set_sid(String val) {sid = val;}

    int real_hashed_sid{};
    int get_real_hashed_sid() const {return real_hashed_sid;}
    void set_real_hashed_sid(int val) {}

    int hashed_sid{};
    int get_hashed_sid() const {return hashed_sid;}
    void set_hashed_sid(int val) {}

    Node* find_node_with_net_id(uint32_t p_net_id);

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

    bool has_room() const {return !room_id.is_empty();}

    bool offline_mode = false;
    bool get_offline_mode() const {return offline_mode;}
    void set_offline_mode(bool val) {offline_mode = val;}

    
    bool pause_receive_spawns = false;
    bool get_pause_receive_spawns() const {return pause_receive_spawns;}
    void set_pause_receive_spawns(bool val) {pause_receive_spawns = val;}
    
    bool is_host = false;
    bool get_is_host() const {return is_host;}
    void set_is_host(bool val) {}

    Node *internal_spawn(uint32_t p_network_id, const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name,
                         const NodePath &p_desired_parent, const Variant &p_spawn_pos, const int authority);

    void internal_register_as_networked_node(int p_network_id, Node* node_to_register, const int authority);

    void set_authority_after_entered(Node *node_entered_tree, const Variant &p_spawn_pos, int authority);

    Node *spawn_with_path(const String &p_spawnable_scene_path, const String &p_spawn_name,
                          const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority);

    void rpc_spawn(const uint32_t p_network_id, const uint32_t &p_spawnable_path_id, const String &p_spawn_name, const String &p_desired_parent, const Variant &p_spawn_pos, const int authority);

    void internal_spawn_with_queued_struct(const NetworkSpawnedObjectInfo &p_nsoi);

    Variant create_spawned_lists_variant();

    void unpack_property_syncer_received_value(const Array &received_property_syncer_value);

    void unpack_spawned_list_variants(const Array &received_spawned_list);

    Variant convert_nsoi_to_variant(const NetworkSpawnedObjectInfo &p_nsoi);

    void unpack_spawninfo_from_variant(const Array &received_spawn_info);

    Node *spawn(const Ref<PackedScene> &p_spawnable_scene, const String &p_spawn_name, const NodePath &p_desired_parent, const Variant &p_spawn_pos, int authority);

    void rpc_request_spawned_nodes(uint32_t id_requesting);

    void rpc_respond_with_spawned_nodes(const Array &spawned_nodes_info);

    void rpc_despawn(uint32_t p_network_id);

    void despawn(uint32_t p_network_id);

    void despawn_node(Node *node_to_despawn);

    String server_or_client_str();

    void test_send_sync();

    void send_sync_vars(uint64_t p_cur_usec);
    void rpc_recv_sync_vars(Variant p_synced_vars_data);

    String host_id;
    String get_host_id() const {return host_id;}
    void set_host_id(String val) {host_id = val;}

    int host_id_hashed{};

    uint32_t get_new_network_id();

    YNet* create_room();

    YNet *create_room_with_code(const String &create_room, const String &password = "");

    YNet* join_or_create_room(const String &join_room, const String &password = "");

    YNet* join_room(const String &p_join_room, const String &password = "");
    YNet* leave_room();

    Error set_password(const String &newPassword);

    Error set_max_players(int newMaxPlayers);

    Error set_private(bool newPrivate);

    Error set_can_host_migrate(bool newCanHostMigrate);

    Error set_room_name(const String &newRoomName);

    Error set_extra_info(const String &new_extra_info);

    YNet* get_room_info(const String &roomCode);

    YNet* get_room_list();

    bool was_last_rpc_sender_host() const;

    void on_room_created(const String &p_new_room_id);
    void on_room_joined(const String &p_new_room_id, const String &p_host_id);
    void on_room_error(const String &p_room_id);

    void on_left_room();

    void on_room_info(const Variant &p_room_info);

    void on_room_list(const Variant &p_room_list);

    void on_room_players(const Array &players_array);

    void on_player_join(const String &p_player);

    void on_player_left(const String &p_player);

#ifdef HOST_MIGRATION
    void on_host_migrated(const String &p_new_host);
#endif

    void register_for_yrpcs(Node *p_registering_node, uint32_t registering_id);

    void cleanup_network_state();

    void set_node_cleanup_with_owner(Node* p_node, bool p_cleanup);
    bool get_node_cleanup_with_owner(Node* p_node) const;

    static void set_debug_run_multiple_instances(bool val);
    static bool get_debug_run_multiple_instances();

    void connect_to(const String &p_address);

    static uint32_t string_to_hash_id(const String &p_string);


    int get_transfer_channel() const;
    int get_transfer_mode() const;
    int get_target_peer() const;

    int _target_peer = 0;
    
    _FORCE_INLINE_ static YNet* get_singleton() {return singleton;}
    YNet();

    ~YNet();

    void add_setting(const String& name, const Variant& default_value, Variant::Type type,
            PropertyHint hint = PROPERTY_HINT_NONE, const String& hint_string = "",
            int usage = PROPERTY_USAGE_DEFAULT, bool restart_if_changed = false);

    void transport_disconnect();

};
VARIANT_ENUM_CAST(YNet::DebuggingLevel);
#endif