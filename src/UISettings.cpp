#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_22.h"
#include "common.h"
#include "fds.h"
#include "tracer.h"
#include "IBikeComputer.h"

#define FILE_ID 0x1111
#define REC_KEY 0x2222

AnalogIn ain(p5);

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

UISettings::UISettings(GFX *tft)
    : tft_(tft),                                                                          //
      sm_state_(eStatus),                                                                 //
      edit_mode_(false),                                                                  //
      setting_sm({{eStatus, "Status", false, &UISettings::LeaveSettings, eLedOn},         //
                  {eLedOn, "LedOn", true, &UISettings::IncBrightnessDisplayOn, eLedOff},  //
                  {eLedOff, "LedOff", true, &UISettings::IncBrightnessDisplayOff, eTime}, //
                  {eTime, "Time", true, &UISettings::IncDislayTime, eDist},               //
                  {eDist, "Dist", true, &UISettings::IncKomootAlertDist, eAutoSwitch},    //
                  {eAutoSwitch, "AMode", true, &UISettings::IncAutoSwitch, eLightUp},     //
                  {eLightUp, "Light", true, &UISettings::IncLightUp, eToggleSec},         //
                  {eToggleSec, "Tsec", true, &UISettings::IncToggleSec, eConnect},        //
                  {eConnect, "Conn", true, &UISettings::Connect, eSave},                  //
                  {eSave, "Save", true, &UISettings::SaveSettings, eExit},                //
                  {eExit, "Exit", false, &UISettings::LeaveSettings, eStatus}})
{
    settings_.display_brightness_on = 7;
    settings_.display_brightness_off = 9;
    settings_.display_time = 10;
    settings_.komoot_alert_dist = 400;
    settings_.toggle_sec = 4;

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
        fds_record_t record;
        fds_record_desc_t record_desc2 = {0};
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
            INFO("Load settings: dispayOn=%u, displayOff=%u, time=%u, dist=%u\r\n", settings_.display_brightness_on, settings_.display_brightness_off, settings_.display_time, settings_.komoot_alert_dist);
            break;
        }
    }
}

void UISettings::LeaveSettings()
{
    sm_state_ = eInactive;
    bike_computer_->SetUiMode(IUIMode::eCsc);
}

void UISettings::HandleLongPress()
{
    HandleEvent(eLong);
}

void UISettings::ShortPress()
{
    HandleEvent(eShort);
}

void UISettings::IncAutoSwitch()
{
    settings_.auto_switch = settings_.auto_switch ? 0 : 1;
}

void UISettings::IncLightUp()
{
    settings_.light_up = settings_.light_up ? 0 : 1;
}

void UISettings::IncBrightnessDisplayOn()
{
    settings_.display_brightness_on = (settings_.display_brightness_on < 10) ? (settings_.display_brightness_on + 1) : 0;
    bike_computer_->SetBacklightBrightness(settings_.display_brightness_on);
}

void UISettings::IncBrightnessDisplayOff()
{
    settings_.display_brightness_off = (settings_.display_brightness_off < 10) ? (settings_.display_brightness_off + 1) : 0;
    bike_computer_->SetBacklightBrightness(settings_.display_brightness_off);
}

void UISettings::IncDislayTime()
{
    settings_.display_time = (settings_.display_time < 30) ? (settings_.display_time + 2) : 2;
}

void UISettings::IncToggleSec()
{
    settings_.toggle_sec = (settings_.toggle_sec < 10) ? (settings_.toggle_sec + 1) : 1;
}

void UISettings::IncKomootAlertDist()
{
    settings_.komoot_alert_dist = (settings_.komoot_alert_dist < 500) ? (settings_.komoot_alert_dist + 50) : 50;
}

void UISettings::Connect()
{
    bike_computer_->Connect(BC::eCsc, 500);
    bike_computer_->Connect(BC::eKomoot, 1500);
    edit_mode_ = false;
}

