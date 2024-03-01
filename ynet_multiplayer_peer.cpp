//
// Created by Daniel on 2024-02-23.
//

#include "ynet_multiplayer_peer.h"

#include "yarnnet.h"
#include "core/core_bind.h"
#include "core/crypto/crypto_core.h"
#include "modules/multiplayer/scene_multiplayer.h"
#include "scene/main/multiplayer_api.h"

void YnetMultiplayerPeer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_string_id", "int_id"), &YnetMultiplayerPeer::get_string_id);
}

void YnetMultiplayerPeer::set_target_peer(int p_target_peer) {
    target_peer = p_target_peer;
    if (target_peer == ynet->host_id_hashed)
        target_peer = 1;
}

int YnetMultiplayerPeer::get_packet_peer() const {
    ERR_FAIL_COND_V_MSG(ynet->unhandled_packets.size() == 0, 1, "No packets to get!");
    auto source = ynet->unhandled_packets.front()->get().source;
    if (source == ynet->host_id_hashed)
        source = 1;
    return source;
}

int YnetMultiplayerPeer::get_unique_id() const {
    return ynet->is_host ? 1 : ynet->hashed_sid;
}

bool YnetMultiplayerPeer::is_server() const {
    return ynet->is_host;
}

void YnetMultiplayerPeer::close() {
    YNet::get_singleton()->engineio_disconnect();
}

void YnetMultiplayerPeer::disconnect_peer(int p_peer_id, bool p_force) {
    //TODO: HANDLE KICK
}

void YnetMultiplayerPeer::poll() {
}

int YnetMultiplayerPeer::get_available_packet_count() const {
	return ynet->unhandled_packets.size();
}

Error YnetMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
    ERR_FAIL_COND_V(get_connection_status() != CONNECTION_CONNECTED, ERR_UNCONFIGURED);

    r_buffer_size = 0;

    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }

    ERR_FAIL_COND_V(ynet->unhandled_packets.is_empty(), ERR_UNAVAILABLE);

    current_packet = ynet->unhandled_packets.front()->get();
    ynet->unhandled_packets.pop_front();

    *r_buffer = current_packet.data;
    r_buffer_size = current_packet.size;

    return OK;
}

Error YnetMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
    ERR_FAIL_COND_V(get_connection_status() != CONNECTION_CONNECTED, ERR_UNCONFIGURED);
    auto stringified_packet = CryptoCore::b64_encode_str(p_buffer,p_buffer_size);
    if (is_server()) {
        // I'M SERVER
        if (target_peer > 0) {
            ERR_FAIL_COND_V(!peers_map.has(target_peer), ERR_DOES_NOT_EXIST);
            // FIND PEER ID?
            Array packet_data;
            packet_data.append(YNet::get_singleton()->room_id);
            packet_data.append(peers_map[target_peer]);
            packet_data.append(stringified_packet);
            ynet->socketio_send("pkt2cid",packet_data);
            return OK;
        } else {
            // SEND TO ALL
            Array packet_data;
            packet_data.append(YNet::get_singleton()->room_id);
            packet_data.append(stringified_packet);
            ynet->socketio_send("pkt2clients",packet_data);
        }
        return OK;
    }
    //I'M CLIENT
    Array packet_data;
    packet_data.append(ynet->room_id);
    packet_data.append(stringified_packet);
    ynet->socketio_send("pkt2serv",packet_data);
    return OK;
}

void YnetMultiplayerPeer::set_transfer_channel(int p_channel) {
    MultiplayerPeer::set_transfer_channel(p_channel);
}

int YnetMultiplayerPeer::get_transfer_channel() const {
    return MultiplayerPeer::get_transfer_channel();
}

