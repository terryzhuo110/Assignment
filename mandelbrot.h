// Mandelbrot generation functions
//
// You can compile the full server with the command
// dcc -o mandelbrotServer server.c mandelbrot.c pixelColor.c
//
// Don't change this file

#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

// Size of each image
//
// Each tile should be TILE_SIZE pixels wide and TILE_SIZE pixels tall.
//
// Make sure this value is unchanged when you submit.
#define TILE_SIZE 512

// Escape Steps
//
// The maximum number of steps that we loop to test if a complex number
// is in the mandelbrot set.
//
// Any number that is assumed to be in the mandelbrot set has NO_ESCAPE
// number of steps.
//
// Make sure this value is unchanged when you submit.
#define MAX_STEPS 256
#define NO_ESCAPE -1

// Structs used by the program

// BMP is made of pixels
struct pixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// Complex Numbers
// Complex number is (re + im * i)
struct complex {
    double re;
    double im;
};

// Functions you need to implement:
void serveMandelbrot(
	struct pixel pixels[TILE_SIZE][TILE_SIZE],
	char *requestPath
);
void drawMandelbrot(
	struct pixel pixels[TILE_SIZE][TILE_SIZE],
    struct complex center,
	int z
);
int escapeSteps(struct complex c);

#endif // !defined(_MANDELBROT_H_)
