#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"


UISettings::UISettings(GFX* tft, IUIMode* mode)
: display_brightness_(5)
, display_time_(10)
, komoot_alert_dist_(400)
, uimode_(mode)
, tft_(tft)
, sm_state_(eBat)
, edit_mode_(false)
, setting_sm({
  { eBat, "Bat", eShort, NULL, eDim}
, { eDim, "Dim", eShort, &UISettings::IncDislayBrightness, eTime,}
, { eTime, "Time", eShort, &UISettings::IncDislayTime, eDist }
, { eDist, "Dist", eShort, &UISettings::IncKomootAlertDist, eExit}
, { eExit, "Exit", eShort, NULL, eBat}
})
{
}

void UISettings::LeaveSettings()
{
    sm_state_ = eDim;
    uimode_->SetUiMode(IUIMode::eCsc);
}

void UISettings::LongPress()
{
    HandleEvent(eLong);
}

void UISettings::ShortPress()
{
    HandleEvent(eShort);
}


void UISettings::IncDislayBrightness()
{
    display_brightness_ = (display_brightness_ < 10) ? (display_brightness_ + 1) : 0;
    uimode_->SetUiBrightness(display_brightness_);
    //display_led = (float)display_brightness_ / 10.0f;
    Draw();
    //::printf("IncDislayBrightness()\n");
}

void UISettings::IncDislayTime()
{
    display_time_ = (display_time_ < 30) ? (display_time_ + 2) : 2;
    Draw();
    //::printf("IncDislayTime()\n");
}

void UISettings::IncKomootAlertDist()
{
    komoot_alert_dist_ = (komoot_alert_dist_ < 500) ? (komoot_alert_dist_ + 50) : 50;
    Draw();
    //::printf("IncKomootAlertDist()\n");
}


void UISettings::Draw()
{
    tft_->fillScreen(ST77XX_BLACK);
    SmEntry_t* entry = GetStateEntry(sm_state_);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(edit_mode_ ? Adafruit_ST7735::Color565(255, 0, 0) : 0xFFFF);
    tft_->WriteStringLen(0, 15, 80, entry->name, -1, 0, GFX::eCenter);
    char str[10];
    int16_t len;
    switch (sm_state_) {
        case eBat:
        len = sprintf(str, "%i %%", csc_bat_);
        break;
        case eDim:
        len = sprintf(str, "%i", display_brightness_);
        break;
        case eTime:
        len = sprintf(str, "%i s", display_time_);
        break;
        case eDist:
        len = sprintf(str, "%i m", komoot_alert_dist_);
        break;
        default:
        len = 0;
        break;
    }

    if (0 < len) {
        tft_->setTextColor(0xFFFF);
        tft_->WriteStringLen(0, 50, 80, str, len, 0, GFX::eCenter);
    }
}

void UISettings::UpdateBat(uint8_t val)
{
    csc_bat_ = val;
}

void UISettings::HandleEvent(eSmEvent ev)
{
    SmEntry_t* entry = GetStateEntry(sm_state_);

    if (ev == eLong) {
        if (eExit == entry->state) {
            LeaveSettings();
        } else if (NULL != entry->func) {
            edit_mode_ = !edit_mode_;
            Draw();
        }
    } else {
        if (edit_mode_) {
            (this->*entry->func)();
        } else {
            sm_state_ = entry->next;
            Draw();
        }

    }

    if ((ev == eLong) && (NULL != entry->func))
    {
    } else if (ev == eShort) {
    }

    /*
    for (size_t i=0; i<(sizeof(setting_sm)/sizeof(setting_sm[0])); i++) {
        if ((sm_state_ == setting_sm[i].state) && (ev == setting_sm[i].event)) {
            sm_state_ = setting_sm[i].next;
            if (NULL != setting_sm[i].func) {
                (this->*setting_sm[i].func)();
            }
            //::printf("next %d\n", sm_state_);
            break;
        }
    }*/
}

SmEntry_t* UISettings::GetStateEntry(eSmState state)
{
    SmEntry_t* ret = NULL;
    for (size_t i=0; i<(sizeof(setting_sm)/sizeof(setting_sm[0])); i++) {
        if (state == setting_sm[i].state) {
            ret = &setting_sm[i];
            break;
        }
    }
    return ret;
}