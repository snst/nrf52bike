#ifndef UISETTINGS_H_
#define UISETTINGS_H_

#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IUIMode.h"

typedef enum eSmState {
    eBat,
    eDim,
    eTime,
    eDist,
    eExit,
} eSmState_t;

typedef enum eSmEvent {
    eShort,
    eLong
} eSmEvent_t;


class UISettings;

typedef void (UISettings::*SmFunc)(void);

typedef struct SmEntry {
    eSmState state;
    const char* name;
    eSmEvent event;
    SmFunc func;
    eSmState next;
} SmEntry_t;

class UISettings
{
   
    public:
    UISettings(GFX* tft, IUIMode* mode);
    void LongPress();
    void ShortPress();
    void IncDislayBrightness();
    void IncDislayTime();
    void IncKomootAlertDist();
    void Draw();
    void UpdateBat(uint8_t val);
    void HandleEvent(eSmEvent ev);
    void LeaveSettings();
    SmEntry_t* GetStateEntry(eSmState state);

    uint8_t csc_bat_;
    uint8_t display_brightness_;
    uint8_t display_time_;
    uint16_t komoot_alert_dist_;
    GFX* tft_;
    IUIMode* uimode_;
    eSmState_t sm_state_;
    bool edit_mode_;

    SmEntry setting_sm[5];

};

#endif