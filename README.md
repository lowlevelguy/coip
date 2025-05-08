# CoIP: Calculator-over-IP

Calculator server-client architecture that takes an operation as input from the client side, and sends it to the server-side to evaluate. A middleman is used to manually corrupt the message in order to simulate noise on a real network.

## Building
With the gcc compiler pre-installed, simply run:
```
make
```

Followingly, three binaries `client`, `middleman` and `server` will be generated in the project root folder.

**Important:** On Windows systems, this project will not normally compile. However, if you have Cygwin or MSYS2 installed, it should then possible to link using their provided DLLs, though I'm unsure how that is.

## Usage

### Client: Error checking/correction left
```
Usage: ./client [server ip] [server port]
```

The client app will request the user to provide two operands and a single operation, wrap them in the following form: `[op1, op2, operation]`, and use an error detecting and/or correcting code on the message before transmitting it.

**Important:** In order to use the middleman trick, simply provide the middleman's ip and port instead of the server's.


### Middleman: Done
```
Usage: ./middleman [server ip] [server port]
```

The middleman will "intercept" (not really) the message sent by the client, perhaps (but not always) corrupt it, and retransmit it to the server followingly. It will then receive the response from the server, and retransmit it back to the client.

The middleman operates on the port 8000.

### Server: Error checking/correction left
```
Usage: ./server
```

The server will receive the message supposed to contain the operation, check for any errors, and evaluate it, before transmitting the result back to the client.

The server operates on the port 8080.
