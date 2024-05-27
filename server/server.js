const express = require("express");
const app = express();
const port = 8211;
const fs = require('fs');
const http = require('http');
const https = require('https');
const path = require('path');
const bodyParser = require('body-parser');
app.use(bodyParser.urlencoded({ extended: true }));

const options = {
  pingInterval: 30005,
  pingTimeout: 5000,
  upgradeTimeout: 3000,
  allowUpgrades: true,
  cookie: false,
  serveClient: true,
  cors: {
    origin: "*"
  },
};

const sqlite3 = require('sqlite3').verbose();
const db = new sqlite3.Database('mydatabase.db');

// Determine whether to use HTTP or HTTPS based on a command-line argument
const isHttps = process.argv.includes('--https');

// Choose the appropriate server module
const serverModule = isHttps ? https : http;

const server = serverModule.createServer(options, app);

server.listen(port, () => {
  console.log(`Server listening at port ${port} with ${isHttps ? 'HTTPS' : 'HTTP'}`);
});

let rooms = {};

const io = require("socket.io")(server, {
  cors: {
    origin: '*'
  }
});
app.use(express.static(path.join(__dirname, 'public')));
app.get('/', function(request, response) {
  // HTML form with password input and checkboxes for message selection
    let room_list_html = "";  // to store the HTML for room list

    // loop through each room in rooms object and generate list item HTML
    for (let room_code in rooms) {
        let room_name = rooms[room_code].roomName;
        let player_count = Object.keys(rooms[room_code].clients).length + 1;  // assuming each room object has a playerCount property

        // append the list item HTML to room_list_html
        room_list_html += `<li>Room Code: ${room_code}, Room Name: ${room_name}, Player count: ${player_count}</li>`;
    }
  const htmlContent = `
    <html>
    <head>
    <title>Server</title>
      <style>
        body {
          background-color: #333; /* Dark Gray */
          color: #eee; /* Off-White / Very Light Gray */
        }
      </style>
    </head>
    <body><h1> ${io.engine.clientsCount} connected</h1></body>
    <ul>
        ${room_list_html}
      </ul>
      </html>`;
    
  response.send(htmlContent);
});

function handleRoomJoin(roomCode, socket, sent_password) {
    if (rooms[roomCode]) {
        if (rooms[roomCode].password === "" || rooms[roomCode].password === sent_password) {
            let amount_of_clients = Object.keys(rooms[roomCode].clients).length + 1
            if (amount_of_clients < rooms[roomCode].maxPlayers) {
                socket.join(roomCode);
                rooms[roomCode].clients[socket.id] = socket.id;
                const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
                if (hostSocketId) {
                    socket.emit('roomjoined', roomCode, rooms[roomCode].host);
                } else {
                    console.log(`No valid host found for room code: ${roomCode}`);
                    rooms[roomCode].host = socket.id;
                    delete rooms[roomCode].clients[rooms[roomCode].host];
                    io.to(roomCode).emit('newhost', socket.id);
                    socket.emit('roomjoined', roomCode, socket.id);
                }

                // Collect all socket IDs in the room except the newly joined one
                const socketIdsInRoom = Object.values(rooms[roomCode].clients).filter(id => id !== socket.id).concat(rooms[roomCode].host);

                // Emit the array of socket IDs to the player who just joined
                socket.emit('roomplayers', socketIdsInRoom);

                io.to(roomCode).except(socket.id).emit('playerjoin', socket.id);
                console.log(`User ${socket.id} has joined room ${roomCode} which has host ${rooms[roomCode].host}`);
            } else {
                socket.emit('roomerror', 'This room is full');
                console.log(`Room is full ${roomCode}. Current player number ${amount_of_clients} max players allowed ${rooms[roomCode].maxPlayers}`);
            }
        } else {
            if (sent_password === "") {
                socket.emit('roomerror', 'This room requires a password.');
                console.log(`Room requires a password ${roomCode}`);
            } else {
                socket.emit('roomerror', 'Wrong password.');
                console.log(`Wrong password ${roomCode}`);
            }
        }
    } else {
        socket.emit('roomerror', 'This room does not exist');
        console.log(`Room doesn't exist ${roomCode}`);
    }
}

