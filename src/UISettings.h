#ifndef UISETTINGS_H_
#define UISETTINGS_H_

#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IUIMode.h"

class UISettings
{
    public:
    UISettings(GFX* tft, IUIMode* mode);
    void LongPress();
    void ShortPress();
    void IncDislayBrightness();
    void Draw();
    void UpdateBat(uint8_t val);

    uint8_t csc_bat_;
    uint8_t display_brightness_;
    GFX* tft_;
    IUIMode* uimode_;
};

#endif