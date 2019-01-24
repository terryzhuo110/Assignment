// Assignment 1 Mandelbrot
//
// Completed by
//  ... (z0000000)
//  ... (z0000000)
//
// Modified on 2017-??-??
// Tutor's name (dayHH-lab)

// Originally by Richard Buckland 28/01/11, 30/3/14.
// Licensed under Creative Commons SA-BY-NC 3.0, share freely.
//
// Extended by Curtis Millar 2017-08-29.

#include <err.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "mandelbrot.h"

#define SIMPLE_SERVER_VERSION 3.0
#define REQUEST_BUFFER_SIZE 1000
#define PORT_GENERATOR 199
#define PORT_MODULUS ((277 * PORT_GENERATOR) - 1)
#define PORT_BASE (1 << 12)
#define HOSTNAME_LEN 256

// after serving this many pages+images the server will halt
#define NUMBER_OF_PAGES_TO_SERVE 1024

// For writing BMP
#define PIXEL_START 26
#define PIXEL_BYTES 3
#define PIXEL_ALIGN 4
#define PIXEL_BITS  24
#define HEADER_SIZE 12

// Functions for setting up the server
int waitForConnection (int serverSocket);
int makeServerSocket (int portno);
void showRequest (int number, char request[REQUEST_BUFFER_SIZE]);
void routeRequest (int socket, char requestBuffer[REQUEST_BUFFER_SIZE]);
void serveIndex (int socket);
void sendHtmlHeader (int socket);
void copyPath (
	char requestBuffer[REQUEST_BUFFER_SIZE],
	char pathBuffer[REQUEST_BUFFER_SIZE]
);
void sendIndex (int socket);
uint16_t get_port(void);

// Write an image to output
void sendBitmapHeader (int socket);
void sendImage (int socket, struct pixel pixels[TILE_SIZE][TILE_SIZE]);

int main (int argc, char *argv[]) {

    signal(SIGPIPE, SIG_IGN);

	uint16_t port = get_port();
	char hostname[HOSTNAME_LEN] = "";
	gethostname(hostname, HOSTNAME_LEN);
	hostname[HOSTNAME_LEN - 1] = '\0';

    printf ("[SERVER] Starting mandelbrot server %.2f\n",
            SIMPLE_SERVER_VERSION);
    printf ("[SERVER] Serving fractals since 2011\n");
    printf ("[SERVER] Access this server at http://%s:%d/\n",
            hostname, port);
    printf ("[SERVER] Waiting for requests...\n");

    int serverSocket = makeServerSocket (port);
    char request[REQUEST_BUFFER_SIZE];
    int numberServed = 0;
    printf ("[SERVER] Served %d requests\n", numberServed);
    while (numberServed < NUMBER_OF_PAGES_TO_SERVE) {

        // STEP 1. wait for a request to be sent from a web browser,
        // then open a new connection for this conversation
        int connectionSocket = waitForConnection (serverSocket);

        // STEP 2. read the first line of the request
        long int bytesRead =
            recv (connectionSocket, request, sizeof (request) - 1, 0);
        // check that we were able to read some data from the connection
        if (bytesRead < 0) {
            errx (EXIT_FAILURE, "Unable to read request data");
        }

        // echo entire request to the console for debugging
        showRequest (numberServed, request);

        // STEP 3. send the browser a simple html page using http
        routeRequest (connectionSocket, request);
        printf ("\n[REQUEST:%3d] Response sent\n", numberServed);
        printf ("[REQUEST:%3d] Closing connection\n", numberServed);

        // STEP 4. close the connection after sending the page- keep aust
        // beautiful
        close (connectionSocket);
        ++numberServed;
        printf ("[SERVER] Served %d requests\n", numberServed);
    }

    // close the server connection after we are done- keep aust beautiful
    printf ("[SERVER] Shutting down the server\n");
    close (serverSocket);

    return EXIT_SUCCESS;
}

// Log a request that was received
void showRequest (int number, char request[REQUEST_BUFFER_SIZE]) {
    // Show request path.
    // printf ("[REQUEST:%3d] Received request:\n\n", number);
    // printf ("%s\n\n", request);
    // Log only initial line
    char basicRequestBuffer[REQUEST_BUFFER_SIZE] = {'\0'};
    copyPath(request, basicRequestBuffer);
    printf ("[REQUEST:%3d] Received request: %s\n",
            number, basicRequestBuffer);
}

// start the server listening on the specified port number
int makeServerSocket (int portNumber) {

    // create socket
    int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        errx (EXIT_FAILURE, "Could not create a server socket");
    }
    // check there was no error in opening the socket

    // bind the socket to the listening port  (1511 in this case)
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons (portNumber);

    // tell the server to restart immediately after a previous shutdown
    // even if it looks like the socket is still in use
    // otherwise we might have to wait a little while before rerunning the
    // server once it has stopped
    const int optionValue = 1;
    setsockopt (
		serverSocket, SOL_SOCKET,
		SO_REUSEADDR, &optionValue, sizeof (int));

    int bindSuccess = bind (
        serverSocket,
        (struct sockaddr *)&serverAddress, sizeof (serverAddress));

    if (bindSuccess < 0) {
        errx (EXIT_FAILURE, "Could not bind to the server port");
    }
    // if this assert fails wait a short while to let the operating
    // system clear the port before trying again

    return serverSocket;
}

