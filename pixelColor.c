// Assignment 1 Mandelbrot: Pixel Color
//
// Completed by
//  Terry (z5191493)
//
// Modified on 2018-08-19
// Tutor's name:Helena 

#include "mandelbrot.h"
#include "pixelColor.h"

// This function should take a number (of steps taken to escape the
// Mandelbrot set), and convert it to a color.
//
// You have artistic licence here, so make your Mandelbrot look amazing.
struct pixel pixelColor(int steps) {
    struct pixel color;
    color.red = steps;
    color.green = (int)steps*0.65;
    color.blue = 0;
	if( steps == NO_ESCAPE ){
		color.red = 0;
		color.green = 0;
		color.blue = 0;
	}
    return color;
}
