#include "UISettings.h"
#include "gfxfont.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"
#include "fds.h"
#include "tracer.h"

// Simple event handler to handle errors during initialization.
static void fds_evt_handler(fds_evt_t const *const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
    case FDS_EVT_INIT:
        if (p_fds_evt->result != FDS_SUCCESS)
        {
            // Initialization failed.
        }
        else
        {
            INFO("FDS_EVT_INIT OK\r\n");
        }
        break;
    case FDS_EVT_WRITE:
        if (p_fds_evt->result != FDS_SUCCESS)
        {
            INFO("FDS_EVT_WRITE ERR 0x%x\r\n", p_fds_evt->result);
        }
        break;
    case FDS_EVT_UPDATE:
        if (p_fds_evt->result != FDS_SUCCESS)
        {
            INFO("FDS_EVT_UPDATE ERR 0x%x\r\n", p_fds_evt->result);
        }
        break;

    default:
        break;
    }
}

UISettings::UISettings(GFX *tft, IUIMode *mode)
    : uimode_(mode), tft_(tft), sm_state_(eBat), edit_mode_(false), setting_sm({{eBat, "Bat", eShort, NULL, eDim}, {
                                                                                                                       eDim,
                                                                                                                       "Dim",
                                                                                                                       eShort,
                                                                                                                       &UISettings::IncDislayBrightness,
                                                                                                                       eTime,
                                                                                                                   },
                                                                                {eTime, "Time", eShort, &UISettings::IncDislayTime, eDist},
                                                                                {eDist, "Dist", eShort, &UISettings::IncKomootAlertDist, eSave},
                                                                                {eSave, "Save", eShort, &UISettings::SaveSettings, eExit},
                                                                                {eExit, "Exit", eShort, NULL, eBat}})
{
    settings_.display_brightness = 5;
    settings_.display_time = 10;
    settings_.komoot_alert_dist = 400;

    ret_code_t ret = fds_register(fds_evt_handler);
    if (ret != FDS_SUCCESS)
    {
        INFO("fds_register\r\n");
    }
    ret = fds_init();
    if (ret != FDS_SUCCESS)
    {
        INFO("fds_init\r\n");
    }
    INFO("FS OK\r\n");
    LoadSettings();
}

#define FILE_ID 0x1111
#define REC_KEY 0x2222

/*
void UISettings::SaveSettings()
{
    static uint32_t const m_deadbeef = 0xDEADBEEF;
    fds_record_t        record;
    fds_record_desc_t   record_desc = {0};
    static uint32_t fdata = 0xdeadbeef;

    record.file_id              = FILE_ID;
    record.key                  = REC_KEY;
    record.data.p_data          = &fdata;
    record.data.length_words    =  1;
    //fds_file_delete(FILE_ID);
    ret_code_t ret = fds_record_write(&record_desc, &record);
    //ret_code_t ret = fds_record_update(&record_desc, &record);

    
        
    if (ret != FDS_SUCCESS)
    {
        INFO("UPDATE FAILED\r\n");
    } else {
        INFO("UPDATE OK\r\n");
    }
}*/

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
            record.data.length_words = 1;
            err_code = fds_record_update(&record_desc, &record);
            INFO("fds_record_update 0x%x\r\n", err_code);

            err_code = fds_record_close(&record_desc);
            if (err_code != FDS_SUCCESS)
            {
                INFO("CLOSE FAILED\r\n");
            }
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
    Draw();
}

void UISettings::LoadSettings()
{
    fds_flash_record_t flash_record;
    fds_record_desc_t record_desc;
    fds_find_token_t ftok = {0}; //Important, make sure you zero init the ftok token
    uint32_t *data;
    uint32_t err_code;
    INFO("Start searching... \r\n");
    // Loop until all records with the given key and file ID have been found.
    while (fds_record_find(FILE_ID, REC_KEY, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code == FDS_SUCCESS)
        {
            memcpy(&settings_, flash_record.p_data, sizeof(settings_));
            err_code = fds_record_close(&record_desc);
            INFO("LS, dim=%u, time=%u, dist=%u\r\n", settings_.display_brightness, settings_.display_time, settings_.komoot_alert_dist);
            //break;
        }
        /*
				INFO("Found Record ID = %d, len=%u\r\n",record_desc.record_id, flash_record.p_header->length_words);
				INFO("Data = ");
				data = (uint32_t *) flash_record.p_data;
				for (uint8_t i=0;i<flash_record.p_header->length_words;i++)
				{
					INFO("0x%8x ",data[i]);
				}
				INFO("\r\n");
				// Access the record through the flash_record structure.
				// Close the record when done.
				err_code = fds_record_close(&record_desc);
				if (err_code != FDS_SUCCESS)
				{
                    INFO("CLOSE FAILED\r\n");
					return ;	
				}
                */
    }
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
    settings_.display_brightness = (settings_.display_brightness < 10) ? (settings_.display_brightness + 1) : 0;
    uimode_->SetUiBrightness(settings_.display_brightness);
    //display_led = (float)display_brightness_ / 10.0f;
    Draw();
    //::printf("IncDislayBrightness()\n");
}

void UISettings::IncDislayTime()
{
    settings_.display_time = (settings_.display_time < 30) ? (settings_.display_time + 2) : 2;
    Draw();
    //::printf("IncDislayTime()\n");
}

void UISettings::IncKomootAlertDist()
{
    settings_.komoot_alert_dist = (settings_.komoot_alert_dist < 500) ? (settings_.komoot_alert_dist + 50) : 50;
    Draw();
    //::printf("IncKomootAlertDist()\n");
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
    case eDim:
        len = sprintf(str, "%i", settings_.display_brightness);
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

    if (ev == eLong)
    {
        if (eExit == entry->state)
        {
            LeaveSettings();
        }
        else if (NULL != entry->func)
        {
            edit_mode_ = !edit_mode_;
            Draw();
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
            Draw();
        }
    }

    if ((ev == eLong) && (NULL != entry->func))
    {
    }
    else if (ev == eShort)
    {
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