MultiplayerPeer::ConnectionStatus YnetMultiplayerPeer::get_connection_status() const {
    auto current_state = ynet->get_current_state();
    switch (current_state) {
        case YNet::STATE_CONNECTING:
            return CONNECTION_CONNECTING;
            break;
        case YNet::STATE_OPEN:
            return CONNECTION_CONNECTED;
            break;
        case YNet::STATE_CLOSING:
            return CONNECTION_DISCONNECTED;
            break;
        case YNet::STATE_CLOSED:
            return CONNECTION_DISCONNECTED;
    }
    return CONNECTION_DISCONNECTED;
}


int YnetMultiplayerPeer::get_max_packet_size() const {
    return ynet->client->get_max_packet_size();
}

void YnetMultiplayerPeer::_clear() {
    connection_status = CONNECTION_DISCONNECTED;
    peers_map.clear();

    if (current_packet.data != nullptr) {
        memfree(current_packet.data);
        current_packet.data = nullptr;
    }
}

void YnetMultiplayerPeer::set_transfer_mode(TransferMode p_mode) {
    MultiplayerPeer::set_transfer_mode(p_mode);
}

MultiplayerPeer::TransferMode YnetMultiplayerPeer::get_transfer_mode() const {
    return MultiplayerPeer::get_transfer_mode();
}

void YnetMultiplayerPeer::set_refuse_new_connections(bool p_enable) {
    MultiplayerPeer::set_refuse_new_connections(p_enable);
}

bool YnetMultiplayerPeer::is_refusing_new_connections() const {
    return MultiplayerPeer::is_refusing_new_connections();
}

void YnetMultiplayerPeer::on_player_joined(const String &p_player) {
    if (ynet->sid == p_player) return;
    print_line("Attempting to do on player joined on client ",ynet->sid," player that joined in theory is ",p_player," has in map? ",ynet->connections_map.has(p_player));
    if (ynet->connections_map.has(p_player)) {
        int peer_id = ynet->connections_map[p_player];
        if (p_player == ynet->host_id) {
            peer_id = 1;
            ynet->connections_map[p_player] = 1;
        }
        if (!peers_map.has(peer_id)) {
            peers_map[peer_id] = p_player;

            // If server is going to sync this to others because server relay is enabled,
            // then only emit this if the new player is server or if I'm server.
            if (peer_id == 1 || get_unique_id() == 1)
                emit_signal(SNAME("peer_connected"), peer_id);
        }
    }
}

void YnetMultiplayerPeer::on_player_left(const String &p_player) {
    if (ynet->sid == p_player) return;
    Vector<int> remove_ints;
    for (auto key_value: peers_map) {
        if (key_value.value == p_player) {
            remove_ints.append(key_value.key);
        }
    }
    for (auto remove_int: remove_ints) {
        if (peers_map.has(remove_int)) {
            peers_map.erase(remove_int);
            if(remove_int != 1) {
                emit_signal(SNAME("peer_disconnected"), remove_int);
            }
        }
    }
}

void YnetMultiplayerPeer::on_room_connected(const int &p_player) {
    on_player_joined(ynet->sid);
}

void YnetMultiplayerPeer::on_room_disconnected(const int &p_player) {
    on_player_left(ynet->sid);
}

void YnetMultiplayerPeer::on_multiplayer_api_peer_connected(const int &peer_id) {
    print_line("on_multiplayer_api_peer_connected id ",peer_id);
}

