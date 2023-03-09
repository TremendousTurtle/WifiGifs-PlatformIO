#ifndef neomatrix_config_h
#define neomatrix_config_h

bool init_done = 0;
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define SMARTMATRIX

#include <Adafruit_GFX.h>

//============================================================================
// Matrix defines (SMARTMATRIX vs NEOMATRIX and size)
//============================================================================

// CHANGEME, see MatrixHardware_ESP32_V0.h in SmartMatrix/src
#define GPIOPINOUT ESP32_FORUM_PINOUT
#pragma message "Compiling for SMARTMATRIX with NEOMATRIX API"

#include <FastLED.h>
#include <SmartMatrix3.h>
#include <SmartMatrix_GFX.h>

uint8_t matrix_brightness = 25;

#pragma message "Compiling for ESP32 with 32x64 16 scan panel"
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;
// width of EACH NEOPIXEL MATRIX (not total display)
const uint16_t MATRIX_TILE_WIDTH = 64;
const uint16_t MATRIX_TILE_HEIGHT = 32;  // height of each matrix

const uint8_t MATRIX_TILE_H = 1;  // number of matrices arranged horizontally
const uint8_t MATRIX_TILE_V = 2;  // number of matrices arranged vertically

const uint16_t mw = MATRIX_TILE_WIDTH * MATRIX_TILE_H;
const uint16_t mh = MATRIX_TILE_HEIGHT * MATRIX_TILE_V;
const uint32_t NUMMATRIX = mw * mh;

const uint32_t NUM_LEDS = NUMMATRIX;
const uint16_t MATRIX_HEIGHT = mh;
const uint16_t MATRIX_WIDTH = mw;

// SmartMatrix Defines
// known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH
// must be 24
#define COLOR_DEPTH 24

const uint8_t kMatrixWidth = mw;
const uint8_t kMatrixHeight = mh;
const uint8_t kRefreshDepth = 24;  // known working: 24, 36, 48
// known working: 2-4, use 2 to save memory, more to keep from dropping
// frames and automatically lowering refresh rate
const uint8_t kDmaBufferRows = 2;

// see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);

const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrixLayer, kMatrixWidth, kMatrixHeight,
                             kRefreshDepth, kDmaBufferRows, kPanelType,
                             kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth,
                                      kMatrixHeight, COLOR_DEPTH,
                                      kBackgroundLayerOptions);

CRGB *matrixleds;

void show_callback();
SmartMatrix_GFX *matrix =
    new SmartMatrix_GFX(matrixleds, mw, mh, show_callback);

// Sadly this callback function must be copied around with this init code
void show_callback() {
  backgroundLayer.swapBuffers(true);

  matrixleds = (CRGB *)backgroundLayer.backBuffer();
  matrix->newLedsPtr(matrixleds);
}
//============================================================================
// rotating "base color" used by many of the patterns
uint8_t gHue = 0;
uint16_t speed = 255;

// higher number is darker, needed for Neomatrix more than SmartMatrix
float matrix_gamma = 1;

// Like XY, but for a mirror image from the top (used by misconfigured code)
int XY2(int x, int y, bool wrap = false) {
  wrap = wrap;  // squelch compiler warning
  return matrix->XY(x, MATRIX_HEIGHT - 1 - y);
}

uint16_t XY(uint8_t x, uint8_t y) { return matrix->XY(x, y); }

int wrapX(int x) {
  if (x < 0) return 0;
  if (x >= MATRIX_WIDTH) return (MATRIX_WIDTH - 1);
  return x;
}

void show_free_mem() { Framebuffer_GFX::show_free_mem(); }

void matrix_setup(int reservemem = 40000) {
  reservemem = reservemem;  // squelch compiler warning if var is unused.
  if (init_done) {
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>> BUG: matrix_setup called twice");
    return;
  }
  init_done = 1;

  Serial.begin(115200);
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>> Serial.begin");

  matrix_gamma = 1;  // SmartMatrix should be good by default.
  matrixLayer.addLayer(&backgroundLayer);
  // SmartMatrix takes all the RAM it can get its hands on. Get it to leave some
  // free RAM so that other libraries can work too.
  if (reservemem) {
    matrixLayer.begin(reservemem);
  } else {
    matrixLayer.begin();
  }
  // This sets the neomatrix and LEDMatrix pointers
  show_callback();
  matrixLayer.setRefreshRate(240);
  backgroundLayer.enableColorCorrection(true);
  Serial.print("SmartMatrix GFX output, total LEDs: ");
  Serial.println(NUMMATRIX);
  delay(1000);
  // Quick hello world test
#ifndef DISABLE_MATRIX_TEST
  Serial.println("SmartMatrix Grey Demo");
  backgroundLayer.fillScreen({0x80, 0x80, 0x80});
  show_callback();
  delay(1000);
#endif
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>> SmartMatrix Init Done");

  show_free_mem();
  matrix->begin();

  Serial.print("Setting Brightness: ");
  Serial.println(matrix_brightness);

  matrixLayer.setBrightness(matrix_brightness);
  Serial.print("Gamma Correction: ");
  Serial.println(matrix_gamma);
  // Gamma is used by AnimatedGIFs and others, as such:
  // CRGB color = CRGB(matrix->gamma[red], matrix->gamma[green],
  // matrix->gamma[blue]);
  matrix->precal_gamma(matrix_gamma);

  Serial.println("neomatrix_config setup done");
}

#endif  // neomatrix_config_h