function createNewRoom(roomCode, protocol, socket) {
    while (rooms[roomCode]) {
        roomCode = generateRoomCode() + protocol;
    }
    rooms[roomCode] = {
        host: socket.id,
        roomName: "",
        private: false,
        password: "",
        maxPlayers: 32,
        can_hostMigrate: false,
        extra_info: "",
        clients: {}
    };
    socket.join(roomCode);
    socket.emit('roomcreated', roomCode);
    console.log(`Room ${roomCode} has been created by ${socket.id}`);
}

io.on('connection', (socket) => {
    console.log(`player joined ${socket.id}`);

    socket.on('leaveroom', () => {
        handlePlayerLeavingRoom(socket);
        socket.emit('leftroom');
    });

    socket.on('disconnect', function(reason) {
        handlePlayerLeavingRoom(socket);
        console.log(`user disconnected Reason: ${reason}`);
    });

    socket.on('requestroom', (protocol) => {
        let roomCode = generateRoomCode() + protocol;
        createNewRoom(roomCode, protocol, socket);
    });


    socket.on('joinroomwithpassword', (roomCode, roompassword) => {
        handleRoomJoin(roomCode, socket, roompassword)
    });

    socket.on('joinroom', (roomCode) => {
        handleRoomJoin(roomCode, socket, "");
    });

    socket.on('joinOrCreateRoom', (roomCode) => {    
        if (!roomCode || roomCode.trim() === "") {
            // Room code is null or empty, generate a new one
            roomCode = generateRoomCode();
        }
        if (rooms[roomCode]) {
            handleRoomJoin(roomCode, socket, "");
        } else {
            // Create and join a new room
            createNewRoom(roomCode,roomCode,socket);
        }
    });

    socket.on('message', (roomCode, message) => {
        socket.to(roomCode).emit('message', message);
    });

    socket.on('message', (message) => {
        console.log("received message:",message)
    });
    
    socket.on('beep', (roomCode, rpctype) => {
        console.log(`Received 'beep' event for room code: ${roomCode}`);
        socket.to(roomCode).emit('boop', rpctype);
    });

    socket.on('rpctoserver', (roomCode, rpctype, nob,buff) => {
        console.log(`Received 'rpctoserver' event for room code: ${roomCode}`);
        const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
        if (hostSocketId) {
            //console.log(`Found valid host with socket ID: ${hostSocketId}`);
            io.to(hostSocketId).emit('rpc', socket.id, rpctype,nob, buff);
        } else {
            console.log(`No valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('rpctoclients', (roomCode, rpctype, nob,buff) => {
        //console.log(`Received 'rpctoclients' event for room code: ${roomCode}`);
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            //console.log(`Valid host with socket ID: ${socket.id} is sending RPC to clients`);
            socket.to(roomCode).emit('rpc', socket.id, rpctype,nob, buff);
        } else {
            console.log(`Invalid host or no host found for room code: ${roomCode}`);
        }
    });

    socket.on('rpctoclientid', (roomCode, targetSocketId, rpctype,nob, buff) => {
        //console.log(`Received 'rpctoclientid' event for socket id: ${targetSocketId} in room: ${roomCode}`);
        // Check if the current socket is the host
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            const clients = Object.keys(rooms[roomCode].clients);
            if (clients.includes(targetSocketId)) {
                //console.log(`Valid host with socket ID: ${socket.id} is sending RPC to client with socket ID: ${targetSocketId}`);
                socket.to(targetSocketId).emit('rpc', socket.id, rpctype,nob, buff);
            } else {
                console.log(`Target client with socket id: ${targetSocketId} not found in room: ${roomCode}`);
            }
        } else {
            console.log(`Invalid host or no host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_password', (roomCode, new_password) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].password = new_password;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_max_players', (roomCode, new_max_players) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].maxPlayers = new_max_players;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_private', (roomCode, new_private) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].private = new_private;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_can_host_migrate', (roomCode, new_can_host_migrate) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].can_hostMigrate = new_can_host_migrate;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_room_name', (roomCode, new_room_name) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].roomName = new_room_name;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('set_extra_info', (roomCode, new_extra_info) => {
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            rooms[roomCode].extra_info = new_extra_info;
        } else {
            console.log(`Invalid host or no valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('get_room_info', (roomCode) => {
        if (rooms[roomCode]) {
            let roomCopy = {...rooms[roomCode]};  // copy the object
            delete roomCopy.password;             // delete the password property from the new object
            socket.emit("roominfo",roomCopy)
        } else {
            console.log(`Invalid room: ${roomCode}`);
        }
    });

    socket.on('get_room_list', (protocol) => {
        let room_codes = Object.keys(rooms).filter(code => code.endsWith(protocol));
        let room_codes_minus_protocol = room_codes.map(code => code.slice(0, -protocol.length));
        let room_info_array = room_codes_minus_protocol.map(code => {
            let room = rooms[code + protocol];  // Get the room object for this code
            if (room && !room.private) { // Check whether the room exists
                return [code, room.roomName, Object.keys(room.clients).length + 1, room.maxPlayers, room.extra_info, room.password !== ""];
            }
        });
        room_info_array = room_info_array.filter(i => i);
        socket.emit("roomlist", room_info_array);
    });

    socket.on('pkt2serv', (roomCode, packetcontent) => {
        //console.log(`Received 'pkt2serv' event for room code: ${roomCode} content ${packetcontent}`);
        const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
        if (hostSocketId) {
            //console.log(`Found valid host with socket ID: ${hostSocketId}`);
            const clients = Object.keys(rooms[roomCode].clients);
            if (clients.includes(socket.id)) {
                io.to(hostSocketId).emit('pkt', socket.id, packetcontent);
            } else {
                console.log(`Receive message to room: ${roomCode} from a client that's not in the room ${socket.id}`);
            }
        } else {
            console.log(`No valid host found for room code: ${roomCode}`);
        }
    });

    socket.on('pkt2clients', (roomCode, packetcontent) => {
        //console.log(`Received 'pkt2clients' event for room code: ${roomCode} content ${packetcontent}`);
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            //console.log(`Valid host with socket ID: ${socket.id} is sending RPC to clients`);
            socket.to(roomCode).except(socket.id).emit('pkt', socket.id, packetcontent);
        } else {
            console.log(`Invalid host or no host found for room code: ${roomCode}`);
        }
    });

    socket.on('pkt2cid', (roomCode, targetSocketId, packetcontent) => {
        //console.log(`Received 'pkt2cid' event for socket id: ${targetSocketId} in room: ${roomCode} content ${packetcontent}`);
        // Check if the current socket is the host
        if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
            const clients = Object.keys(rooms[roomCode].clients);
            if (clients.includes(targetSocketId)) {
                //console.log(`Valid host with socket ID: ${socket.id} is sending RPC to client with socket ID: ${targetSocketId}`);
                socket.to(targetSocketId).emit('pkt', socket.id, packetcontent);
            } else {
                console.log(`Target client with socket id: ${targetSocketId} not found in room: ${roomCode}`);
            }
        } else {
            console.log(`Invalid host or no host found for room code: ${roomCode}`);
        }
    });
});

