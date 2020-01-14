#include "display.h"
#include <Wire.h>
#include <Arduino.h>
#include "utils.h"

#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Display::Display()
{
}

Display::~Display()
{

}

//////////////////////////////////////////////
void Display::initialize()
{

      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...
  myDebug_P(PSTR("[Display] Initialized"));

}

//////////////////////////////////////////////
void Display::Process()
{
    static int temperature = 200; //20.0

    char temp_cur[20] = "A: 20.0 C";
    char temp_set[20] = "S: 21.0 C";
    size_t i;
    size_t len;

    temperature += 1;
    if(temperature > 220)
        temperature = 180;
    snprintf((char *)&temp_cur[0], 20, "A: %d.%d C", temperature / 10, (temperature % 10)); 
    myDebug_P(PSTR("[Display] %s"), (char *)&temp_cur);
    display.clearDisplay();

    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Not all the characters will fit on the display. This is normal.
    // Library will draw what it can and the rest will be clipped.
    len = strnlen((const char *)&temp_cur, 50);
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(temp_cur[i]);
    }
    //display.setCursor(0, 40);     // Start at top-left corner
    len = strnlen((const char *)&temp_set, 50);
    display.write(' ');
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(temp_set[i]);
    }
    display.display();
    delay(10);
}