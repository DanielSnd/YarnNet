//
// Created by Daniel on 2024-02-23.
//

#ifndef YNET_MULTIPLAYER_PEER_H
#define YNET_MULTIPLAYER_PEER_H
#include "yarnnet.h"
#include "modules/websocket/websocket_peer.h"
#include "scene/main/multiplayer_peer.h"
#include "core/error/error_list.h"
#include "core/io/stream_peer_tls.h"
#include "core/templates/list.h"
#include "modules/multiplayer/multiplayer_synchronizer.h"

class YnetMultiplayerPeer : public MultiplayerPeer {
	GDCLASS(YnetMultiplayerPeer, MultiplayerPeer);

private:

protected:
	enum {
		SYS_NONE = 0,
		SYS_ADD = 1,
		SYS_DEL = 2,
		SYS_ID = 3,
		PROTO_SIZE = 9
	};

	ConnectionStatus connection_status = CONNECTION_DISCONNECTED;

	HashMap<int, String> peers_map;
	YNet::Packet current_packet;

	int target_peer = 0;

	YNet* ynet;
	static void _bind_methods();

	static MultiplayerSynchronizer *get_id_as_synchronizer(const ObjectID &p_id) {
		return p_id.is_valid() ? Object::cast_to<MultiplayerSynchronizer>(ObjectDB::get_instance(p_id)) : nullptr;
	}
	void _clear();
public:
	/* MultiplayerPeer */
	virtual void set_target_peer(int p_target_peer) override;
	virtual int get_packet_peer() const override;
	virtual int get_packet_channel() const override { return 0; }
	virtual TransferMode get_packet_mode() const override { return TRANSFER_MODE_RELIABLE; }
	virtual int get_unique_id() const override;
	virtual bool is_server_relay_supported() const override { return true; }

	virtual int get_max_packet_size() const override;

	virtual bool is_server() const override;
	virtual void close() override;
	virtual void disconnect_peer(int p_peer_id, bool p_force = false) override;
	virtual void poll() override;

	virtual ConnectionStatus get_connection_status() const override;

	/* PacketPeer */
	virtual int get_available_packet_count() const override;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;

	virtual void set_transfer_channel(int p_channel) override;
	virtual int get_transfer_channel() const override;
	virtual void set_transfer_mode(TransferMode p_mode) override;
	virtual TransferMode get_transfer_mode() const override;
	virtual void set_refuse_new_connections(bool p_enable) override;
	virtual bool is_refusing_new_connections() const override;

	void on_player_joined(const String &p_player);

	void on_player_left(const String &p_player);

	void on_room_connected(const int &p_player);

	void on_room_disconnected(const int &p_player);

#ifdef HOST_MIGRATION
	void on_host_migration(const String &p_new_host);
#endif

	void on_multiplayer_api_peer_connected(const int &peer_id);

	String get_string_id(int _int_id) const;

	YnetMultiplayerPeer();


	~YnetMultiplayerPeer();
};



#endif //YNET_MULTIPLAYER_PEER_H
