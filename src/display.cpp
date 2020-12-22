#include "display.h"
#include <Wire.h>
#include <Arduino.h>
#include "utils.h"

#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define REL_TEMP_ON_SCREEN //for printing the relative temperature ~(hum * temp) output


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define STRLEN 20
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
void Display::process(float temperature, float setPoint, float humidity, float relTemp)
{

    char temp_cur[STRLEN] = "A 20.0 °C";
    char temp_set[STRLEN] = "S 21.0 °C";
    char hum_curr[STRLEN] = "  55.0%  ";
    char rel_temp[STRLEN] = "R 20.0 C";

    char temp_string[4] = {'\0', '\0','\0', '\0'};

    size_t i;
    size_t len;

    memset(temp_cur, 0, STRLEN);
    memset(temp_set, 0, STRLEN);
    memset(hum_curr, 0, STRLEN);

    dtostrf(temperature, 2, 1, temp_string);
    snprintf((char *)&temp_cur[0], 20, "C : %s C",temp_string); 
    dtostrf(setPoint, 2, 1, temp_string);
    snprintf((char *)&temp_set[0], 20, "Sp: %s C",temp_string); 
    dtostrf(humidity, 2, 0, temp_string);
    snprintf((char *)&hum_curr[0], 20, "     %s%%",temp_string); 

    dtostrf(relTemp, 2, 1, temp_string);
    snprintf((char *)&rel_temp[0], 20, "     %s%C",temp_string); 
    myDebug_P(PSTR("[Display] Process %s <-> %s [%s] [%s]"), (char *)&temp_cur,  (char *)&temp_set, (char *)&hum_curr, (char *)&rel_temp);
    display.clearDisplay();

    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Not all the characters will fit on the display. This is normal.
#ifdef REL_TEMP_ON_SCREEN    // Library will draw what it can and the rest will be clipped.
    len = strnlen((const char *)&rel_temp, 50);
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(rel_temp[i]);
    }
#else
    len = strnlen((const char *)&temp_cur, 50);
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(temp_cur[i]);
    }
#endif
    display.setCursor(0, 20);     // Start at top-left corner, 40 down
    len = strnlen((const char *)&temp_set, 50);
    //display.write(' ');
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(temp_set[i]);
    }
    display.setCursor(0, 40);     // Start at top-left corner, 40 down
    len = strnlen((const char *)&hum_curr, 50);
    //display.write(' ');
    for(i=0; i < len; i++) {
        //if(i == '\n') display.write(' ');
        //else          display.write(i);
        display.write(hum_curr[i]);
    }


    display.display();
}