//=============================================================================
// "Arduino" example program for Crystalfontz ePaper.
//
// This project is for the CFAP152152C1-0154:
//
//   https://www.crystalfontz.com/product/cfap152152c10154
//
// It was written against a Seeduino v4.2 @3.3v. An Arduino UNO modified to
// operate at 3.3v should also work.
//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
//=============================================================================
// Connecting the Arduino to the display
//
// ARDUINO |Wire Color |Function
// --------+-----------+--------------------
// D3      |Green      |Busy Line
// D4      |Brown      |Reset Line
// D5      |Purple     |Data/Command Line
// D10     |Blue       |Chip Select Line
// D11     |White      |MOSI
// D13     |Orange     |Clock
// 3.3V    |Red        |Power
// GND     |Black      |Ground
//
// Short the following pins on the adapter board:
// BS1 -> "LO" |OR (depending on adapter board)| BS2 -> "GND"
// RESE -> .47ohms
//=============================================================================
// Connecting the Arduino to the SD card
//
// ARDUINO  |Wire Color |Function
// ---------+-----------+--------------------
// D8       |Blue       |CS
// D11      |Green      |MOSI
// D12      |Purple     |MISO
// D13      |Brown      |CLK
//
//=============================================================================
// Creating image data arrays
//
// Bmp_to_epaper is code that will aid in creating bitmaps necessary from .bmp files.
// The code can be downloaded from the Crystalfontz website: https://www.Crystalfontz.com
// or it can be downloaded from github: https://github.com/crystalfontz/bmp_to_epaper
//=============================================================================

// The display is SPI, include the library header.
#include <SPI.h>
#include <SD.h>
#include <avr/io.h>

// Include LUTs
#include "Images_for_CFAP152152C10154.h"

#define EPD_READY 3
#define EPD_RESET 4
#define EPD_DC 5
#define EPD_CS 10
#define SD_CS 8

#define ePaper_RST_0 (digitalWrite(EPD_RESET, LOW))
#define ePaper_RST_1 (digitalWrite(EPD_RESET, HIGH))
#define ePaper_CS_0 (digitalWrite(EPD_CS, LOW))
#define ePaper_CS_1 (digitalWrite(EPD_CS, HIGH))
#define ePaper_DC_0 (digitalWrite(EPD_DC, LOW))
#define ePaper_DC_1 (digitalWrite(EPD_DC, HIGH))

#define HRES 152
#define VRES 152

//=============================================================================
// this function will take in a byte and send it to the display with the
// command bit low for command transmission
void writeCMD(uint8_t command)
{
  delay(1);
  ePaper_DC_0;
  ePaper_CS_0;
  SPI.transfer(command);
  ePaper_CS_1;
}

//=============================================================================
// this function will take in a byte and send it to the display with the
// command bit high for data transmission
void writeData(uint8_t data)
{
  ePaper_DC_1;
  ePaper_CS_0;
  SPI.transfer(data);
  ePaper_CS_1;
}

//=============================================================================
// This function will intialize the display
void initEPD()
{

  // reset driver
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);

  //-----------------------------------------------------------------------------
  // more detail on the following commands and additional commands not used here
  // can be found on the CFAP152152C1-0154 datasheet on the Crystalfontz website
  //-----------------------------------------------------------------------------

  writeCMD(0x04); // Power on

  waitBusyLine();

  writeCMD(0x00);  // panel setting
  writeData(0x1f); // LUT from OTP????KW-BF   KWR-AF  BWROTP 0f BWOTP 1f

  writeCMD(0x50);  // VCOM AND DATA SETTING
  writeData(0x97); // WBmode
}

//=============================================================================
void waitBusyLine()
{
  writeCMD(0x71);
  while (0 == digitalRead(EPD_READY))
    ;
  delay(200);
}

//================================================================================
void Load_Flash_Image_To_Display_RAM(uint16_t width_pixels,
                                     uint16_t height_pixels,
                                     const uint8_t *BW_image)
{
  // Index into *image, that works with pgm_read_byte()
  uint16_t index;

  // Get width_bytes from width_pixel, rounding up
  uint8_t width_bytes;
  width_bytes = width_pixels / 8;

  writeCMD(0x10);
  for (index = 0; index < 2888; index++)
  {
    writeData(0xff);
  }

  // Display start transimmiting black/white data old data.
  // Write the command: DATA START TRANSMISSION 2 (DTM2) (R13H)
  //  Display Start transmission 2
  //  (DTM2, Yellow Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes ???NEW??? data to SRAM.
  //  * In B/W/Yellow mode, this command writes ???Yellow??? data to SRAM.
  writeCMD(0x13); // new data
  // Pump out the B/W data.

  index = 0;
  for (uint16_t y = 0; y < height_pixels; y++)
  {
    for (uint8_t x = 0; x < width_bytes; x++)
    {
      // We have to use pgm_read_byte to pull from flash memory (PROGMEM)
      writeData(pgm_read_byte(&BW_image[index]));
      index++;
    }
  }

  // Write the command: Display Refresh (DRF)
  writeCMD(0x12);
}

