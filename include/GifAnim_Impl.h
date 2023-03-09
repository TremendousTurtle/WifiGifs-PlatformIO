// when including in other code, allow disabling all those inits
// to avoid double inits.
#ifndef GIFANIM_INCLUDE
#include "animatedgif_config.h"
#endif

// Use NeoMatrix backend? Defined in main ino that calls sav_loop
#include "GifDecoder.h"
#include "neomatrix_config.h"

extern File file;
#include "FilenameFunctions.h"

void die(const char *mesg) {
  Serial.println(mesg);
  delay(100000);  // while 1 loop only triggers watchdog on ESP chips
}

/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 * defined in animatedgif_config.h
 */
GifDecoder<gif_width, gif_height, lzwMaxBits> decoder;

void screenClearCallback(void) { matrix->clear(); }

void updateScreenCallback(void) { matrix->show(); }

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green,
                       uint8_t blue) {
  red = matrix->gamma[red];
  green = matrix->gamma[green];
  blue = matrix->gamma[blue];
#if DEBUGLINE
  if (y == DEBUGLINE) {
    Serial.print(x);
    Serial.print(",");
    Serial.print(y);
    Serial.print(">");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.print(blue);
    Serial.println("");
  }
  if (y > DEBUGLINE) return;
#endif
  CRGB color = CRGB(red, green, blue);
  if (FACTX == 15 && FACTY == 15) {
    matrix->drawPixel(x * 1.5 + 0.5 + OFFSETX, y * 1.5 + 0.5 + OFFSETY, color);
    if (x % 2 == 0)
      matrix->drawPixel(x * 1.5 + 1.5 + OFFSETX, y * 1.5 + 0.5 + OFFSETY,
                        color);
    if (y % 2 == 0)
      matrix->drawPixel(x * 1.5 + 0.5 + OFFSETX, y * 1.5 + 1.5 + OFFSETY,
                        color);
    if (x % 2 == 0 && y % 2 == 0)
      matrix->drawPixel(x * 1.5 + 1.5 + OFFSETX, y * 1.5 + 1.5 + OFFSETY,
                        color);
  } else if (FACTY == 15) {
    matrix->drawPixel(x + OFFSETX, y * 1.5 + 0.5 + OFFSETY, color);
    if (y % 2 == 0)
      matrix->drawPixel(x + OFFSETX, y * 1.5 + 1.5 + OFFSETY, color);
  } else if (FACTX == 15) {
    matrix->drawPixel(x * 1.5 + 0.5 + OFFSETX, y + OFFSETY, color);
    if (x % 2 == 0)
      matrix->drawPixel(x * 1.5 + 1.5 + OFFSETX, y + OFFSETY, color);
  } else {
    matrix->drawPixel(x + OFFSETX, y + OFFSETY, color);
  }
}

// Setup method runs once, when the sketch starts
void sav_setup() {
  decoder.setScreenClearCallback(screenClearCallback);
  decoder.setUpdateScreenCallback(updateScreenCallback);
  decoder.setDrawPixelCallback(drawPixelCallback);

  decoder.setFileSeekCallback(fileSeekCallback);
  decoder.setFilePositionCallback(filePositionCallback);
  decoder.setFileReadCallback(fileReadCallback);
  decoder.setFileReadBlockCallback(fileReadBlockCallback);

// when including in other code, allow disabling all those inits
// to avoid double inits.
#ifndef GIFANIM_INCLUDE
  matrix_setup();
#endif  // GIFANIM_INCLUDE

  // SPIFFS Begin (can crash/conflict with IRRemote on ESP32)
  File dir = FSO.open("/");
  while (File file = dir.openNextFile()) {
    Serial.print("FS File: ");
    Serial.print(file.name());
    Serial.print(" Size: ");
    Serial.println(file.size());
  }
  Serial.println();
}

bool sav_newgif(const char *pathname) {
  if (file) file.close();
  Serial.print(pathname);
  file = FSO.open(pathname);
  if (!file) {
    Serial.println(": Error opening GIF file");
    return 1;
  }
  Serial.println(": Opened GIF file, start decoding");
  decoder.startDecoding();
  return 0;
}

bool sav_loop() {
  // ERROR_WAITING means it wasn't time to display the next frame and the
  // display did not get updated (this is important for a neopixel matrix where
  // the display being updated causes a pause in the code).
  if (decoder.decodeFrame() == ERROR_WAITING) return 1;
  return 0;
}

// vim:sts=4:sw=4
