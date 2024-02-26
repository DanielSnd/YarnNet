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
  
  const htmlContent = `
    <html>
    <head>
      <style>
        body {
          background-color: #333; /* Dark Gray */
          color: #eee; /* Off-White / Very Light Gray */
        }
      </style>
    </head>
    <body><h1> ${io.engine.clientsCount} connected</h1></body></html>`;
    
  response.send(htmlContent);
});

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
        while (rooms[roomCode]) {
            roomCode = generateRoomCode() + protocol;
        }
        rooms[roomCode] = { host: socket.id };
        socket.join(roomCode);
        socket.emit('roomcreated', roomCode);
        console.log(`Room ${roomCode} has been created by ${socket.id}`);
    });

    socket.on('joinroom', (roomCode) => {
        if (rooms[roomCode]) {
            socket.join(roomCode);
            rooms[roomCode][socket.id] = socket.id;
            const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
            if (hostSocketId) {
                socket.emit('roomjoined', roomCode, rooms[roomCode].host);
            } else {
                console.log(`No valid host found for room code: ${roomCode}`);
                rooms[roomCode].host = socket.id;
                delete rooms[roomCode][rooms[roomCode].host];
                io.to(roomCode).emit('newhost', socket.id);
                socket.emit('roomjoined', roomCode, socket.id);
            }
            
            // Collect all socket IDs in the room except the newly joined one
            const socketIdsInRoom = Object.values(rooms[roomCode]).filter(id => id !== socket.id);
            // Emit the array of socket IDs to the player who just joined
            socket.emit('roomplayers', socketIdsInRoom);

            io.to(roomCode).except(socket.id).emit('playerjoin',socket.id);
            console.log(`User ${socket.id} has joined room ${roomCode} which has host ${rooms[roomCode].host}`);
        } else {
            socket.emit('roomerror', 'This room does not exist');
        }
    });

    socket.on('joinOrCreateRoom', (roomCode) => {    
        if (!roomCode || roomCode.trim() === "") {
            // Room code is null or empty, generate a new one
            roomCode = generateRoomCode();
        }
        if (rooms[roomCode]) {
            // Join the existing room
            socket.join(roomCode);
            rooms[roomCode][socket.id] = socket.id;
            const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
            if (hostSocketId) {
                socket.emit('roomjoined', roomCode, rooms[roomCode].host);
            } else {
                console.log(`No valid host found for room code: ${roomCode}`);
                rooms[roomCode].host = socket.id;
                delete rooms[roomCode][rooms[roomCode].host];
                io.to(roomCode).emit('newhost', socket.id);
                socket.emit('roomjoined', roomCode, socket.id);
            }
            // Collect all socket IDs in the room except the newly joined one
            const socketIdsInRoom = Object.values(rooms[roomCode]).filter(id => id !== socket.id);
            // Emit the array of socket IDs to the player who just joined
            socket.emit('roomplayers', socketIdsInRoom);
            io.to(roomCode).except(socket.id).emit('playerjoin',socket.id);
            console.log(`User ${socket.id} has joined room ${roomCode} which has host ${rooms[roomCode].host}`);
        } else {
            // Create and join a new room
            let newRoomCode = roomCode;
            while (rooms[newRoomCode]) {
                newRoomCode = generateRoomCode() + roomCode;
            }
            rooms[newRoomCode] = { host: socket.id };
            socket.join(newRoomCode);
            socket.emit('roomcreated', newRoomCode);
            console.log(`Room ${newRoomCode} has been created by ${socket.id}`);
        }
    });

    socket.on('message', (roomCode, message) => {
        socket.to(roomCode).emit('message', message);
    });

    socket.on('message', (message) => {
        console.log("received message:",message)
    });
    // socket.on('rpctoserver', (roomCode, ...rpcArgs) => {
    //     const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
    //     if (hostSocketId) {
    //         io.to(hostSocketId).emit('rpc', ...rpcArgs);
    //     }
    // });

    // socket.on('rpctoclients', (roomCode, ...rpcArgs) => {
    //     if (rooms[roomCode] && rooms[roomCode].host === socket.id) {
    //         socket.to(roomCode).emit('rpc', ...rpcArgs);
    //     }
    // });    
    
    
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
            const clients = Object.keys(rooms[roomCode]);
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

    
    socket.on('pkt2serv', (roomCode, packetcontent) => {
        //console.log(`Received 'pkt2serv' event for room code: ${roomCode} content ${packetcontent}`);
        const hostSocketId = rooms[roomCode] && rooms[roomCode].host;
        if (hostSocketId) {
            //console.log(`Found valid host with socket ID: ${hostSocketId}`);
            const clients = Object.keys(rooms[roomCode]);
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
            const clients = Object.keys(rooms[roomCode]);
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
    var hex = "ABCDEFRTPXZSQHJYWKN123456789";
    var code = "";
    for (var i = 0; i < 6; i++) {
      code += hex[Math.floor(Math.random() * 16)];
    }
    return code;
  }

function handlePlayerLeavingRoom(socket) {
    for (const roomCode in rooms) {
        let leavingId = socket.id;
        if (rooms[roomCode][socket.id] || rooms[roomCode].host === leavingId) {
                io.to(roomCode).emit('playerleft', leavingId);
                delete rooms[roomCode][leavingId];
                if (rooms[roomCode].host === socket.id) {
                    const clients = Object.keys(rooms[roomCode]).filter(id => id !== 'host');
                    if (clients.length > 0) {
                        rooms[roomCode].host = clients[0];
                        delete rooms[roomCode][rooms[roomCode].host];
                        io.to(roomCode).emit('newhost', clients[0]);
                        console.log(`New host of room ${roomCode} is ${clients[0]}`);
                    } else {
                        console.log(`Room ${roomCode} was deleted`);
                        delete rooms[roomCode];
                    }
                    break;
                }
            }
    }
}