//================================================================================
void show_BMPs_in_root(void)
{
  File
      root_dir;
  root_dir = SD.open("/");
  if (0 == root_dir)
  {
    Serial.println("show_BMPs_in_root: Can't open \"root\"");
    return;
  }

  File
      bmp_file;

  while (1)
  {
    bmp_file = root_dir.openNextFile();
    if (0 == bmp_file)
    {
      // no more files, break out of while()
      // root_dir will be closed below.
      break;
    }
    // Skip directories (what about volume name?)
    if (0 == bmp_file.isDirectory())
    {
      // The file name must include ".BMP"
      if (0 != strstr(bmp_file.name(), ".BMP"))
      {
        Serial.println(bmp_file.name());
        // The BMP must be exactly 36918 long
        //(this is correct for182x96, 24-bit)
        uint32_t size = bmp_file.size();

        Serial.println(size);
        if ((uint32_t)HRES * VRES * 3 + 50 <= size <= (uint32_t)HRES * VRES * 3 + 60)
        {
          Serial.println("in final loop");

          // Make sure the display is not busy before starting a new command.
          Serial.print("refreshing......");
          // while (0 == digitalRead(EPD_READY));
          Serial.println(" complete");
          // Select the controller

          writeCMD(0x13);
          // Jump over BMP header
          bmp_file.seek(54);

// Throw this in here to make it easier to change the length of the array
#define divider 1
          // grab one row of pixels from the SD card at a time
          // The length of this array must be divisible by 8
          uint8_t one_line[HRES / divider * 3];
          Serial.println(VRES * 2);
          for (int line = 0; line < VRES * divider; line++)
          {
            // Set the LCD to the left of this line. BMPs store data
            // to have the image drawn from the other end, uncomment the line below

            // read a line from the SD card
            bmp_file.read(one_line, HRES / divider * 3);

            // send the line to the display
            send_pixels_BW(HRES / divider * 3, one_line);
          }
          Serial.println("outside of loop");

          Serial.print("refreshing......");
          // Write the command: Display Refresh (DRF)
          writeCMD(0x12);
          // Make sure the display is not busy before starting a new command.
          waitBusyLine();

          Serial.println(" complete");
          // Give a bit to let them see it
          delay(3000);
        }
      }
    }
    // Release the BMP file handle
    bmp_file.close();
  }
  // Release the root directory file handle
  root_dir.close();
}

//================================================================================
void send_pixels_BW(uint16_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    // Serial.println(byteCount);
    // delay(100);
    uint8_t data = 0;
    // the first byte received from a .BMP file is red, then green, then blue.
    red = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;

    // check to see if pixel should be set as black or white
    if (127 > ((red * .21) + (green * .72) + (blue * .07)))
    {
      data = data | 0x01;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
      red = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      blue = *dataPtr;
      dataPtr++;
      byteCount--;

      // shift data over one bit so we can store the value
      data = data << 1;

      // check to see if pixel should be set as black or white
      if (127 > ((red * .21) + (green * .72) + (blue * .07)))
      {
        data = data | 0x01;
      }
      else
      {
        data = data & 0xFE;
      }
    }
    writeData(data);
  }
}

//===========================================================================
void partialUpdateSolid(uint8_t x1, uint16_t y1, uint8_t x2, uint16_t y2, uint8_t color)
{
  // turn on partial update mode
  writeCMD(0x91);

  // set the partial area
  writeCMD(0x90);
  writeData((x1)&0xf8);       // x1
  writeData((x2 - 8) | 0x07); // x2
  writeData(y1 >> 8);         // 1st half y1
  writeData(y1 & 0xff);       // 2nd half y1
  writeData(y2 >> 8);         // 1st half y2
  writeData((y2 - 1) & 0xff); // 2nd half y2
  writeData(0x01);

  int i;
  int h;

  // send the old data to SRAM
  writeCMD(0x10);
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i < (x2 - x1) / 8; i++)
    {
      writeData(0xff);
    }
  }

  // send black and white information
  writeCMD(0x13);
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i < (x2 - x1) / 8; i++)
    {
      writeData(color);
    }
  }

  // Write the command: Display Refresh (DRF)
  writeCMD(0x12);
  waitBusyLine();

  // turn off partial update mode
  writeCMD(0x92);
}

//===========================================================================
void powerON()
{
  writeCMD(0x04);
}

