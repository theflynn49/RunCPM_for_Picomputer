/*
    (c) Marcelo Dantas https://github.com/MockbaTheBorg/RunCPM
    (c) GUIDO LEHWALDER https://github.com/guidol70/RunCPM_RPi_Pico/tree/main/v6_7
    port/fork version for Picomputer by theFlynn49 2025 https://github.com/theflynn49
*/
#include <SdFat_Adafruit_Fork.h>

/*
  SD card connection

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
   // Arduino-pico core
   ** MISO - Pin 21 - GPIO 16
   ** MOSI - Pin 25 - GPIO 19
   ** CS   - Pin 22 - GPIO 17
   ** SCK  - Pin 24 - GPIO 18
*/

// only AVR and ARM CPU
// #include <MemoryFree.h>

#include "globals.h"

// =========================================================================================
// Guido Lehwalder's Code-Revision-Number
// =========================================================================================
#define GL_REV  "GL20250601.0"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "KMatrix.h"
// #include "pico.h"
// #include "hardware/vreg.h"

#define TFT_CS        21
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        16
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_BL   20

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define SPI_DRIVER_SELECT 0
#define SDFAT_FILE_TYPE 1           // Uncomment for Due, Teensy or RPi Pico
#define ENABLE_DEDICATED_SPI 1      // Dedicated SPI 1=ON 0=OFF

#include <SdFat.h>        // One SD library to rule them all - Greinman SdFat from Library Manager
// #include <ESP8266SdFat.h>    // SdFat-version from RP2040 arduino-core

// Test with reduced SPI speed for breadboards.
// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
// const uint8_t spiSpeed = SPI_HALF_SPEED;

// =========================================================================================
// Board definitions go into the "hardware" folder, if you use a board different than the
// Arduino DUE, choose/change a file from there and reference that file here
// =========================================================================================

// Raspberry Pi Pico - normal (LED = GPIO25)
#include "hardware/pico/pico_sd_spi.h"

// Raspberry Pi Pico W(iFi)   (LED = GPIO64)
// #include "hardware/pico/pico_w_sd_spi.h"

// =========================================================================================
// Delays for LED blinking
// =========================================================================================
#define sDELAY 200
#define DELAY  400

#include "abstraction_arduino.h"

// =========================================================================================
// Serial port speed
// =========================================================================================
#define SERIALSPD 115200

// =========================================================================================
// PUN: device configuration
// =========================================================================================
#ifdef USE_PUN
File32 pun_dev;
int pun_open = FALSE;
#endif

// =========================================================================================
// LST: device configuration
// =========================================================================================
#ifdef USE_LST
File32 lst_dev;
int lst_open = FALSE;
#endif

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif


void setup(void) {

//  vreg_set_voltage(VREG_VOLTAGE_1_20);
//  delay(10);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init(240, 320);

// =========================================================================================
// Serial Port Definition
// =========================================================================================
//  Serial = USB
// =========================================================================================
//  Serial1 = UART0/COM1 / 
// -----------------------------------------------------------------------------------------
//   Serial1.setTX(0);
//   Serial1.setRX(1);
// -----------------------------------------------------------------------------------------
//   Serial1.setTX(12);
//   Serial1.setRX(13);
// -----------------------------------------------------------------------------------------
//   Serial1.setTX(16);
//   Serial1.setRX(17);   
// =========================================================================================
//  Serial2 = UART1/COM2
// -----------------------------------------------------------------------------------------
//   Serial2.setTX(4);
//   Serial2.setRX(5);
// -----------------------------------------------------------------------------------------
//   Serial2.setTX(8);
//   Serial2.setRX(9);
// =========================================================================================

  // _clrscr();
  // _puts("Opening serial-port...\r\n");

  // Serial.setFIFOSize(128);
  Serial.begin(SERIALSPD);
  /*
  while (!Serial) {  // Wait until serial is connected
    digitalWrite(LED, HIGH^LEDinv);
    delay(sDELAY);
    digitalWrite(LED, LOW^LEDinv);
    delay(DELAY);
  }
  */

#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  
// =========================================================================================  
// Printing the Startup-Messages
// =========================================================================================

  // if (bootup_press == 1)
  //   { _puts("Recognized \e[1m#\e[0m key as pressed! :)\r\n\r\n");
  //   }
  
  if (getMatrix(0)=='q') {
    // Force the Pico into bootloader mode
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
  }

  //tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(3) ;
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_GREY, ST77XX_BLACK);
  tft.setTextSize(1);
  _clrscr();

  _puts("CP/M Emulator \e[1m\e#3v" VERSION "\e[0m   by   \e[1mMarcelo  Dantas\e[0m\r\n");
  _puts("RPI-Pico port        by   \e[1mGuido Lehwalder\e[0m\r\n");
  _puts("Picomputer port      by   \e[1mtheFlynn49\e[0m\r\n");

  #ifdef ABDOS
  _puts("     (A)BDOS.SYS     by   \e[1mPavel    Zampach\e[0m\r\n");
  #endif
  
  _puts("-------------------------------------------------\r\n");  
//  _puts("     running    on   Raspberry Pi  [\e[1mPico 2\e[0m] [\e[1mARM\e[0m]\r\n");
  _puts("     running    on   Raspberry Pi  [\e[1m Pico \e[0m]\r\n");
//  _puts("     running    on   Raspberry Pi  [\e[1mPico-W\e[0m]\r\n");
  _puts("     compiled with   RP2040        [\e[1mv3.9.5\e[0m] \r\n");  
