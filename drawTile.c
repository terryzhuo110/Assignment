/// Simple tool to help draw mandelbrot tiles from the terminal.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mandelbrot.h"

// For writing BMP
#define PIXEL_START 26
#define PIXEL_BYTES 3
#define PIXEL_ALIGN 4
#define PIXEL_BITS  24
#define HEADER_SIZE 12

#define FILE_NAME_LENGTH 256

void writeImage(FILE *file, struct pixel pixels[TILE_SIZE][TILE_SIZE]);

int main(int argc, char *argv[]) {
	int zoom;
	struct complex center;
	char filename[FILE_NAME_LENGTH] = "";

	if (argc == 4) {
		// You can also use
		// ./drawTile x_pos y_pos zoom
		center.re = atof(argv[1]);
		center.im = atof(argv[2]);
		zoom = atoi(argv[3]);
	} else {
		// Get co-ordinates
		printf("Enter x co-ordinate: ");
		scanf("%lf", &center.re);
		printf("Enter y co-ordinate: ");
		scanf("%lf", &center.im);
		printf("Enter zoom level: ");
		scanf("%d", &zoom);
	}

	snprintf(filename, FILE_NAME_LENGTH, "tile.%lf_%lf_%d.bmp",
		center.re, center.im, zoom);

	// Render tile
	struct pixel pixels[TILE_SIZE][TILE_SIZE];
	drawMandelbrot(pixels, center, zoom);

	// Save the tile to a file.
	FILE *tile_file = fopen(filename, "w");
	if (tile_file == NULL) {
		printf("Could not create tile file '%s'.\n", filename);
		return EXIT_FAILURE;
	}

	writeImage(tile_file, pixels);
	fclose(tile_file);

	printf("Wrote tile to '%s'.\n", filename);

	return EXIT_SUCCESS;
}

// Writes the pixels as a BMP file using the specification from
// https://en.wikipedia.org/wiki/BMP_file_format
void writeImage(FILE *file, struct pixel pixels[TILE_SIZE][TILE_SIZE]) {

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
    memcpy(&(headerBuffer[2]), &fileSize, sizeof(fileSize));

    // start of pixel data
    // pixels start immediately after header
    unsigned int pixelStart = PIXEL_START;
    memcpy(&(headerBuffer[10]), &pixelStart, sizeof(pixelStart));

    // Size of header
    unsigned int headerSize = HEADER_SIZE;
    memcpy(&(headerBuffer[14]), &headerSize, sizeof(headerSize));

    // Image width and height
    unsigned short size = TILE_SIZE;
    memcpy(&(headerBuffer[18]), &size, sizeof(size));
    memcpy(&(headerBuffer[20]), &size, sizeof(size));

    // Number of image planes (1)
    unsigned short planes = 1;
    memcpy(&(headerBuffer[22]), &planes, sizeof(planes));

    // Number of bits per pixel (24)
    unsigned short bitsPerPixel = PIXEL_BITS;
    memcpy(&(headerBuffer[24]), &bitsPerPixel, sizeof(bitsPerPixel));

    // Send the header
    fwrite(headerBuffer, sizeof(char), PIXEL_START, file);

    // Write each of the pixels
    unsigned char *pixelBuffer = malloc (pixelData);
    if (pixelBuffer == NULL) {
		exit(EXIT_FAILURE);
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
    fwrite(pixelBuffer, sizeof(char), pixelData, file);
}