void powerOff()
{
  writeCMD(0x03);
  writeData(0x00);
  writeCMD(0x02);
}

void sleepEPD()
{
  writeCMD(0x50);
  writeData(0xf7);

  writeCMD(0X02); // power off

  waitBusyLine();

  writeCMD(0X07); // deep sleep
  writeData(0xA5);
}

//===========================================================================
void setup(void)
{
  // Debug port / Arduino Serial Monitor (optional)
  Serial.begin(115200);
  Serial.println("setup started");
  // Configure the pin directions
  pinMode(EPD_CS, OUTPUT);
  pinMode(EPD_RESET, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_READY, INPUT);
  pinMode(SD_CS, OUTPUT);

  // select the EPD
  ePaper_CS_0;

  // If an SD card is connected, do the SD setup
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD could not initialize");
  }

  // Set up SPI interface @ 2MHz
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  // go to the EPD init code
  initEPD();
  Serial.println("setup complete");
}

//=============================================================================
// The following code enables/disables different demos
#define waittime 2000
#define splashscreen 1
#define black 1
#define checkerboard 1
#define partialUpdate 1
#define showBMPs 1
#define white 1
// It is recommended that at least 2min are waited in between each update for EPDs. For demo purposes
// we are bypassing this but in the final use environment, the recommendation should be followed
void loop()
{

#if splashscreen
  // Initialize the display
  initEPD();
  // load an image to the display
  Load_Flash_Image_To_Display_RAM(HRES, VRES, Splash_Mono_1BPP);

  Serial.print("refreshing . . . ");
  delay(100);
  waitBusyLine();
  Serial.println("refresh complete");
  // for maximum power conservation, put the EPD to sleep
  sleepEPD();
  delay(waittime);
#endif

#if black
  // Initialize the display
  initEPD();

  // Write the old data to SRAM
  writeCMD(0x10);
  for (int i = 0; i < (uint32_t)VRES * HRES / 8; i++)
  {
    writeData(0xff);
  }

  // Write the new data to SRAM
  writeCMD(0x13);
  for (int i = 0; i < (uint32_t)VRES * HRES / 8; i++)
  {
    writeData(0x00);
  }

  // Write the command: Display Refresh (DRF)
  writeCMD(0x12);
  waitBusyLine();
  // for maximum power conservation, put the EPD to sleep
  sleepEPD();

  delay(waittime);
#endif

#if checkerboard
  // Initialize the display
  initEPD();

  // color is for the bw data
  uint8_t color = 0x00;

  // Write the old data to SRAM
  writeCMD(0x10);
  for (int i = 0; i < (uint32_t)VRES * HRES / 8; i++)
  {
    writeData(0xff);
  }

  // Write the new data to SRAM
  writeCMD(0x13);
  for (int i = 0; i < VRES; i++)
  {
    if (i % 8 != 0)
    {
      color = ~color;
    }
    for (uint8_t j = 0; j < HRES / 8; j++)
    {
      writeData(color);
      color = ~color;
    }
  }

  Serial.println("before refresh wait");
  // refresh the display
  writeCMD(0x12);
  waitBusyLine();
  Serial.println("after refresh wait");
  // for maximum power conservation, put the EPD to sleep
  sleepEPD();
  delay(waittime);
#endif

#if partialUpdate
  // Initialize the display
  initEPD();

  partialUpdateSolid(48, 48, 112, 112, 0xff);
  delay(1000);

  partialUpdateSolid(48, 48, 112, 112, 0x00);
  delay(1000);

  // for maximum power conservation, put the EPD to sleep
  sleepEPD();
  delay(waittime);
#endif

#if showBMPs
  // Initialize the display
  initEPD();

  // Panel Setting
  // the image has to be turned upside down since BMPs are stored
  // bottom to top
  writeCMD(0x00);
  writeData(0x1b);

  show_BMPs_in_root();
  // reset Panel Setting
  writeCMD(0x00);
  writeData(0x1f);
  // for maximum power conservation, put the EPD to sleep
  sleepEPD();
  delay(waittime);

#endif

#if white
  // Initialize the display
  initEPD();

  // Write the old data to SRAM
  writeCMD(0x10);
  for (int i = 0; i < (uint32_t)VRES * HRES / 8; i++)
  {
    writeData(0xff);
  }

  // Write the new data to SRAM
  writeCMD(0x13);
  for (int i = 0; i < (uint32_t)VRES * HRES / 8; i++)
  {
    writeData(0xff);
  }

  // Write the command: Display Refresh (DRF)
  writeCMD(0x12);
  waitBusyLine();
  // for maximum power conservation, put the EPD to sleep
  sleepEPD();
  delay(waittime);
#endif

  while (1)
    ;
}
//=============================================================================
