/*
 * This code displays .gif files from the /gifs directory on SPIFFS,
 * controlled via web interface through a Wifi network broadcast by
 * the ESP32.
 */
#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <WiFi.h>

#include "ESPAsyncWebServer.h"

#define DISABLE_MATRIX_TEST
#define NEOMATRIX
#include "GifAnim_Impl.h"

#define defaultBrightness 25
#define minBrightness 0
#define maxBrightness 210

int currentBrightness = defaultBrightness;

DNSServer dnsServer;
AsyncWebServer server(80);

#define FIRSTINDEX 0  // index of GIF to display on startup/first

bool nextFlag = false;
bool prevFlag = false;
int newIndex = -1;
int currentIndex = FIRSTINDEX;

// offset GIF position from the upper left corner of the matrix
// both positive and negative values will work
int OFFSETX = 0;
int OFFSETY = 0;

// resize GIF? Only does anything when set to 15 in which case it
// draws slightly shifted extra pixels. Needs experimentation
int FACTX = 0;
int FACTY = 0;

int num_files;
String filenameOptions = "";

String processor(const String& var) {
  if (var == "MIN_BRIGHTNESS") return String(minBrightness);
  if (var == "MAX_BRIGHTNESS") return String(maxBrightness);
  if (var == "CURRENT_BRIGHTNESS") return String(currentBrightness);
  if (var == "CURRENT_INDEX") return String(currentIndex);
  if (var == "LIST_FILENAME_OPTIONS") return filenameOptions;
  return String();
}

class CaptiveRequestHandler : public AsyncWebHandler {
 public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest* request) {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest* request) {
    // handle other file requests
    if (request->url() == "/styles.css") {
      request->send(SPIFFS, "/www/styles.css", "text/css");
    } else {  // handle parameters
      if (request->hasParam("next")) {
        Serial.println("NEXT pressed");
        nextFlag = true;
      }
      if (request->hasParam("prev")) {
        Serial.println("PREVIOUS pressed");
        prevFlag = true;
      }
      if (request->hasParam("brightness")) {
        AsyncWebParameter* p = request->getParam("brightness");
        currentBrightness = p->value().toInt();
        Serial.print("New brightness: ");
        Serial.println(currentBrightness);
      }
      if (request->hasParam("newFileIndex")) {
        AsyncWebParameter* p = request->getParam("newFileIndex");
        newIndex = p->value().toInt();
        Serial.print("New file index: ");
        Serial.println(newIndex);
      }

      // catch everything else
      // Send index.htm with template processor function
      request->send(SPIFFS, "/www/index.htm", "text/html", false, processor);
    }
  }
};

// Setup method runs once, when the sketch starts
void setup() {
  Serial.println("Starting AnimatedGIFs Sketch");
  sav_setup();

#if ENABLE_SCROLLING == 1
  matrix.addLayer(&scrollingLayer);
#endif

  // for ESP32 we need to allocate SmartMatrix DMA buffers after initializing
  // the SD card to avoid using up too much memory
  // Determine how many animated GIF files exist
  num_files = enumerateGIFFiles(GIF_DIRECTORY, true);

  if (num_files < 0) {
#if ENABLE_SCROLLING == 1
    scrollingLayer.start("No gifs directory", -1);
#endif
    die("No gifs directory");
  }

  if (!num_files) {
#if ENABLE_SCROLLING == 1
    scrollingLayer.start("Empty gifs directory", -1);
#endif
    die("Empty gifs directory");
  }
  Serial.print("Index of files: 0 to ");
  Serial.println(num_files);
  Serial.flush();

  // At least on teensy, due to some framework bug it seems, early
  // serial output gets looped back into serial input
  // Hence, flush input.
  while (Serial.available() > 0) {
    char t = Serial.read();
    t = t;
  }

#if ENABLE_SCROLLING == 1
  scrollingLayer.start("Hello, world!", -1);
#endif

  Serial.println("Configuring access point...");
  WiFi.softAP("myLEDPanel");
  // Replace line above with line below if you want a wifi password
  // WiFi.softAP("myLEDPanel", "myPassword");

  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler());
  // more handlers...
  server.begin();
  Serial.print("Server started. IP Address: ");
  // IP Address is usually 192.168.4.1
  Serial.println(WiFi.softAPIP());
}

void adjust_gamma(float change) {
  matrix_gamma += change;
  matrix->precal_gamma(matrix_gamma);
  Serial.print("Change gamma to: ");
  Serial.println(matrix_gamma);
}

void loop() {
  dnsServer.processNextRequest();

  matrixLayer.setBrightness(currentBrightness);

  static int index = FIRSTINDEX;
  currentIndex = index;
  static int8_t new_file = 1;
  static uint16_t frame = 0;
  char readchar;
  // frame by frame display
  static bool debugframe = false;
  bool gotnf = false;
  // clear display before each frame
  static bool clear = false;

  if (nextFlag) {
    new_file = 1;
    index++;
    nextFlag = false;
  }
  if (prevFlag) {
    new_file = 1;
    index--;
    prevFlag = false;
  }
  if (newIndex != -1) {
    new_file = 1;
    index = newIndex;
    newIndex = -1;
  }

  if (Serial.available())
    readchar = Serial.read();
  else
    readchar = 0;

  switch (readchar) {
    case 'n':
      Serial.println("Serial => next");
      new_file = 1;
      index++;
      break;

    case 'p':
      Serial.println("Serial => previous");
      new_file = 1;
      index--;
      break;

    case 'f':
      Serial.println("Serial => debug frames, press 'g' for next frame");
      debugframe = true;
      break;

    case 'g':
      Serial.println("Serial => next frame");
      gotnf = true;
      break;

    case 'c':
      Serial.print("Toggle clear screen: ");
      clear = !clear;
      ;
      Serial.println(clear);
      break;

    case '+':
      adjust_gamma(+0.2);
      break;

    case '-':
      adjust_gamma(-0.2);
      break;

    default:
      // BUG: this does not work for index '0', just type '1', and 'p'
      if (readchar) {
        while ((readchar >= '0') && (readchar <= '9')) {
          new_file = 10 * new_file + (readchar - '0');
          readchar = 0;
          if (Serial.available()) readchar = Serial.read();
        }

        if (new_file) {
          Serial.print("Got new file via serial ");
          Serial.println(new_file);
          index = new_file;
        } else {
          Serial.print("Got serial char ");
          Serial.println(readchar);
        }
      }
  }

  if (debugframe) {
    if (!gotnf) return;
  }

  if (new_file) {
    frame = 0;
    new_file = 0;
    if (index >= num_files) index = 0;
    if (index <= -1) index = num_files - 1;
    Serial.print("Fetching file index #");
    Serial.println(index);

    if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0) {
      // Can clear screen for new animation here, but this might cause flicker
      // with short animations matrix->clear();
      decoder.startDecoding();
    } else {
      die("FATAL: failed to open file");
    }
  }

  if (clear) screenClearCallback();
  decoder.decodeFrame();
  frame++;
  if (debugframe) {
    Serial.print("Displayed frame #");
    Serial.print(frame);
    Serial.println(". Press g for next frame");
  }
#if DEBUGLINE
  delay(1000000);
#endif
}