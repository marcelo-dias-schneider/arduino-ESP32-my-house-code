#include "lcd.h"

LiquidCrystal_I2C mylcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Non-blocking LCD manager
static const int MAX_LCD_MESSAGES = 20;
static String msgBuffer[MAX_LCD_MESSAGES];
static int msgCount = 0;
static unsigned long pageDelay = 1000;
static unsigned long scrollDelay = 250;

static int currentPage = 0; // index of first message on current page
static int scrollPos0 = 0;
static int scrollPos1 = 0;
static bool finished0 = false;
static bool finished1 = false;
static unsigned long lastScrollMillis = 0;
static unsigned long pageStartMillis = 0;

void lcdInit()
{
  mylcd.init();
  mylcd.backlight();
  mylcd.clear();
}

static void updateDisplay()
{
  mylcd.clear();

  // Row 0
  if (currentPage < msgCount)
  {
    String &t0 = msgBuffer[currentPage];
    if (t0.length() <= LCD_COLUMNS)
      mylcd.print(t0);
    else
      mylcd.print(t0.substring(scrollPos0, scrollPos0 + LCD_COLUMNS));
  }

  // Row 1
  if (currentPage + 1 < msgCount)
  {
    mylcd.setCursor(0, 1);
    String &t1 = msgBuffer[currentPage + 1];
    if (t1.length() <= LCD_COLUMNS)
      mylcd.print(t1);
    else
      mylcd.print(t1.substring(scrollPos1, scrollPos1 + LCD_COLUMNS));
  }
}

void setMessageLCD(String messages[], int count, unsigned long delayTime)
{
  // copy messages into internal buffer (truncate if necessary)
  msgCount = min(count, MAX_LCD_MESSAGES);
  for (int i = 0; i < msgCount; ++i)
    msgBuffer[i] = messages[i];

  pageDelay = delayTime;
  scrollDelay = delayTime; // original code used same delay for scroll and page

  currentPage = 0;
  scrollPos0 = scrollPos1 = 0;
  finished0 = finished1 = false;
  lastScrollMillis = millis();
  pageStartMillis = millis();

  updateDisplay();
}

void lcdLoop()
{
  if (msgCount == 0)
    return;

  unsigned long now = millis();

  int idx0 = currentPage;
  int idx1 = (currentPage + 1 < msgCount) ? currentPage + 1 : -1;

  // handle scrolling steps
  if (now - lastScrollMillis >= scrollDelay)
  {
    lastScrollMillis = now;

    // Row 0
    if (idx0 >= 0)
    {
      String &t0 = msgBuffer[idx0];
      if (t0.length() > LCD_COLUMNS)
      {
        if (scrollPos0 < t0.length() - LCD_COLUMNS)
        {
          scrollPos0++;
          finished0 = false;
        }
        else
        {
          finished0 = true;
        }
      }
      else
        finished0 = true;
    }
    else
      finished0 = true;

    // Row 1
    if (idx1 >= 0)
    {
      String &t1 = msgBuffer[idx1];
      if (t1.length() > LCD_COLUMNS)
      {
        if (scrollPos1 < t1.length() - LCD_COLUMNS)
        {
          scrollPos1++;
          finished1 = false;
        }
        else
        {
          finished1 = true;
        }
      }
      else
        finished1 = true;
    }
    else
      finished1 = true;

    updateDisplay();
  }

  // when both rows finished scrolling, wait pageDelay then advance page
  if (finished0 && finished1 && (now - pageStartMillis >= pageDelay))
  {
    currentPage += 2;
    if (currentPage >= msgCount)
      currentPage = 0;

    scrollPos0 = scrollPos1 = 0;
    finished0 = finished1 = false;
    pageStartMillis = now;

    updateDisplay();
  }
}
