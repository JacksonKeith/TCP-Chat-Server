chatserver.c
--Creates a server for a simple chatroom that uses TCP for communication between clients. Uses sockets and threads.

chatclient.c
--Creates a client for a simple chatroom that uses TCP for communication with the server. Uses sockets and threads.

server command:
./chatserver [port]
ex: ./chatserver 5001

client command:
./chatclient [hostname] [port] [username] [passcode]
ex: ./chatclient 127.0.0.1 5001 Jax secretcode

Created on Windows machine using WSL Ubuntu 18.04 via VSCode.