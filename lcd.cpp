#include "lcd.h"

LiquidCrystal_I2C mylcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

void lcdInit()
{
  mylcd.init();
  mylcd.backlight();
  mylcd.clear();
}

// Immediately update the 2-line LCD when called. This function is intended
// to be invoked only when `outputs.lcd.currentHash != outputs.lcd.lastHash`.
void setMessageLCD(String messages[], int count, unsigned long delayTime)
{
  Serial.println("LCD messages: Printing Hash " + outputs.lcd.currentHash);

  mylcd.clear();

  if (count > 0)
  {
    String t0 = messages[0];
    if (t0.length() > LCD_COLUMNS)
      t0 = t0.substring(0, LCD_COLUMNS);
    mylcd.print(t0);
  }

  if (count > 1)
  {
    mylcd.setCursor(0, 1);
    String t1 = messages[1];
    if (t1.length() > LCD_COLUMNS)
      t1 = t1.substring(0, LCD_COLUMNS);
    mylcd.print(t1);
  }
}
