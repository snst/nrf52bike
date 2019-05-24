#ifndef UIMAIN_H_
#define UIMAIN_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IUICsc.h"
#include "IUIKomoot.h"
#include "IUILog.h"
#include "UISettings.h"
#include "IUIMode.h"
#include "IBikeComputer.h"

class UIMain : public IUICsc, public IUIKomoot, public IUILog
{
public:
  UIMain(GFX *tft, events::EventQueue &event_queue);
  virtual void Update(const IUICsc::CscData_t &data, bool force);
  virtual void Update(const IUIKomoot::KomootData_t &data, bool force);
  virtual void UpdateCscBat(uint8_t val);
  virtual void Log(const char *str);
  virtual void UpdateCscConnState(ConState_t state);
  void SetOperational();
  void SetUiMode(IUIMode::eUiMode_t mode);
  void TouchDown();
  void TouchUp();
  void SetBacklightOn();
  void SetBacklightOff();
  void SetBacklightBrightness(uint8_t val);

  //protected:
  GFX *tft_;
  events::EventQueue &event_queue_;

  IUIMode::eUiMode_t gui_mode_;

  void DrawCscConnState();
  void DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color = 0xFFFF);
  void DrawCadence(uint16_t x, uint16_t y, uint16_t cadence);
  void DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color = 0xFFFF);
  void DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color = 0xFFFF);
  void DrawKomootDistance(const IUIKomoot::KomootData_t &data);
  void DrawKomootStreet(const IUIKomoot::KomootData_t &data);
  void DrawKomootDirection(const IUIKomoot::KomootData_t &data);
  void SetCadenceColor(uint16_t cadence);
  void DrawKomootDistanceBar(const IUIKomoot::KomootData_t &data);
  uint16_t GetKomootDistanceBarColor(uint16_t distance_m);
  uint16_t GetKomootDirectionColor(uint16_t distance_m);
  void LongPress();
  void DrawSettings();
  uint8_t komoot_view_;
  uint8_t last_distance_bar_;
  uint16_t last_direction_color_;
  bool enable_komoot_switch_;
  bool switched_to_csc_;
  uint32_t touch_down_ms_;
  int longpress_id_;
  bool longpress_handled_;
  void IncBrightnessDisplayOn();
  void IncBrightnessDisplayOff();
  int led_event_id_;
  bool ignore_touch_up_;
  bool switched_to_komoot_100_;
  bool switched_to_komoot_500_;

  IUICsc::CscData_t last_csc_;
  IUIKomoot::KomootData_t last_komoot_;
  UISettings uisettings_;
  ConState_t csc_conn_state_;
  int csc_watchdog_event_id_;
  void OnCscWatchdog();
  IBikeComputer *bike_computer_;
  void SetBikeComputer(IBikeComputer *bike_computer);
};

#endif