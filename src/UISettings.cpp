#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"


PwmOut display_led((PinName)11);


UISettings::UISettings(GFX* tft, IUIMode* mode)
: display_brightness_(5)
, display_time_(0)
, komoot_alert_dist_(0)
, uimode_(mode)
, tft_(tft)
, sm_state_(eDim)
, setting_sm({
  { eDim, eShort, NULL, eTime}
, { eTime, eShort, NULL, eDist}
, { eDist, eShort, NULL, eExit}
, { eExit, eShort, NULL, eDim}

, { eDim, eLong, NULL, eDimSel}
, { eDimSel, eLong, NULL, eDim}
, { eDimSel, eShort, &UISettings::IncDislayBrightness, eDimSel}

, { eTime, eLong, NULL, eTimeSel}
, { eTimeSel, eLong, NULL, eTime}
, { eTimeSel, eShort, &UISettings::IncDislayTime, eTimeSel}

, { eDist, eLong, NULL, eDistSel}
, { eDistSel, eLong, NULL, eDist}
, { eDistSel, eShort, &UISettings::IncKomootAlertDist, eDistSel}

, { eExit, eLong, &UISettings::LeaveSettings, eDim}
})
{

    display_led.period_ms(1);  
    display_led = (float)display_brightness_ / 10.0f;
}

void UISettings::LeaveSettings()
{
    uimode_->SetUiMode(IUIMode::eCsc);
}

void UISettings::LongPress()
{
    HandleEvent(eLong);
    Draw();
    //uimode_->SetUiMode(IUIMode::eCsc);
}

void UISettings::ShortPress()
{
    //IncDislayBrightness();
    HandleEvent(eShort);
    Draw();
}


void UISettings::IncDislayBrightness()
{
    display_brightness_ = (display_brightness_ < 10) ? (display_brightness_ + 1) : 0;
    display_led = (float)display_brightness_ / 10.0f;
    //::printf("IncDislayBrightness()\n");
}

void UISettings::IncDislayTime()
{
    display_time_ = (display_time_ < 10) ? (display_time_ + 1) : 0;
    //::printf("IncDislayTime()\n");
}

void UISettings::IncKomootAlertDist()
{
    komoot_alert_dist_ = (komoot_alert_dist_ < 10) ? (komoot_alert_dist_ + 1) : 0;
    //::printf("IncKomootAlertDist()\n");
}


void UISettings::Draw()
{
    char c;
    char str[10];
    uint16_t len = sprintf(str, "Bat %i", csc_bat_);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(0xFFFF);
    tft_->WriteStringLen(0, 5, 80, str, len, 0, GFX::eCenter);

    c = (sm_state_==eDimSel) ? '*' : ((sm_state_==eDim) ? '>' : ' ');
    len = sprintf(str, "%cDim %i", c, display_brightness_);
    tft_->WriteStringLen(0, 30, 80, str, len, 0, GFX::eCenter);

    c = (sm_state_==eTimeSel) ? '*' : ((sm_state_==eTime) ? '>' : ' ');
    len = sprintf(str, "%cTime %i", c, display_time_);
    tft_->WriteStringLen(0, 55, 80, str, len, 0, GFX::eCenter);

    c = (sm_state_==eDistSel) ? '*' : ((sm_state_==eDist) ? '>' : ' ');
    len = sprintf(str, "%cDist %i", c, komoot_alert_dist_);
    tft_->WriteStringLen(0, 80, 80, str, len, 0, GFX::eCenter);

    c = ((sm_state_==eExit) ? '>' : ' ');
    len = sprintf(str, "%cExit", c);
    tft_->WriteStringLen(0, 105, 80, str, len, 0, GFX::eCenter);

}

void UISettings::UpdateBat(uint8_t val)
{
    csc_bat_ = val;
}

void UISettings::HandleEvent(eSmEvent ev)
{
    for (size_t i=0; i<(sizeof(setting_sm)/sizeof(setting_sm[0])); i++) {
        if ((sm_state_ == setting_sm[i].state) && (ev == setting_sm[i].event)) {
            sm_state_ = setting_sm[i].next;
            if (NULL != setting_sm[i].func) {
                (this->*setting_sm[i].func)();
            }
            //::printf("next %d\n", sm_state_);
            break;
        }
    }
}