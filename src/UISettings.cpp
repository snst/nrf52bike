#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"
#include "fds.h"
#include "tracer.h"

#define FILE_ID 0x1111
#define REC_KEY 0x2222


static void fds_evt_handler(fds_evt_t const *const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
    case FDS_EVT_INIT:
        INFO("FDS_EVT_INIT=0x%x\r\n", p_fds_evt->result);
        break;
    case FDS_EVT_WRITE:
        INFO("FDS_EVT_WRITE=0x%x\r\n", p_fds_evt->result);
        break;
    case FDS_EVT_UPDATE:
        INFO("FDS_EVT_UPDATE=0x%x\r\n", p_fds_evt->result);
        break;
    default:
        break;
    }
}

UISettings::UISettings(GFX *tft, IUIMode *mode)
    : uimode_(mode), tft_(tft)
    , sm_state_(eBat)
    , edit_mode_(false)
    , setting_sm({
        {eBat, "Bat", false, NULL, eLedOn}
      , {eLedOn, "LedOn", true, &UISettings::IncBrightnessDisplayOn, eLedOff}
      , {eLedOff, "LedOff", true, &UISettings::IncBrightnessDisplayOff, eTime}
      , {eTime, "Time", true, &UISettings::IncDislayTime, eDist}
      , {eDist, "Dist", true, &UISettings::IncKomootAlertDist, eSave}
      , {eSave, "Save", true, &UISettings::SaveSettings, eExit}
      , {eExit, "Exit", false, &UISettings::LeaveSettings, eBat}
      })
{
    settings_.display_brightness_on = 7;
    settings_.display_brightness_off = 9;
    settings_.display_time = 10;
    settings_.komoot_alert_dist = 400;

    ret_code_t ret = fds_register(fds_evt_handler);
    if (ret == FDS_SUCCESS)
    {
        ret = fds_init();
    }
    if (ret == FDS_SUCCESS)
    {
        LoadSettings();
    }
    INFO("FDS init: 0x%x\r\n", ret);
}

void UISettings::SaveSettings()
{
    fds_flash_record_t flash_record;
    fds_record_desc_t record_desc;
    fds_find_token_t ftok = {0};
    uint32_t *data;
    ret_code_t err_code;
    bool found = false;

    //fds_file_delete(FILE_ID);
    //return;

    while (fds_record_find(FILE_ID, REC_KEY, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code == FDS_SUCCESS)
        {
            fds_record_t record;
            record.file_id = FILE_ID;
            record.key = REC_KEY;
            record.data.p_data = &settings_;
            record.data.length_words = sizeof(settings_) / sizeof(uint32_t);
            err_code = fds_record_update(&record_desc, &record);
            INFO("fds_record_update 0x%x\r\n", err_code);
            err_code = fds_record_close(&record_desc);
            found = true;
            break;
        }
    }

    if (!found)
    {
        fds_record_t        record;
        fds_record_desc_t   record_desc2 = {0};
        record.file_id = FILE_ID;
        record.key = REC_KEY;
        record.data.p_data = &settings_;
        record.data.length_words = 1;
        err_code = fds_record_write(&record_desc2, &record);
        INFO("fds_record_write 0x%x\r\n", err_code);
    }

    edit_mode_ = false;
}

void UISettings::LoadSettings()
{
    fds_flash_record_t flash_record;
    fds_record_desc_t record_desc;
    fds_find_token_t ftok = {0};
    uint32_t *data;
    uint32_t err_code;
    INFO("Search settings..\r\n");
    while (fds_record_find(FILE_ID, REC_KEY, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code == FDS_SUCCESS)
        {
            memcpy(&settings_, flash_record.p_data, sizeof(settings_));
            err_code = fds_record_close(&record_desc);
            INFO("Load settings: dispayOn=%u, displayOff=%u, time=%u, dist=%u\r\n", settings_.display_brightness_on, settings_.display_brightness_off , settings_.display_time, settings_.komoot_alert_dist);
            break;
        }
    }
}

void UISettings::LeaveSettings()
{
    sm_state_ = eLedOn;
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

void UISettings::IncBrightnessDisplayOn()
{
    settings_.display_brightness_on = (settings_.display_brightness_on < 10) ? (settings_.display_brightness_on + 1) : 0;
    uimode_->SetUiBrightness(settings_.display_brightness_on);
}

void UISettings::IncBrightnessDisplayOff()
{
    settings_.display_brightness_off = (settings_.display_brightness_off < 10) ? (settings_.display_brightness_off + 1) : 0;
    uimode_->SetUiBrightness(settings_.display_brightness_off);
}

void UISettings::IncDislayTime()
{
    settings_.display_time = (settings_.display_time < 30) ? (settings_.display_time + 2) : 2;
}

void UISettings::IncKomootAlertDist()
{
    settings_.komoot_alert_dist = (settings_.komoot_alert_dist < 500) ? (settings_.komoot_alert_dist + 50) : 50;
}

void UISettings::Draw()
{
    tft_->fillScreen(ST77XX_BLACK);
    SmEntry_t *entry = GetStateEntry(sm_state_);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(edit_mode_ ? Adafruit_ST7735::Color565(255, 0, 0) : 0xFFFF);
    tft_->WriteStringLen(0, 15, 80, entry->name, -1, 0, GFX::eCenter);
    char str[10];
    int16_t len;
    switch (sm_state_)
    {
    case eBat:
        len = sprintf(str, "%i %%", csc_bat_);
        break;
    case eLedOn:
        len = sprintf(str, "%i", settings_.display_brightness_on);
        break;
    case eLedOff:
        len = sprintf(str, "%i", settings_.display_brightness_off);
        break;
    case eTime:
        len = sprintf(str, "%i s", settings_.display_time);
        break;
    case eDist:
        len = sprintf(str, "%i m", settings_.komoot_alert_dist);
        break;
    default:
        len = 0;
        break;
    }

    if (0 < len)
    {
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
    SmEntry_t *entry = GetStateEntry(sm_state_);

    if (NULL != entry) 
    {
        if (ev == eLong)
        {
            if (entry->editable) {
                edit_mode_ = !edit_mode_;
                Draw();
            }
            else if (NULL != entry->func)
            {
                (this->*entry->func)();
            }
        }
        else
        {
            if (edit_mode_)
            {
                (this->*entry->func)();
            }
            else
            {
                sm_state_ = entry->next;
            }
            Draw();
        }
    }
}

SmEntry_t *UISettings::GetStateEntry(eSmState state)
{
    SmEntry_t *ret = NULL;
    for (size_t i = 0; i < (sizeof(setting_sm) / sizeof(setting_sm[0])); i++)
    {
        if (state == setting_sm[i].state)
        {
            ret = &setting_sm[i];
            break;
        }
    }
    return ret;
}