# ChatBox with Client-Server Architecture
 The chat application implemented in C using a client-server architecture. The application allows multiple clients to connect to a central server and communicate with each other in real-time.This project was developed as part of my 4th semester Operating Systems coursework.

## Features

- *Client-Server Communication:* Clients connect to a central server using TCP/IP sockets. The server listens for incoming connections and handles communication between clients.
- *Multi-Threaded Design:* The server utilizes multi-threading to handle multiple client connections simultaneously. Each client connection is managed by a separate thread, allowing for concurrent communication.
- *User Authentication:* Clients provide a username when connecting to the server. This username is used to identify clients in the chat room.
- *Message Broadcasting:* Messages sent by clients are broadcasted to all connected clients by the server. This enables real-time communication among all participants in the chat room.
- *Private Messaging:* Clients can send private messages to specific users by specifying the recipient's username in the message. The server facilitates the delivery of private messages between clients.
- *Online User List:* Clients can request a list of online users from the server. The server responds with a list of currently connected usernames, enabling clients to see who is currently active in the chat room.
- *Backup Chat History:* Messages exchanged between clients are logged to a text file on the client's side. This ensures that chat history is preserved even after disconnecting from the server.

## Video

https://github.com/Ahmad-Murtaza2/ChatBox-with-Client-Server-Architecture/assets/142945526/c3e5deaf-07e8-4685-a0ed-c1ff3bdd0b66