//  _puts("               and   ESP8266SdFat [\e[1mv2.2.2\e[0m] \r\n");
  _puts("                     SDFat         [\e[1mv2.2.54\e[0m] \r\n");  
  _puts("                     GFX           [\e[1mv1.12.1\e[0m] \r\n");  
  _puts("               and   ST77XX        [\e[1mv1.11.0\e[0m] \r\n");  
  
  _puts("                     Revision      [\e[1m");
  _puts(GL_REV);
  _puts("\e[0m]\r\n");
  
  _puts("-------------------------------------------------\r\n");

  _puts("BIOS              at [\e[1m0x");
  _puthex16(BIOSjmppage);
//  _puts(" - ");
  _puts("\e[0m]\r\n");

  #ifdef ABDOS
	_puts("ABDOS.SYS \e[1menabled\e[0m at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #else
	_puts("BDOS              at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #endif

  _puts("CCP               at [\e[1m0x");
  _puthex16(CCPaddr);
        _puts("\e[0m]     [\e[1m");
        _puts(CCPname);
  _puts("\e[0m]\r\n");

   #if BANKS > 1
  _puts("Banked Memory        [\e[1m");
  _puthex8(BANKS);
    _puts("\e[0m]banks\r\n");
  #else
  _puts("Banked Memory        [\e[1m");
  _puthex8(BANKS);
  _puts("\e[0m]bank\r\n");
  #endif

   // Serial.printf("Free Memory          [\e[1m%d bytes\e[0m]\r\n", freeMemory());

  _puts("CPU-Clock            [\e[1m250Mhz\e[0m]\r\n");


// =========================================================================================
// SPIINIT !! ONLY !! for ESP32-Boards = #define board_esp32
// =========================================================================================
#if defined board_esp32
  _puts("Initializing  SPI    [ ");
  SPI.begin(SPIINIT);
  _puts("\e[1mDone\e[0m ]\r\n");
#endif

// =========================================================================================
// Redefine SPI-Pins - if needed : (SPI.) = SPI0 / (SPI1.) = SPI1
// =========================================================================================
/*
  SPI.setRX(16);   // MISO
  SPI.setCS(17);   // Card Select
  SPI.setSCK(18);  // Clock
  SPI.setTX(19);   // MOSI
  */
  SPI1.setRX(12);   // MISO
  SPI1.setCS(13);   // Card Select
  SPI1.setSCK(10);  // Clock
  SPI1.setTX(11);   // MOSI

// =========================================================================================
// Setup SD card writing settings
// Info at: https://github.com/greiman/SdFat/issues/285#issuecomment-823562829
// =========================================================================================

// These 2 has been definied before the include of SdFat.h at the top of the .ino
// #define SDFAT_FILE_TYPE 1           // Uncomment for Due, Teensy or RPi Pico
// #define ENABLE_DEDICATED_SPI 1      // Dedicated SPI 1=ON 0=OFF

#define SDMHZ_TXT "12"              // for outputing SDMHZ-Text
// normal is 12Mhz because of https://www.pschatzmann.ch/home/2021/03/14/rasperry-pico-with-the-sdfat-library/
#define SDMHZ 12                    // setting Mhz  - SPI-Bus = SPI_FULL_SPEED = 50Mhz
// #define SS 17
#define SS 13
// select required SPI-Bus : (&SPI) = SPI0 / (&SPI1) = SPI1
#define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(SDMHZ), &SPI1) // self-selecting the Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_FULL_SPEED, &SPI)
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_QUARTER_SPEED, &SPI)  // for breadboard QUARTER - 2Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_HALF_SPEED, &SPI)     // for breadboard HALF    - 4Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_DIV3_SPEED, &SPI)        // for z.B. DUE        - 16Mhz

// =========================================================================================

  _puts("Init MicroSD-Card    [ \e[1m");

//  old SDINIT
//  if (SD.begin(SDINIT)) {

 
// NEW SDCONFIG = formerly SDINIT
if (SD.begin(SD_CONFIG)) {
                        _puts(SDMHZ_TXT);
                        _puts("Mhz\e[0m]\r\n");
  _puts("-------------------------------------------------");

  _puts("\r\nPress 'Q' during Reset to enter boot mode.");
                        
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
#ifdef ABDOS
      _PatchBIOS();
#endif
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
  Status = 0;

 

#ifdef CCP_INTERNAL
        _ccp();
#else
        if (!_RamLoad((uint8 *)CCPname, CCPaddr, 0)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
     		// Loads an autoexec file if it exists and this is the first boot
		    // The file contents are loaded at ccpAddr+8 up to 126 bytes then the size loaded is stored at ccpAddr+7
		    if (firstBoot) {
			    if (_sys_exists((uint8*)AUTOEXEC)) {
				    uint16 cmd = CCPaddr + 8;
				    uint8 bytesread = (uint8)_RamLoad((uint8*)AUTOEXEC, cmd, 125);
				    uint8 blen = 0;
				    while (blen < bytesread && _RamRead(cmd + blen) > 31)
				    	blen++;
				    _RamWrite(cmd + blen, 0x00);
				    _RamWrite(--cmd, blen);
			    }
			    if (BOOTONLY)
				    firstBoot = FALSE;
		    }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(DSKByte));
        PC = CCPaddr;
        Z80run();
#endif
        if (Status == 1)
#ifdef DEBUG
	#ifdef DEBUGONHALT
    			Debug = 1;
		    	Z80debug();
	#endif
#endif
          break;

#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
}

void loop(void) {
  /*
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY);
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY * 4);
*/
  delay(DELAY) ;
}
