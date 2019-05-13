#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"


PwmOut display_led((PinName)11);


UISettings::UISettings(GFX* tft, IUIMode* mode)
: display_brightness_(5), uimode_(mode)
, tft_(tft)
{

    display_led.period_ms(1);  
    display_led = (float)display_brightness_ / 10.0f;
}

void UISettings::LongPress()
{
    uimode_->SetUiMode(IUIMode::eCsc);
}

void UISettings::ShortPress()
{
    IncDislayBrightness();
    Draw();
}


void UISettings::IncDislayBrightness()
{
    display_brightness_ = (display_brightness_ < 10) ? (display_brightness_ + 1) : 0;
    display_led = (float)display_brightness_ / 10.0f;
}

void UISettings::Draw()
{
    char str[10];
    uint16_t len = sprintf(str, "Bat %i", csc_bat_);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(0xFFFF);
    tft_->WriteStringLen(0, 5, 80, str, len, 0, GFX::eCenter);

    len = sprintf(str, "Bri %i", display_brightness_);
    tft_->WriteStringLen(0, 35, 80, str, len, 0, GFX::eCenter);
}

void UISettings::UpdateBat(uint8_t val)
{
    csc_bat_ = val;
}