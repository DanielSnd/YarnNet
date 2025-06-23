//
// Created by Daniel on 2024-02-23.
//

#include "ynet_multiplayer_peer.h"

#include "yarnnet.h"
#include "core/core_bind.h"
#include "core/crypto/crypto_core.h"
#include "modules/multiplayer/scene_multiplayer.h"
#include "scene/main/multiplayer_api.h"

void YNetMultiplayerPeer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_string_id", "int_id"), &YNetMultiplayerPeer::get_string_id);
}

void YNetMultiplayerPeer::set_target_peer(int p_target_peer) {
    target_peer = p_target_peer;
    // print_line(vformat("Sending to target peer %d. My peer id %d",p_target_peer,ynet->hashed_sid));
    if (target_peer == ynet->host_id_hashed)
        target_peer = 1;
    ynet->_target_peer = p_target_peer;
}

int YNetMultiplayerPeer::get_packet_peer() const {
    if (!ynet->transport.is_valid()) {
        return 1;
    }
    return ynet->transport->get_packet_peer();
}

int YNetMultiplayerPeer::get_unique_id() const {
    return ynet->is_host ? 1 : ynet->hashed_sid;
}

bool YNetMultiplayerPeer::is_server() const {
    return ynet->is_host;
}

void YNetMultiplayerPeer::close() {
    if (ynet->transport.is_valid()) {
        ynet->transport->transport_disconnect();
    }
}

void YNetMultiplayerPeer::disconnect_peer(int p_peer, bool p_force) {
    if (ynet->transport.is_valid()) {
        ynet->transport->kick_peer(get_string_id(p_peer), p_force);
    }
}

void YNetMultiplayerPeer::poll() {
    // if (ynet != nullptr && ynet->transport.is_valid()) {
    //     ynet->transport->poll();
    // }
}

int YNetMultiplayerPeer::get_available_packet_count() const {
    if (!ynet->transport.is_valid()) {
        return 0;
    }
    return ynet->transport->get_available_packet_count();
}

Error YNetMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
    ERR_FAIL_COND_V(get_connection_status() != CONNECTION_CONNECTED, ERR_UNCONFIGURED);
    if (ynet->transport.is_valid()) {
        return ynet->transport->get_packet(r_buffer, r_buffer_size);
    }
    return OK;
}

Error YNetMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
    if (ynet->transport.is_valid()) {
        if (is_server()) {
            if (target_peer > 0) {
                ERR_FAIL_COND_V(!peers_map.has(target_peer), ERR_DOES_NOT_EXIST);
                ynet->transport->target_peer = peers_map[target_peer];
            } else {
                if (!ynet->transport->target_peer.is_empty()) {
                    ynet->transport->target_peer = "";
                }
            }
            ynet->transport->send_packet(p_buffer, p_buffer_size);
        } else {
            if (!ynet->transport->target_peer.is_empty()) {
                ynet->transport->target_peer = "";
            }
            ynet->transport->send_packet(p_buffer, p_buffer_size);
        }
        return OK;
    }
    return ERR_UNCONFIGURED;
}

void YNetMultiplayerPeer::set_transfer_channel(int p_channel) {
    MultiplayerPeer::set_transfer_channel(p_channel);
}

int YNetMultiplayerPeer::get_transfer_channel() const {
    return MultiplayerPeer::get_transfer_channel();
}

MultiplayerPeer::ConnectionStatus YNetMultiplayerPeer::get_connection_status() const {
    if (!ynet->transport.is_valid()) {
        return CONNECTION_DISCONNECTED;
    }
    auto current_state = ynet->transport->get_current_state();
    switch (current_state) {
        case YNetTransport::STATE_CONNECTING:
            return CONNECTION_CONNECTING;
        case YNetTransport::STATE_OPEN:
            return CONNECTION_CONNECTED;
        case YNetTransport::STATE_CLOSING:
            return CONNECTION_DISCONNECTED;
        case YNetTransport::STATE_CLOSED:
            return CONNECTION_DISCONNECTED;
    }
    return CONNECTION_DISCONNECTED;
}


