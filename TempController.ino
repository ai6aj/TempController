// #include <DallasTemperature.h>
#include <DHT.h>
//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
#include "TempSensor.h"
#include "Heater.h"

TempSensor tempSensor(12);
Heater theHeater(13, &tempSensor);

/* LCD shield solder points

    TOP CONNECTOR
    7  6  5  4  3  2  1
    13 12 11 D3 D2 D1 D0


*/



// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#include "screens.h"

// define some values used by the panel and buttons

#define BUTTON_RIGHT  1
#define BUTTON_UP     2
#define BUTTON_DOWN   3
#define BUTTON_LEFT   4
#define BUTTON_SELECT 5
#define BUTTON_NONE   0
#define BUTTON_DEBOUNCE 6

// read the buttons
int prevAdcRead = 1024;
int read_LCD_buttons()
{
  int adc_key_in = analogRead(0);      // read the value from the sensor
  int priorRead = prevAdcRead;
  prevAdcRead = adc_key_in;

  if (abs(adc_key_in - priorRead) > 1) {
    return BUTTON_DEBOUNCE;
  }

  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return BUTTON_NONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return BUTTON_RIGHT;
  if (adc_key_in < 200)  return BUTTON_DOWN;
  if (adc_key_in < 375)  return BUTTON_UP;
  if (adc_key_in < 550)  return BUTTON_LEFT;
  if (adc_key_in < 800)  return BUTTON_SELECT;
  return BUTTON_NONE;  // when all others fail, return this...
}

StartScreen startScreen;



void setup() {
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0, 0);
  theHeater.setTargetTemp(750);
}


void xxloop() {
  tempSensor.loop();
  theHeater.loop();
  lcd.setCursor(0, 0);
  lcd.print(formatTemp(tempSensor.getAsyncTemperature(0), 'F'));
  lcd.setCursor(10, 0);
  lcd.print(formatTemp(theHeater.getTargetTemp(), 'F'));
  lcd.setCursor(6, 1);
  if (theHeater.getHeaterState()) {
    lcd.print("Heat");
  }
  else {
    lcd.print("----");
  }
}

void loop()
{
  Screen* curscreen = &startScreen; //&filmProcessScreen;
  colorStandbyScreen.switchTo(curscreen, PROCESS_C41, 0, 0, 0, 960);
  curscreen->display();

  char ch;
  int haveEvent = 0;
  int lcd_key = BUTTON_NONE;

  while (1) {
    int new_lcd_key = read_LCD_buttons();  // read the buttons
    if (new_lcd_key != BUTTON_DEBOUNCE && lcd_key != new_lcd_key) {
      lcd_key = new_lcd_key;

      switch (lcd_key) {
        case BUTTON_UP:
          curscreen->up();
          haveEvent++;
          break;

        case BUTTON_DOWN:
          curscreen->down();
          haveEvent++;
          break;

        case BUTTON_LEFT:
          curscreen = curscreen->back();
          haveEvent++;
          break;

        case BUTTON_RIGHT:
          curscreen = curscreen->next();
          haveEvent++;
          break;
      }
    }

    if (curscreen->loop()) haveEvent++;
    tempSensor.loop();
    theHeater.loop();

    //if (haveEvent) {
    curscreen->display();
    haveEvent = 0;
    //}
  }
}