void UISettings::UpdateBat()
{
    if (eStatus == sm_state_)
    {
        float volt;
        uint8_t percent;
        CalculateBat(volt, percent);

        tft_->setFont(&Open_Sans_Condensed_Bold_22);
        tft_->setTextColor(0xFFFF);
        char str[10];
        sprintf(str, "bc %u %%", percent);
        tft_->WriteStringLen(0, 70, 80, str, -1, 0, GFX::eCenter);

        bike_computer_->GetEventQueue()->call_in(1000, mbed::callback(this, &UISettings::UpdateBat));
    }
}

void UISettings::Draw()
{
    tft_->fillScreen(ST77XX_BLACK);
    SmEntry_t *entry = GetStateEntry(sm_state_);
    tft_->setFont(&Open_Sans_Condensed_Bold_22);
    tft_->setTextColor(edit_mode_ ? Adafruit_ST7735::Color565(255, 0, 0) : 0xFFFF);
    tft_->WriteStringLen(0, 15, 80, entry->name, -1, 0, GFX::eCenter);
    tft_->setTextColor(0xFFFF);

    char str[10];
    char *str_val = str;
    int16_t len;
    switch (sm_state_)
    {
    case eStatus:
    {
        sprintf(str, "csc %i %%", bike_computer_->GetCscBat());
        tft_->WriteStringLen(0, 40, 80, str, -1, 0, GFX::eCenter);
        //        sprintf(str, "bc %u %%", percent);
        //        tft_->WriteStringLen(0, 70, 80, str, -1, 0, GFX::eCenter);
        uint16_t max = bike_computer_->GetCscData()->max_speed_kmhX10;
        sprintf(str, "max %i.%i", max / 10, max % 10);
        tft_->WriteStringLen(0, 100, 80, str, -1, 0, GFX::eCenter);
        sprintf(str, "cad %u", bike_computer_->GetCscData()->average_cadence);
        tft_->WriteStringLen(0, 130, 80, str, -1, 0, GFX::eCenter);
        str_val = NULL;
        UpdateBat();
    }
    break;
    case eLedOn:
        sprintf(str, "%i", settings_.display_brightness_on);
        break;
    case eLedOff:
        sprintf(str, "%i", settings_.display_brightness_off);
        break;
    case eTime:
        sprintf(str, "%i s", settings_.display_time);
        break;
    case eDist:
        sprintf(str, "%i m", settings_.komoot_alert_dist);
        break;
    case eAutoSwitch:
        sprintf(str, "%s", settings_.auto_switch ? "on" : "off");
        break;
    case eLightUp:
        sprintf(str, "%s", settings_.light_up ? "on" : "off");
        break;
    case eToggleSec:
        sprintf(str, "%i s", settings_.toggle_sec);
        break;
    case eConnect:
        sprintf(str, "%u", bike_computer_->GetCscDisconnects());
        break;
    default:
        str_val = NULL;
        break;
    }

    if (NULL != str_val)
    {
        tft_->WriteStringLen(0, 40, 80, str_val, -1, 0, GFX::eCenter);
    }
}

void UISettings::HandleEvent(eSmEvent ev)
{
    SmEntry_t *entry = GetStateEntry(sm_state_);

    if (NULL != entry)
    {
        if (ev == eLong)
        {
            if (entry->editable)
            {
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

void UISettings::SetBikeComputer(IBikeComputer *bike_computer)
{
    bike_computer_ = bike_computer;
}

void UISettings::SystemOff()
{
    //sd_power_system_off();
}

void UISettings::CalculateBat(float &volt, uint8_t &percent)
{
    float raw = ain.read();
    volt = 4.2f / 0.177f * raw;
    float per = (100.0 / 0.7f) * (volt - 3.5f);
    percent = per > 0.0f ? (uint8_t)per : 0;
    INFO("Bat raw=%f, volt=%f, percent=%u\r\n", raw, volt, percent);
}

void UISettings::Activate()
{
    sm_state_ = eStatus;
    Draw();
}