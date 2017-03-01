# Luis Serota (LS843) and Osasu Eboh (OE37)

# project-1 HTTP/1.1 server

This repository provides a C implementation for a simple HTTP/1.1 server that accepts
HEAD, GET, and POST requests.

To run the server, from the root directory:
1 - `make`
2 - `./lisod`

The server is now running on localhost port 9034!

To make a HEAD request to the server, run:
`curl -I localhost:9034/test.txt`

To make a HEAD request to the server, run:
`curl -i localhost:9034/test/txt`

All requests made to the server are routed to the `www/` directory found at the
root of this repository. For example, if a user made a GET request to localhost:9034/test.txt,
the server considers the `www/` directory as the root and would check for the file there.

Note: for reasons unclear to us, if the client makes more than 4 requests over the same socket
without closing the socket, the server will crash.