void YnetMultiplayerPeer::on_host_migration(const String &p_new_host) {
    int previous_hashed_for_new_host = ynet->string_to_hash_id(p_new_host);

    Vector<int> remove_ints;
    for (auto key_value: peers_map) {
        if (key_value.value == p_new_host) {
            remove_ints.append(key_value.key);
        }
    }
    for (auto remove_int: remove_ints) {
        if (peers_map.has(remove_int)) {
            peers_map.erase(remove_int);
        }
    }

    ynet->connections_map[p_new_host] = 1;
    peers_map[1] = p_new_host;

    Vector<ObjectID> objects_to_despawn;
    Ref<SceneMultiplayer> scene_multiplayer = (ynet->get_tree()->get_multiplayer(ynet->get_path()));
    const String migrate_meta ="migrate";

    ERR_FAIL_COND_MSG(!scene_multiplayer.is_valid(),ERR_DOES_NOT_EXIST);
    HashSet<ObjectID> sync_nodes_to_further_transfer_after;
    if (scene_multiplayer.is_valid()) {
        HashSet<ObjectID> sync_nodes = scene_multiplayer->get_replicator_object_ids();
        for (const ObjectID &oid : sync_nodes) {
            MultiplayerSynchronizer *sync = get_id_as_synchronizer(oid);
            if(!sync) continue;
            if (sync->get_multiplayer_authority() == previous_hashed_for_new_host) {
                sync->set_multiplayer_authority(1);
                sync_nodes_to_further_transfer_after.insert(oid);
            } else if (sync->get_multiplayer_authority() == 1 && (!sync->has_meta(migrate_meta) || !(sync->get_meta(migrate_meta,false)))) {
                objects_to_despawn.push_back(oid);
            }
        }
    }

    if (objects_to_despawn.size() > 0) {
        for (auto remove_int: objects_to_despawn) {
            MultiplayerSynchronizer *sync = get_id_as_synchronizer(remove_int);
            if(!sync) continue;
            sync->get_root_node()->queue_free();
        }
    }

    if (scene_multiplayer.is_valid()) {
        scene_multiplayer->transfer_peer_id_ownership(previous_hashed_for_new_host,1, true);
    }

    for (const ObjectID &oid : sync_nodes_to_further_transfer_after) {
        MultiplayerSynchronizer *sync = get_id_as_synchronizer(oid);
        if(!sync) continue;
        Node* get_node = sync->get_root_node();
        if (get_node != nullptr && get_node->get_multiplayer_authority() == previous_hashed_for_new_host) {
            // Here I'm not setting it recursively on the root because I could have a synchronizer with different authority as a direct child
            get_node->set_multiplayer_authority(1, false);
            for (int i = 0; i < get_node->get_child_count(); ++i) {
                Node* child_node = get_node->get_child(i);
                if (child_node != nullptr && child_node->get_multiplayer_authority() == previous_hashed_for_new_host) {
                    // Here I am setting it recursively because I would need to create a new recursive method if I
                    // wanted to do this checking further, and at least on my own use it won't matter this deep into the node path.
                    child_node->set_multiplayer_authority(true);
                }
            }
        }
    }
}

String YnetMultiplayerPeer::get_string_id(int _int_id) const {
    if (peers_map.has(_int_id)) {
        return peers_map[_int_id];
    }
    for (auto key_value: ynet->connections_map) {
        if (key_value.value == _int_id) {
            return key_value.key;
        }
    }
    return vformat("%d",_int_id);
}

YnetMultiplayerPeer::YnetMultiplayerPeer() {
    if (Engine::get_singleton()->is_editor_hint()) return;
    ynet = YNet::get_singleton();
    ynet->connect(SNAME("room_connected"),callable_mp(this,&YnetMultiplayerPeer::on_room_connected));
    ynet->connect(SNAME("room_disconnected"),callable_mp(this,&YnetMultiplayerPeer::on_room_disconnected));
    ynet->connect(SNAME("player_joined"),callable_mp(this,&YnetMultiplayerPeer::on_player_joined));
    ynet->connect(SNAME("player_left"),callable_mp(this,&YnetMultiplayerPeer::on_player_left));
    ynet->connect(SNAME("host_migration"),callable_mp(this,&YnetMultiplayerPeer::on_host_migration));
    ynet->get_tree()->get_multiplayer(ynet->get_path())->connect(SNAME("peer_connected"),callable_mp(this,&YnetMultiplayerPeer::on_multiplayer_api_peer_connected));
}

YnetMultiplayerPeer::~YnetMultiplayerPeer() {
    ynet = nullptr;
    _clear();
}
