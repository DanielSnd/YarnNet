# YNet _for Godot 4.2+_

A Godot 4.2+ Custom Module for making online multiplayer games.

**Main Features:**
 - A custom multiplayer peer that uses a node.js socket.io server to connect players together in rooms and relay messages between them, including a way of retrieving room lists and room info from the server.
 - Custom RPC/Spawner/Synchronizer methods to replace godot's RPCs, MultiplayerSpawner and MultiplayerSynchronizer. These custom implementations rely on synchronized numeric IDs instead of node paths. If a player doesn't currently have the desired parent node path for a networked node it will queue the spawn/synchronization calls until it does.

I'm mostly doing this for my own personal use so things are messier than they probably should be, and I haven't been documenting things. I'll try to do some more documentation and add a demo if anyone wants to try it.

# Getting Started

You need to compile the module with Godot Engine (4.2+). When compiling use the argument `host_migration=no` so the module doesn't include the host migration code. The host migration code requires changes to godot engine itself (You can see those changes in my [fork here](https://github.com/godotengine/godot-proposals/issues/7912#issuecomment-1963170915) if you'd like to use host migration.)

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

## Server

The server is made in node.js/socket.io and can be found in the server folder. It's the `server.js` file.

## Hosting a room:

```gdscript
func _on_host_pressed() -> void:
	var peer:MultiplayerPeer = YNetMultiplayerPeer.new()
	YNet.transport = YNetSocketIO.new()
	print("Await for connection attempt")
  ## First step is to connect to the socket.io server. After calling YNet.ynet_connect you need to await for the socket.io connection to happen. If result[1] is false then the connection failed. result[0] is an error String.
	var result = await YNet.ynet_connect("http://localhost:8211").connected
	if !result[1]:
		printerr("Failed to connect to ynet socket io %s" % result[0])
		return

  ## If the connection succeeded add the created peer to the multiplayer peer.
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(on_peer_connected)
	multiplayer.peer_disconnected.connect(on_peer_disconnected)

  ## At this point you can now create a room. After requesting a room creation you need to await for the result of the room creation, if result[1] is false then the room creation failed. result[0] is an error String.
	result = YNet.create_room().room_connection_result
	if !result[1]:
		printerr("Failed to create room %s" % result[0])
		multiplayer.peer_connected.disconnect(on_peer_connected)
		multiplayer.multiplayer_peer = null
		return

  ## If the room creation succeeded you can now get the room id without the protocool with `YNet.room_id_without_protocol` This is what you show the player so they can give their friends. The real room id on socket.io includes the protocol after it.
	print_to_screen("Room created: "+YNet.room_id_without_protocol)
```

## Connecting to a room.

The first part is the same as the way to Host a game. It only changes once you are connected to Socket.io, then instead of creating a room you join a room.


```gdscript
func _on_join_pressed() -> void:
	var peer:MultiplayerPeer = YNetMultiplayerPeer.new()
	YNet.transport = YNetSocketIO.new()
	print("Await for connection attempt")
  ## First step is to connect to the socket.io server. After calling YNet.ynet_connect you need to await for the socket.io connection to happen. If result[1] is false then the connection failed. result[0] is an error String.
	var result = await YNet.ynet_connect("http://localhost:8211").connected
	if !result[1]:
		printerr("Failed to connect to ynet socket io %s" % result[0])
		return

  ## If the connection succeeded add the created peer to the multiplayer peer.
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(on_peer_connected)
	multiplayer.peer_disconnected.connect(on_peer_disconnected)

  ## At this point you can now join a room. With YNet.join_or_create_room it will try to join the room,
  ## but if it can't find the room to join it will create a room with the provided room id. This can be used to create rooms with specific room ids.
  ## If you pass an empty string it will attempt to join any valid room in the server, or create a new one if it can't find any.
  ## After calling join_or_create_room you need to wait for the result of the join attempt. If result[1] is false then it failed.
	result = YNet.join_or_create_room(line_edit.text).room_connection_result
	if !result[1]:
		printerr("Failed to join room. %s" % result[0])
		multiplayer.peer_connected.disconnect(on_peer_connected)
		multiplayer.multiplayer_peer = null
		return

  ## The connection should now be established.
```

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
Or if you would like to also run the RPC locally besides calling it remotely:
```gdscript
Error YNet.send_and_receive_yrpc(method: Callable, ...) vararg
```

Here's an example:
```gdscript
func example_send_rpc():
	YNet.send_yrpc(receive_chat_message, "This is a message", 5)

## This will be called remotely with the values "This is a message" and 5:
func receive_chat_message(message:String, number:int):
	print(number,message)
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


# Demo Project

This is a small demo project showing how to connect, create and join rooms. It also includes a multiplayer spawner and multiplayer synchronizer to show the custom MultiplayerPeer working. The spawned "players" are actually texts with the socket.io id for easier debugging.

[YarnNetDemoProject.zip](https://github.com/DanielSnd/YarnNet/files/15455894/YarnNetDemoProject.zip)
