# ft_irc

## Project Description

ft_irc is a project developed as part of the 42Porto curriculum. The goal of this project is to implement an IRC (Internet Relay Chat) server in C++ that adheres to the IRC protocol. The server allows multiple clients to connect, join channels, send messages, and perform other IRC-related commands.

## Features

- Support for multiple clients
- Channel creation and management
- User authentication
- Private messaging
- Channel modes and user modes
- Bot integration

## Setup Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/MicchelFAB/ft_irc.git
   cd ft_irc
   ```

2. Build the project:
   ```bash
   make
   ```

3. Run the server:
   ```bash
   ./ircserv <port> <password>
   ```

## Usage Instructions

To connect to the IRC server, you can use any IRC client. Below are the basic commands to connect and authenticate with the server:

1. Connect to the server:
   ```bash
   /server <hostname> <port>
   ```

2. Set the password:
   ```bash
   /pass <password>
   ```

3. Set your nickname:
   ```bash
   /nick <nickname>
   ```

4. Set your username:
   ```bash
   /user <username> <hostname> <servername> <realname>
   ```

5. Authenticate with the server:
   ```bash
   /auth
   ```

6. Join a channel:
   ```bash
   /join <#channel>
   ```

7. Send a private message:
   ```bash
   /msg <nickname> <message>
   ```

8. Set a channel mode:
   ```bash
   /mode <#channel> <+mode> [parameters]
   ```

9. Invite a user to a channel:
   ```bash
   /invite <nickname> <#channel>
   ```

10. Kick a user from a channel:
    ```bash
    /kick <#channel> <nickname>
    ```

11. Set a channel topic:
    ```bash
    /topic <#channel> <topic>
    ```

## Testing

We recommend using the following testers to validate the functionality of the IRC server:

- [ft_irc-tester](https://github.com/opsec-infosec/ft_irc-tester)
- [IRC-tester](https://github.com/bennamrouche/irc-tester)

These testers provide automated tests to ensure that the server behaves correctly according to the IRC protocol.

## Custom Commands

The server includes a custom __AUTH__ command that is necessary for client authentication. After the client sends the PASS, NICK, and USER commands, the client must call the AUTH command for the server to authenticate the client.

## Notes

- The server follows the IRC protocol, and the flow for the client to connect to the server is the same as the IRC protocol (PASS, NICK, USER, etc.). After these commands, the client must call the AUTH command, and then the server will authenticate the client.
- The server uses epoll for efficient I/O event handling. Each accept, recv, and send call is made only after an epoll_wait call to ensure non-blocking I/O operations.

## Contributors

[MicchelFAB](https://github.com/MicchelFAB)  
[BelmiroRodrigues](https://github.com/BelmiroRodrigues)  
[Zahhask45](https://github.com/Zahhask45)
