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

### Client
```
Usage: ./client [middleman ip] [middleman port]
```

The client app will request the user to provide two operands and a single operation. It will then wrap them in the following form: `[op1, op2, operation]`, and encode them using the Hamming(7,4) encoding, before transmitting it to the server.

This repeats inside a while loop, until the user requests it to stop. The client then transmits an (encoded) `exit` message.


### Middleman
```
Usage: ./middleman [server ip] [server port]
```

The middleman simulates an interception of the message sent by the client. Its role is to then perhaps (but not always) corrupt it (by flipping one bit at random), and retransmit it to the server followingly. It will then receive the response from the server, and retransmit it back to the client. The middleman does not decode the message at any stage.

Just as the client, this all repeats inside a while loop. The middleman compares the encoded message with the hard-coded encoding of the keyword `exit`, so that it stops when the client disconnects.

The middleman operates on the port 2223.

### Server
```
Usage: ./server
```

The server will receive the message supposed to contain the operation, check for any errors, correct them, then decode and evaluate the operation, before transmitting the result back to the client (really, to the middleman, who retransmits it).

As always, this happens inside a while loop, until an `exit` message is received.

The server operates on the port 2222.
