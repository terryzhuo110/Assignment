// Assignment 1 Mandelbrot
//
// Completed by
//  Terry (z5191493)
//
// Modified on 2018-08-19
// Helena

#include <stdio.h>

// Add your own #includes here

#include "mandelbrot.h"
#include "pixelColor.h"

// Add your own #defines here

#define TILES_SIZE 512
#define modulusSquare 4
#define FALSE 0


// Add your own function prototypes here


// Take a pixel grid and and the path from the web browser used to
// request the 'tile'.
//
// The requestPath will be in the form "/mandelbrot/2/{zoom}/{x}/{y}.bmp".
// In the query, 'x' and 'y' are decimals and describe the 'center' of the
// tile and need to be converted into a complex number.
// 'zoom' is an integer representing the zoom level for the tile.
void serveMandelbrot(
	struct pixel pixels[TILE_SIZE][TILE_SIZE],
	char *requestPath
) {
	struct complex center = {0.0, 0.0};
	int z = 7;

	sscanf(
		requestPath,
		"/mandelbrot/2/%d/%lf/%lf.bmp",
		&z, &center.re, &center.im
	);

	// Use drawMandelbrot to draw the immage.
	drawMandelbrot(pixels, center, z);
}

// Draw a single Mandelbrot tile, by calculating and colouring each of
// the pixels in the tile.
//
// Your function "returns" this tile by filling in the `pixels` 2D array
// that is passed in to your function.
//
// The `pixels` array is TILE_SIZE high and TILE_SIZE wide.
// The bottom left corner of the image is pixels[0][0].
//
// `center` is a complex number, representing the center of the tile.
// `z` is used to calculate the distance between each pixel in the image.
// Each pixel is 2^(-z) apart from each other.
void drawMandelbrot(
	struct pixel pixels[TILE_SIZE][TILE_SIZE],
    struct complex center,
	int z
) {
    int i = 0;
  	int j = 0;
	int steps = 0;
 	struct complex c;
//Calculate the distance of each pixel
//distance = 2^(-z)
//We are encouraged to use our own loop to calculate the distance
//instead of calling the "pow()" function in the <math.h>
	double distance = 0.5;
	int count = 1;
	while(count < z){
		distance = distance*0.5;
		count++;
	}
//min is the left bottom pixel point
//min_im is the y coordinate
//min_re is the x coordinate
	double min_im = center.im - (TILE_SIZE - 1) / 2.0 * distance;
	double min_re = center.re - (TILE_SIZE - 1) / 2.0 * distance;

	c.im = min_im;
	c.re = min_re;

//Use the while loop to get information of each pixel point
//Call escape() function to tell whether the point is in the set or not
//Fill the pixel with different color by different steps value
  	while (j < TILE_SIZE) {
    	while (i < TILE_SIZE) {
			steps = escapeSteps( c );
			pixels[j][i] = pixelColor( steps );
 			i++;
        	c.re += distance;
    	}
    	i = 0;
    	c.re = min_re;
    	j++;
    	c.im += distance;
  	}
}



// Determine the number of steps required to escape the Mandelbrot set,
// for the given complex number `c`.
int escapeSteps(struct complex c) {
    int steps = 0;
	struct complex w;
	w.re = 0;
	w.im = 0;
	double tem;
//To determine if a complex number z is in the Mandelbrot set 
//we can use the following loop:
//Let z be the complex number to test.
//Let w be the complex number 0 + 0i
//While |w| < 2, change w equal to w*w + z
//If the loop ever exits, then the complex number 
//z is not in the Mandelbrot Set.

	while(steps < MAX_STEPS && w.re * w.re + w.im * w.im < modulusSquare){
		tem = w.re;
		w.re = w.re * w.re - w.im * w.im + c.re;
		w.im = 2 * tem * w.im + c.im;
		steps = steps + 1;
	}

	if( steps == MAX_STEPS ) {
		return NO_ESCAPE;
	} else {
		return steps;
	}
}

// Add your own functions here.
// Remember to make them static.