int YNetMultiplayerPeer::get_max_packet_size() const {
    if (!ynet->transport.is_valid()) {
        return ynet->transport->get_max_packet_size();
    }
    return 65535; // ENet default max packet size
}

void YNetMultiplayerPeer::_clear() {
    connection_status = CONNECTION_DISCONNECTED;
    peers_map.clear();
}

void YNetMultiplayerPeer::set_transfer_mode(TransferMode p_mode) {
    MultiplayerPeer::set_transfer_mode(p_mode);
}

MultiplayerPeer::TransferMode YNetMultiplayerPeer::get_transfer_mode() const {
    return MultiplayerPeer::get_transfer_mode();
}

void YNetMultiplayerPeer::set_refuse_new_connections(bool p_enable) {
    MultiplayerPeer::set_refuse_new_connections(p_enable);
}

bool YNetMultiplayerPeer::is_refusing_new_connections() const {
    return MultiplayerPeer::is_refusing_new_connections();
}

void YNetMultiplayerPeer::on_player_joined(const String &p_player) {
    if (ynet->sid == p_player) return;
    
    bool connection_map_has_player = ynet->transport->connections_map.has(p_player);
    int peer_id = -1;
    if (connection_map_has_player) {
        peer_id = ynet->transport->connections_map[p_player];
        if (p_player == ynet->host_id) {
            peer_id = 1;
            ynet->transport->connections_map[p_player] = 1;
        }
        if (!peers_map.has(peer_id)) {
            peers_map[peer_id] = p_player;

            // If server is going to sync this to others because server relay is enabled,
            // then only emit this if the new player is server or if I'm server.
            if (peer_id == 1 || get_unique_id() == 1) {
                    emit_signal(SNAME("peer_connected"), peer_id);
            }
        }
    }
    if (ynet->debugging > 1) {
        print_line(vformat("[YNetMultiplayerPeer] Attempting to do on player joined on %s sid %s. Had in the connection map? %s Peer id %d", p_player == ynet->host_id ? "Server" : "Client", ynet->sid, connection_map_has_player, peer_id));
    }
    if (peer_id == -1) {
        ERR_PRINT(vformat("[YNetMultiplayerPeer] Peer id is -1. This should not happen. Peer id %d SID %s", peer_id, p_player));
    }
}

void YNetMultiplayerPeer::on_player_left(const String &p_player) {
    if (ynet->sid == p_player) return;
    Vector<int> remove_ints;
    for (auto key_value: peers_map) {
        if (key_value.value == p_player) {
            remove_ints.append(key_value.key);
        }
    }
    for (auto remove_int: remove_ints) {
        if(remove_int != 1) {
            YNet::get_singleton()->attempt_despawn_nodes_from_peer_that_left(remove_int);
            emit_signal(SNAME("peer_disconnected"), remove_int);
        }
        if (peers_map.has(remove_int)) {
            peers_map.erase(remove_int);
        }
    }
}

void YNetMultiplayerPeer::on_room_connected(const int &p_player) {
    on_player_joined(ynet->sid);
}

void YNetMultiplayerPeer::on_room_disconnected(const int &p_player) {
    on_player_left(ynet->sid);
}

void YNetMultiplayerPeer::on_multiplayer_api_peer_connected(const int &peer_id) {
    if (YNet::get_singleton()->get_debugging() > 1) {
        print_line("on_multiplayer_api_peer_connected id ",peer_id);
    }
}

// #ifdef HOST_MIGRATION
// void YNetMultiplayerPeer::on_host_migration(const String &p_new_host) {
//     int previous_hashed_for_new_host = ynet->string_to_hash_id(p_new_host);

//     Vector<int> remove_ints;
//     for (auto key_value: peers_map) {
//         if (key_value.value == p_new_host) {
//             remove_ints.append(key_value.key);
//         }
//     }
//     for (auto remove_int: remove_ints) {
//         if (peers_map.has(remove_int)) {
//             peers_map.erase(remove_int);
//         }
//     }

//     ynet->connections_map[p_new_host] = 1;
//     peers_map[1] = p_new_host;

