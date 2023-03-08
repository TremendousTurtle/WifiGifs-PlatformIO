# Display GIFs on HUB75 LED Panels controlled over Wifi
## Information 
----------
Actual code is from [Project Mc2 LED Purses WifiControlledGIFs example](https://github.com/rorosaurus/project-mc2-led-purse/tree/master/WifiControlledGIFs) by rorosaurus. 
Very light modifications have been made for panel configuration and to resolve a few complier warnings. The code has also been adapted to use Platform.IO rather than Arduino IDE.

## Setup
----------
1. Install [CP210X driver](https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers.zip) for serial communication with the ESP32 over USB.
1. Install [Visual Studio Code](https://code.visualstudio.com/).
1. Install [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) for Visual Studio Code.
1. Clone this repository to a location of your choice on your computer.
   - `git clone https://github.com/TremendousTurtle/WifiGifs-PlatformIO.git`
   - You may need to download and install [Git](https://git-scm.com/download/win).
   - You can also add the repository to VSCode from GitHub or through GitHub Desktop.
   - You may also download and extract the respository ZIP archive instead but cloning will allow you to easily receive any changes pushed to the code.
1. Open the respository folder in VSCode. PlatformIO should recognize and load the project.

PlatformIO should automatically download and install the appropriate versions of the development platform, board configuration, and libraries needed to compile and upload the code.

## Configuration
----------
### Wifi
----
SSID and passphrase can be configured in [main.cpp](src/main.cpp):
```cpp
// SSID "myLEDPanel" without a passphrase
WiFi.softAP("myLEDPanel");

// OR if you want a password: SSID "myLEDPanel" with passphrase "myPassword"
WiFi.softAP("myLEDPanel", "myPassword");
```

### LED Panel
----
LED Panel congifuration is in [neomatrix_config.h](include/neomatrix_config.h):
```cpp
uint8_t matrix_brightness = 25; // Default LED panel brightness

// LED Panel scan type - configured for 32 row (height) 1/16 scan panel by default
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;

// Enter the width/height of each INDIVIDUAL LED panel (NOT the total display)
const uint16_t MATRIX_TILE_WIDTH = 64;
const uint16_t MATRIX_TILE_HEIGHT= 32;

// Number of LED panels arranged horizontally/vertically
const uint8_t MATRIX_TILE_H     = 1;  // horizontally
const uint8_t MATRIX_TILE_V     = 2;  // vertically

// Color depth options: 24 or 48 should work. If program uses type `rgb24` directly, COLOR_DEPTH must be 24
#define COLOR_DEPTH 24

const uint8_t kRefreshDepth = 24; // known working: 24, 36, 48

// Number of LED panel rows to buffer in memory
// use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kDmaBufferRows = 2; // known working: 2-4

// SmartMatrix options - see http://docs.pixelmatix.com/SmartMatrix for options
// use multiple options with bitwise OR operator like: "const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_C_SHAPE_STACKING | SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING);"
// defines multi-panel layout options - normally "none" should be fine
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);
// Background layer options - again "none" should be fine
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
```

### GIF Animation
----
Most configuration related to animating GIFs is in [animatedgif_config.h](include/animatedgif_config.h)
 ```cpp
// Set the maximum dimensions of GIF decoding
#define gif_width 64
#define gif_height 64

// The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
// lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
// All 32x32-pixel GIFs tested work with 11, most work with 10
const int lzwMaxBits = 11;

// Define the folder to pull GIFs from
// This folder must exist under the data directory in your project
#define GIF_DIRECTORY "/gifs" //do NOT add a trailing slash
 ```

Some are in [main.cpp](src/main.cpp):
```cpp
// Shift GIF on screen by setting the offset from the upper left corner
// Negative or positive values are ok
int OFFSETX = 0;
int OFFSETY = 0;

// If set to 15 then ~double GIF size? Does nothing if set to anything other than 15
int FACTX = 0;
int FACTY = 0;
```

### General Settings
----
Most general settings are configured in [main.cpp](src/main.cpp):
```cpp
#define DISABLE_MATRIX_TEST //Delete or comment out this line to display a panel alignment pattern on startup


#define defaultBrightness 25 // Default LED panel brightness
#define minBrightness 0 // Minimum LED panel brightness allowed by web client
#define maxBrightness 210 // Maximum LED panel brightness allowed by web client

#define FIRSTINDEX 0 // 0 based index of GIF to play first
```

## PlatformIO Operations
----------
 - Connect the ESP32 board to your computer via USB. Make sure it is recognized and the appropriate driver is used. Generally this should all happen automatically.
 - PlatformIO should automatically detect and choose the correct port. If not then you will need to identify and choose the correct COM port on your own.
 - Make sure that any serial monitors to your board are closed before attempting to upload any code or other data.
 - After starting an upload from VSCode/PlatformIO, when it displays "Connecting", you will need to hold down the "Boot" button on the board for ~3 seconds until the upload begins. Once you see upload progress you can release the button.

### Uploading Code
----
1. Open the PlatformIO panel in VSCode and choose "Upload" under rorrosaurus-esp32-hub75-driver -> General
   - PlatformIO will download and compile any neccessary dependencies and then attempt to compile the project before starting the upload
1. Once the status changes to "Connecting" in the terminal window, press and hold the "Boot" button on the board for ~3 seconds. You may release the button once upload progress begins.
1. When the upload is complete it should start executing the program automatically. If you have any trouble press the "EN" button on the board to reset/reload the software.

### Uploading GIFs
----
Remember space and memory are very limited on the ESP32 so keep GIFs small and short! SPIFFS (the filesystem where the GIFs are stored) can not handle long filenames so keep those short as well.
1. Place the desired GIFs in the [data/gifs](data/gifs) folder.
   - The entire data folder will be transferred to the board each time you upload a filesystem image. All data that was already on the board will be overwritten each time. Make sure that everything you want on the board is in the data folder when building and uploading the filesystem image.
1. Open the PlatformIO panel in VSCode and choose "Build Filesystem Image" under rorrosaurus-esp32-hub75-driver -> Platform
1. When complete choose "Upload Filesystem Image" under rorrosaurus-esp32-hub75-driver -> Platform
1. The data should be available to the board and its software. Upload the program if you haven't already.

### Serial Monitoring
----
Most features of the project print some sort of status or configuration information to the ESP32's serial console. These messages can be viewed by connecting to the board via PlatformIO's Serial Monitor. The serial console can also send messages to the software while running to enable additional debugging information, change settings, or any other functionality added to the software.
1. Open VSCode/PlatformIO and navigate to the "Serial Monitor" panel next to the Terminal and Output panels near the bottom of the window.
1. PlatformIO should already have selected the appropriate connection settings. Confirm that the selected port is the correct ESP32 board and then click "Start Monitoring".
1. Many of the configuration items are printed as the program is first starting up. If you want to view these items, start the serial monitor and then press the "EN" button on the board to restart it. The serial monitor will remain connected and display all messages as they are generated.
1. Remember that the serial monitor needs to be closed in order to upload any code or data to the board. If it is not closed you will get a permissions error when attempting to connect to the board.

You may enter the following commands to invoke debugging and/or control functions through the serial console:
 - "n" -> Next GIF command
 - "p" -> Previous GIF command
 - "f" -> Enter debug frames mode
 - "g" -> Next frame in debug frames mode
 - "c" -> Toggle clearing of screen before each frame
 - "+" -> Increase gamma value by 0.2
 - "-" -> Decrease gamma value by 0.2
 - "0" - "9" -> Display GIF with entered index

## Program Usage
----------
### Web Interface
----
The ESP32 will broadcast its own Wifi network ("myLEDPanel" by default) which you can connect to access the programs web interface. From here you can select the current GIF, adjust brightness, or any other setting which is available there.
1. On another device find and connect to the Wifi network name you specified in the Wifi configuration. By default the network is named "myLEDPanel" and does not have a passphrase.
   - The Wifi network the board creates does not have internet access so mobile devices may complain and automatically attempt to connect to a better network. Disable this behavior to prevent issues sending commands to the program.
1. Open the web interface by entering the IP address of the device into a web browser. Normally the IP address should be 192.168.4.1
1. Adjust any settings you'd like and watch the display change accordingly.
