#ifndef UISETTINGS_H_
#define UISETTINGS_H_

#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IUIMode.h"

typedef enum eSmState
{
    eInactive,
    eStatus,
    eLedOn,
    eLedOff,
    eTime,
    eDist,
    eAutoSwitch,
    eLightUp,
    eConnect,
    eToggleSec,
    eSave,
    eExit,
} eSmState_t;

typedef enum eSmEvent
{
    eShort,
    eLong
} eSmEvent_t;

class UISettings;
class IBikeComputer;

typedef void (UISettings::*SmFunc)(void);

typedef struct SmEntry
{
    eSmState state;
    const char *name;
    bool editable;
    SmFunc func;
    eSmState next;
} SmEntry_t;


class UISettings
{
public:
    UISettings(GFX *tft);
    void HandleLongPress();
    void ShortPress();
    void IncBrightnessDisplayOn();
    void IncBrightnessDisplayOff();
    void IncDislayTime();
    void IncKomootAlertDist();
    void IncAutoSwitch();
    void IncLightUp();
    void IncToggleSec();
    void SaveSettings();
    void LoadSettings();
    void Draw();
    void SystemOff();
    void HandleEvent(eSmEvent ev);
    void LeaveSettings();
    void Connect();
    SmEntry_t *GetStateEntry(eSmState state);
    void SetBikeComputer(IBikeComputer *bike_computer);
    void CalculateBat(float &volt, uint8_t &percent);
    void UpdateBat();
    void Activate();
protected:
    GFX *tft_;
    eSmState_t sm_state_;
    bool edit_mode_;

    SmEntry setting_sm[11];

    typedef struct SettingsData
    {
        uint16_t komoot_alert_dist;
        uint8_t display_brightness_on;
        uint8_t display_brightness_off;
        uint8_t display_time;
        uint8_t auto_switch;
        uint8_t light_up;
        uint8_t toggle_sec;
    } SettingsData_t;

    IBikeComputer* bike_computer_;
public:
    SettingsData_t config_;
};

#endif