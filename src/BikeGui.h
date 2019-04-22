#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "gfxfont.h"
#include "FreeMonoBold24pt7b.h"
#include <Adafruit_ST7735.h>
#include "icons.h"

class BikeGUI
{

    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
    //PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

  public:
    BikeGUI()
    {
        tft.initR(INITR_MINI160x80);
        tft.setTextWrap(false);
        tft.fillScreen(ST77XX_BLACK);
        tft.setFont(&FreeMonoBold24pt7b);
        tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));
    }

    void UpdateSpeed(uint8_t speed)
    {
        tft.fillRect(0, 80, 80, 50, ST77XX_BLACK);
        tft.setCursor(10, 120);
        tft.printf("%d", speed);
    }
};

#endif