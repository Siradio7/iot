// Pont entre le port série de l'ESP32 et Node-RED (dans Docker).
//
// Sur macOS, un conteneur Docker n'a pas accès aux ports USB. Ce script tourne
// donc sur le Mac : il lit les lignes JSON envoyées par l'ESP32 et les transmet
// à Node-RED via un serveur TCP. Ce que Node-RED envoie est renvoyé à l'ESP32.
//
//   ESP32 ──USB──▶ bridge.js (Mac) ◀──TCP──▶ Node-RED (Docker)
require('dotenv').config();

const net = require('net');
const { SerialPort, ReadlineParser } = require('serialport');

const SERIAL_PORT = process.env.SERIAL_PORT || '/dev/cu.usbserial-0001';
const BAUD_RATE = Number(process.env.BAUD_RATE) || 9600;
const TCP_PORT = Number(process.env.TCP_PORT) || 5001;
const RECONNECT_DELAY = 2000;

const clients = new Set();
let port = null;

// --- Port série (ESP32) ---------------------------------------------------

function openSerial() {
    port = new SerialPort({ path: SERIAL_PORT, baudRate: BAUD_RATE, autoOpen: false });

    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

    port.on('open', () => {
        console.log(`ESP32 connecté sur ${SERIAL_PORT} (${BAUD_RATE} bauds)`);
    });

    port.on('close', () => {
        console.warn('ESP32 déconnecté, nouvelle tentative...');
        setTimeout(openSerial, RECONNECT_DELAY);
    });

    port.on('error', (error) => {
        console.error('Erreur série :', error.message);
    });

    parser.on('data', (line) => {
        line = line.trim();

        if (line === '') {
            return;
        }

        console.log('ESP32 →', line);

        for (const client of clients) {
            client.write(line + '\n');
        }
    });

    port.open((error) => {
        if (error) {
            console.error(`Ouverture de ${SERIAL_PORT} impossible : ${error.message}`);
            setTimeout(openSerial, RECONNECT_DELAY);
        }
    });
}

// --- Serveur TCP (Node-RED) -----------------------------------------------

const server = net.createServer((socket) => {
    const name = `${socket.remoteAddress}:${socket.remotePort}`;

    clients.add(socket);
    console.log(`Node-RED connecté (${name})`);

    socket.on('data', (data) => {
        console.log('Node-RED →', data.toString().trim());

        if (port && port.isOpen) {
            port.write(data);
        }
    });

    socket.on('close', () => {
        clients.delete(socket);
        console.log(`Node-RED déconnecté (${name})`);
    });

    socket.on('error', (error) => {
        console.error(`Erreur TCP (${name}) :`, error.message);
    });
});

server.listen(TCP_PORT, () => {
    console.log(`Serveur TCP en écoute sur le port ${TCP_PORT}`);
});

openSerial();
