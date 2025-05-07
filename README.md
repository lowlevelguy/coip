# CoIP: Calculator-over-IP

Calculator server-client architecture that takes an operation as input from the client side, and sends it to the server-side to evaluate. A middleman is used to manually corrupt the message in order to simulate noise on a real network.

## Client: in progress
```
Usage: ./client [server ip] [server port]
```

The client app will request the user to provide two operands and a single operation, wrap them in the following form: `[op1, op2, operation]`, and use an error detecting and/or correcting code on the message before transmitting it.

In order to use the middleman trick, simply provide the middleman's ip and port instead of the server's.

The client operates on the port 2222.

## Middleman: work has not begun
```
Usage: ./middleman [server ip] [server port]
```

The middleman will "intercept" (not really) the message sent by the client, perhaps (but not always) corrupt it, and retransmit it to the server followingly. It will then receive the response from the server, and retransmit it back to the client.

The middleman operates on the port 3333.

## Server: work has not begun
```
Usage: ./server
```

The server will receive the message supposed to contain the operation, check for any errors, and evaluate it, before transmitting the result back to the client.

The server operates on the port 4444.