//     Vector<ObjectID> objects_to_despawn;
//     Ref<SceneMultiplayer> scene_multiplayer = (ynet->get_tree()->get_multiplayer(ynet->get_path()));
//     const String migrate_meta ="migrate";

//     ERR_FAIL_COND_MSG(!scene_multiplayer.is_valid(),ERR_DOES_NOT_EXIST);
//     HashSet<ObjectID> sync_nodes_to_further_transfer_after;
//     if (scene_multiplayer.is_valid()) {
//         HashSet<ObjectID> sync_nodes = scene_multiplayer->get_replicator_object_ids();
//         for (const ObjectID &oid : sync_nodes) {
//             MultiplayerSynchronizer *sync = get_id_as_synchronizer(oid);
//             if(!sync) continue;
//             if (sync->get_multiplayer_authority() == previous_hashed_for_new_host) {
//                 sync->set_multiplayer_authority(1);
//                 sync_nodes_to_further_transfer_after.insert(oid);
//             } else if (sync->get_multiplayer_authority() == 1 && (!sync->has_meta(migrate_meta) || !(sync->get_meta(migrate_meta,false)))) {
//                 objects_to_despawn.push_back(oid);
//             }
//         }
//     }

//     if (objects_to_despawn.size() > 0) {
//         for (auto remove_int: objects_to_despawn) {
//             MultiplayerSynchronizer *sync = get_id_as_synchronizer(remove_int);
//             if(!sync) continue;
//             sync->get_root_node()->queue_free();
//         }
//     }

//     if (scene_multiplayer.is_valid()) {
//         scene_multiplayer->transfer_peer_id_ownership(previous_hashed_for_new_host,1, true);
//     }

//     for (const ObjectID &oid : sync_nodes_to_further_transfer_after) {
//         MultiplayerSynchronizer *sync = get_id_as_synchronizer(oid);
//         if(!sync) continue;
//         Node* get_node = sync->get_root_node();
//         if (get_node != nullptr && get_node->get_multiplayer_authority() == previous_hashed_for_new_host) {
//             // Here I'm not setting it recursively on the root because I could have a synchronizer with different authority as a direct child
//             get_node->set_multiplayer_authority(1, false);
//             for (int i = 0; i < get_node->get_child_count(); ++i) {
//                 Node* child_node = get_node->get_child(i);
//                 if (child_node != nullptr && child_node->get_multiplayer_authority() == previous_hashed_for_new_host) {
//                     // Here I am setting it recursively because I would need to create a new recursive method if I
//                     // wanted to do this checking further, and at least on my own use it won't matter this deep into the node path.
//                     child_node->set_multiplayer_authority(true);
//                 }
//             }
//         }
//     }
// }
// #endif

String YNetMultiplayerPeer::get_string_id(int _int_id) const {
    if (peers_map.has(_int_id)) {
        return peers_map[_int_id];
    }
    for (auto key_value: ynet->transport->connections_map) {
        if (key_value.value == _int_id) {
            return key_value.key;
        }
    }
    return vformat("%d",_int_id);
}

YNetMultiplayerPeer::YNetMultiplayerPeer() {
    if (Engine::get_singleton()->is_editor_hint()) return;
    ynet = YNet::get_singleton();
    ynet->connect(SNAME("room_connected"),callable_mp(this,&YNetMultiplayerPeer::on_room_connected));
    ynet->connect(SNAME("room_disconnected"),callable_mp(this,&YNetMultiplayerPeer::on_room_disconnected));
    ynet->connect(SNAME("player_joined"),callable_mp(this,&YNetMultiplayerPeer::on_player_joined));
    ynet->connect(SNAME("player_left"),callable_mp(this,&YNetMultiplayerPeer::on_player_left));
// #ifdef HOST_MIGRATION
//     ynet->connect(SNAME("host_migration"),callable_mp(this,&YNetMultiplayerPeer::on_host_migration));
// #endif
}

YNetMultiplayerPeer::~YNetMultiplayerPeer() {
    ynet = nullptr;
    _clear();
}
