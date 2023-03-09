#ifndef animatedgif_config
#define animatedgif_config

// control the maximum dimensions of gif decoding
#define gif_width 64
#define gif_height 64

/* GifDecoder needs lzwMaxBits
 * The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
 * lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
 * All 32x32-pixel GIFs tested work with 11, most work with 10
 */
const int lzwMaxBits = 11;

// enable debug mode over serial?
// #define DEBUGLINE 16

// These are just declarations for moving and/or resizing the GIFs. Set the
// values in main.cpp
extern int OFFSETX;
extern int OFFSETY;
extern int FACTX;
extern int FACTY;

// Use built in flash via SPIFFS
#include <SPIFFS.h>
#define FSO SPIFFS
// Do NOT add a trailing slash, or things will fail
#define GIF_DIRECTORY "/gifs"

#endif