// wait for a browser to request a connection,
// returns the socket on which the conversation will take place
int waitForConnection (int serverSocket) {

    // listen for a connection
    const int serverMaxBacklog = 10;
    listen (serverSocket, serverMaxBacklog);

    // accept the connection
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof (clientAddress);
    int connectionSocket = accept (
        serverSocket, (struct sockaddr *)&clientAddress, &clientLen);

    // check for connection error
    if (connectionSocket < 0) {
        errx (EXIT_FAILURE, "Unable to accept a connection");
    }

    return connectionSocket;
}

// Route requests from the browser to the relevant parts of the
// mandelbrot server.
// Takes in a string that contains the requested URL, and parses it to
// determine what response to send back to the browser.
void routeRequest (int socket, char requestBuffer[REQUEST_BUFFER_SIZE]) {
	// Image data to serve.
    struct pixel pixels[TILE_SIZE][TILE_SIZE] = {{{0, 0, 0}}};

	// Extract just the query path
	char pathBuffer[REQUEST_BUFFER_SIZE] = {'\0'};
	copyPath(requestBuffer, pathBuffer);

	if (strcmp(pathBuffer, "/") == 0) {
		// Send static HTML.
		sendIndex(socket);
	} else {
		// Render the tile.
		serveMandelbrot(pixels, pathBuffer);
	}

    // Send the image to the client
    sendImage (socket, pixels);
}

// Copy the request path into a buffer.
void copyPath (
	char requestBuffer[REQUEST_BUFFER_SIZE],
	char pathBuffer[REQUEST_BUFFER_SIZE]
) {
	char *pathStart = strchr(requestBuffer, ' ') + 1;
	char *pathEnd = strchr(pathStart, ' ');
	unsigned int pathLength = pathEnd - pathStart;
	strncpy(pathBuffer, pathStart, pathLength);
	pathBuffer[pathLength] = '\0';
}

// Sent the index HTML
void sendIndex (int socket) {
    const char *index =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n"
		"<script src=\"http://almondbread.cse.unsw.edu.au/tiles.js\"></script>";
    send (socket, index, strlen (index), 0);
}

// Send a HTTP headers for BMP
void sendBitmapHeader (int socket) {
    const char *message =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/bmp\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n";
    send (socket, message, strlen (message), 0);
}

// Writes the pixels as a BMP file using the specification from
// https://en.wikipedia.org/wiki/BMP_file_format
void sendImage (int socket, struct pixel pixels[TILE_SIZE][TILE_SIZE]) {
    sendBitmapHeader (socket);

    // Create a buffer to put the bitmap header data in
    char headerBuffer[PIXEL_START] = {'\0'};

    // Write headers
    // Initial BM bytes
    headerBuffer[0] = 'B';
    headerBuffer[1] = 'M';

    // File size
    unsigned int rowSize = TILE_SIZE * PIXEL_BYTES;
    unsigned int rowPadding = 0;
    if (rowSize % PIXEL_ALIGN != 0) {
        // Pad pixel to align properly
        rowPadding = PIXEL_ALIGN - (rowSize % PIXEL_ALIGN);
        rowSize += rowPadding;
    }
    unsigned int pixelData = (rowSize * TILE_SIZE);

    // We use memcpy so you don't have to
    unsigned int fileSize = PIXEL_START + pixelData;
    memcpy (&(headerBuffer[2]), &fileSize, sizeof (fileSize));

    // start of pixel data
    // pixels start immediately after header
    unsigned int pixelStart = PIXEL_START;
    memcpy (&(headerBuffer[10]), &pixelStart, sizeof (pixelStart));

    // Size of header
    unsigned int headerSize = HEADER_SIZE;
    memcpy (&(headerBuffer[14]), &headerSize, sizeof (headerSize));

    // Image width and height
    unsigned short size = TILE_SIZE;
    memcpy (&(headerBuffer[18]), &size, sizeof (size));
    memcpy (&(headerBuffer[20]), &size, sizeof (size));

    // Number of image planes (1)
    unsigned short planes = 1;
    memcpy (&(headerBuffer[22]), &planes, sizeof (planes));

    // Number of bits per pixel (24)
    unsigned short bitsPerPixel = PIXEL_BITS;
    memcpy (&(headerBuffer[24]), &bitsPerPixel, sizeof (bitsPerPixel));

    // Send the header
    send (socket, headerBuffer, PIXEL_START, MSG_MORE);

    // Write each of the pixels
    unsigned char *pixelBuffer = malloc (pixelData);
    if (pixelBuffer == NULL) {
        exit (EXIT_FAILURE);
    }

    int y = 0;
    while (y < TILE_SIZE) {
        int x = 0;
        while (x < TILE_SIZE) {
            // Write the blue, green, then red pixels
            struct pixel pixel = pixels[y][x];
            int byteIndex = (y * rowSize) + (x * PIXEL_BYTES);
            pixelBuffer[byteIndex + 0] = pixel.blue;
            pixelBuffer[byteIndex + 1] = pixel.green;
            pixelBuffer[byteIndex + 2] = pixel.red;
            x++;
        }
        y++;
    }

    // Send the header
    send (socket, pixelBuffer, pixelData, 0);
}

/// Get the port for a given user.
uint16_t get_port(void) {
	return PORT_BASE + ((PORT_GENERATOR * getuid()) % PORT_MODULUS);
}
