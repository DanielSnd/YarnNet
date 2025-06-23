# YNet _for Godot 4.4+_

A Godot 4.4+ Custom Module for making online multiplayer games

**Main Features:**
 - **Middle-man Servers**: Uses a node.js socket.io, or an ENet server to connect players together in rooms and relay messages between them, including a way of retrieving room lists and room info from the server. No more worrying about NAT Punchthrough or Port Forwarding.
 - **Simplified Connection API**: Easy one-line connection and room management
 - **Custom RPC/Spawner/Synchronizer**: Methods to replace godot's RPCs, 
 MultiplayerSpawner and MultiplayerSynchronizer. These custom implementations 
 rely on synchronized numeric IDs instead of node paths. If a player doesn't 
 - **Buffered RPCs**: RPCs that persist and are sent to new players when they join
 - **YSnapSyncer3D**: Smooth position/rotation synchronization with interpolation and extrapolation
 - **Automatic Property Synchronization**: Real-time property updates across the network
 - **Room-based Multiplayer**: Create and join rooms with automatic host migration (Host Migration requires modifications to godot source code.)

## How to compile with godot
1. Download godot source
2. Create a new directory under modules directory called yarnnet
3. Copy all the contents of this repository to that new directory modules/yarnnet
4. If you are not using the features for host migration run the standard [godot compile command](https://docs.godotengine.org/en/latest/contributing/development/compiling/) with additional argument `host_migration=no` e.g. `scons platform=windows host_migration=no`
5. If you are using the features for host migration the godot source must include changes from [this repo](https://github.com/DanielSnd/godot/tree/rebased4.3) for the necessary engine changes. Run build command as normal.

## Setting up in Project Settings

Once you have the YarnNet module in your version of the engine, if you want to use it in your project go to settings and enable it.

The protocol setting is any string you'd like to use to identify this project. This string is used to filter rooms in the socket.io server, so you can use the same socket.io server for different projects and make sure that your players aren't trying to join the wrong project's room. You could also include a game version in this protocol string so players from different versions can't join the same room together since that could cause issues with RPCs and things that aren't present in different versions.

![image](https://github.com/DanielSnd/YarnNet/assets/9072324/0a7f78e0-1ce4-4026-8ed7-39ce2304e45b)

## Host Migration

If you want to make use of hos_migration, when compiling use the argument `host_migration=yes` so the module includes the host migration code. The host migration code requires changes to godot engine itself (You can see those changes in my [fork here](https://github.com/godotengine/godot-proposals/issues/7912#issuecomment-1963170915) if you'd like to use host migration.)

## Quick Start Example

Here's a simple example of hosting and connecting:

```gdscript
extends Node3D

static var client:bool = false
static var host:bool = false

func _ready():
	host = OS.has_feature("host")
	client = OS.has_feature("client")
	
	YNet.server_disconnected.connect(_on_server_disconnected)
	YNet.peer_connected.connect(_on_peer_joined)
	YNet.peer_disconnected.connect(_on_peer_left)
	
	if host:
		await get_tree().create_timer(0.1).timeout
		YNet.connect_to_and_create_room("localhost:7777")
		if await YNet.connection_result:
			print("[%d] Connected %s" % [multiplayer.get_unique_id(), YNet.room_id])
			DisplayServer.clipboard_set_primary(YNet.room_id)
		else:
			print("Error hosting: %s" % YNet.get_last_error_message())
	else:
		await get_tree().create_timer(1.0).timeout
		if await YNet.connect_to_and_join_room("localhost:7777", DisplayServer.clipboard_get_primary()).connection_result:
			print("[%d] Connected %s" % [multiplayer.get_unique_id(), YNet.room_id])
		else:
			print("Error joining: %s" % YNet.get_last_error_message())

	YNet.register_for_yrpc(self, 1)
	await get_tree().create_timer(1.3).timeout
	YNet.send_yrpc(receive_rpc_on_any_peer, "Hi folks! I'm %d" % multiplayer.get_unique_id())

@rpc("any_peer", "call_remote")
func receive_rpc_on_any_peer(message_sent):
	print("[%d] Received rpc with message [%s]" % [multiplayer.get_unique_id(), message_sent])

func _on_peer_joined(peer):
	print("[%d] Peer joined %s" % [multiplayer.get_unique_id(), peer])

func _on_peer_left(peer):
	print("[%d] Peer left %s" % [multiplayer.get_unique_id(), peer])

func _on_server_disconnected():
	print("[%d] Server disconnected" % multiplayer.get_unique_id())
```

## Transport Protocols

YNet supports multiple transport protocols:

### ENet Transport (Default)
- **Pros**: Low latency, UDP-based, good for real-time games
- **Cons**: Doesn't work for web-games.
- **Best for**: Low-latency requirements

### Socket.IO Transport
- **Pros**: Works through firewalls, WebSocket-based, good for web games
- **Cons**: Higher latency, requires a Socket.IO server
- **Best for**: Web games, games that need to work through firewalls

## Connection Methods

### Simple Connection Methods
```gdscript
# Connect and create a room
var result:bool = await YNet.connect_to_and_create_room("localhost:7777").connection_result

# Connect and join a specific room
var result:bool = await YNet.connect_to_and_join_room("localhost:7777", "ROOM123").connection_result

# Connect and join with password
var result:bool = await YNet.connect_to_and_join_room_with_password("localhost:7777", "ROOM123", "password").connection_result

# Connect without joining a room
var result:bool = await YNet.connect_to("localhost:7777").connection_result
```

### Advanced Connection Methods
```gdscript
# Specify transport type
YNet.connect_to_and_create_room("localhost:7777", YNet.TRANSPORT_TYPE_ENET)
YNet.connect_to_and_create_room("ws://localhost:7777", YNet.TRANSPORT_TYPE_SOCKETIO)

# Manual connection flow
YNet.connect_to("localhost:7777")
await YNet.connection_result
YNet.create_room()
await YNet.room_created
```


### Socket.IO Server
The Socket.IO server is made in node.js and can be found in the server folder. It's the `server.js` file.

## YNet Multiplayer Spawner

### Defining spawnable scenes:
To use the YNet Multiplayer spawner you need to define the scenes that can be spawned over the network. There's two ways of doing that.

#### Project Settings
One way is to modify the Network Spawnable Scenes array in project settings, adding the node paths there.
![image](https://github.com/DanielSnd/YarnNet/assets/9072324/844fa9d7-5a81-4f37-9b6a-687ab832b373)

#### Through code
The other way is through code with `YNet.add_network_spawnable(file_path)`. You would want to do that before any spawns occur, ideally before you start a connection. It only has to be done once. I usually do it in an Autoload's `_ready()` function. 

Here's an example of adding all the files in a folder as network spawnable:
```gdscript
	for file in DirAccess.get_files_at("res://resources/monsters/"):
		var file_full_path:= "res://resources/monsters/{0}".format({0:file})
		YNet.add_network_spawnable(file_full_path)
```

### Spawning/Despawning a Networked Nodes

The following method can be used in the server to spawn a networked scene: (Only the server can spawn networked scenes)
```gdscript
Node YNet.spawn(spawnable_scene: PackedScene, spawned_name: String, parent_path: NodePath, global_pos: Variant, authority: int = 1)
```
It will automatically instantiate the desired node and place it in the hierarchy under the desired parent_path if it is available. If the desired parent path isn't available it will return null but will queue the spawn to happen once the desired parent path becomes available.

Networked Nodes are spawned with a numeric id that can be accessed with get_meta("_net_id"). This ID is synchronized over the network and can be used to find the node for RPCs and Synchronization.

To despawn a networked scene you can either `queue_free()` them as usual (It handles the despawning logic under the hood) or use the following methods: (Only the host can despawn networked nodes)
```gdscript
void YNet.despawn(network_obj_id: int)
void YNet.despawn_node(node: Node)
```

## YNet RPCs

### RPCs in Networked Nodes

RPCs can be sent from Networked Nodes or children of Networked Nodes. If sending from a child node you have to make sure that the relative nodepath to the parent networked node is the same across the network.

To call an RPC use this method:
```gdscript
Error YNet.send_yrpc(method: Callable, ...) 
```

Here's an example:
```gdscript
func example_send_rpc():
	YNet.send_yrpc(receive_chat_message, "This is a message", 5)

## This will be called remotely with the values "This is a message" and 5:
@rpc("any_peer","call_local")
func receive_chat_message(message:String, number:int):
	print(number,message)
```

### Buffered RPCs

Buffered RPCs are automatically sent to new players when they join the room. They persist until explicitly removed.

```gdscript
# Buffered RPCs are automatically detected by method names ending with "_buffered"
@rpc("any_peer", "call_remote")
func set_player_name_buffered(name: String):
	player_name = name

# Remove a buffered RPC
YNet.remove_buffered_yrpc(set_player_name_buffered)

# Remove all buffered RPCs for a specific network ID
YNet.remove_buffered_yrpc_method(network_id, "")
```

## YNet Properties Synchronization

### Automatically Synchronizing variables in Networked Nodes:

To register variables for automatically serialization use this method: (Only variables in the Root networked object can be synchronized with this method at the moment)
```gdscript
YNetPropertySyncer YNet.register_sync_property(networked_node: Node, property_path: NodePath, authority: int = 1, always_sync: bool = false)
```

Example for a Sprite2D node:

```gdscript
func _ready():
	YNet.register_sync_property(self, "modulate")
```

All SyncProperties get automatically synced on Spawn on clients. They also get automatically Synchronized when their values change. If you want them to always synchronize pass true on the fourth variable.

To control the interval for the automatic synchronizing you can modify these values in the project settings:
![image](https://github.com/DanielSnd/YarnNet/assets/9072324/b138249a-abbb-4693-bc93-1ce605aa4968)

## YSnapSyncer3D

YSnapSyncer3D provides smooth position and rotation synchronization with interpolation and extrapolation. It's perfect for character movement and physics objects.

## Debugging

YNet includes comprehensive debugging options:

```gdscript
# Set debugging level (0-4)
# 0: None
# 1: Minimal
# 2: Most messages
# 3: Messages and ping
# 4: All
YNet.debugging = YNet.DebuggingLevel.ALL
```

## Demo Project (Outdated)

This is a small demo project showing how to connect, create and join rooms. It also includes a multiplayer spawner and multiplayer synchronizer to show the custom MultiplayerPeer working. The spawned "players" are actually texts with the socket.io id for easier debugging. (Outdated)

[YarnNetDemoProject.zip](https://github.com/DanielSnd/YarnNet/files/15455894/YarnNetDemoProject.zip)