function generateRoomCode() {
    const hex = "ABCDEFRTPXZSQHJYWKN123456789";
    let code = "";
    for (let i = 0; i < 6; i++) {
      code += hex[Math.floor(Math.random() * 16)];
    }
    return code;
  }

function handlePlayerLeavingRoom(socket) {
    for (const roomCode in rooms) {
        let leavingId = socket.id;
        if (rooms[roomCode].clients[socket.id] || rooms[roomCode].host === leavingId) {
                io.to(roomCode).emit('playerleft', leavingId);
                delete rooms[roomCode].clients[leavingId];
                if (rooms[roomCode].host === socket.id) {
                    if (rooms[roomCode].can_hostMigrate) {
                        const clients = Object.keys(rooms[roomCode].clients);
                        if (clients.length > 0) {
                            rooms[roomCode].host = clients[0];
                            delete rooms[roomCode].clients[rooms[roomCode].host];
                            io.to(roomCode).emit('newhost', clients[0]);
                            console.log(`New host of room ${roomCode} is ${clients[0]}`);
                        } else {
                            console.log(`Room ${roomCode} was deleted`);
                            delete rooms[roomCode];
                        }
                    } else {
                        io.to(roomCode).emit('roomerror', "The Host Left");
                        io.in(roomCode).disconnectSockets(true);
                        delete rooms[roomCode];
                    }
                }
                break;
            }
    }
}