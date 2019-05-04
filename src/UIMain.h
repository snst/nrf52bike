#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "ISinkCsc.h"
#include "ISinkKomoot.h"
#include "IUILog.h"

class UIMain : public ISinkCsc, public ISinkKomoot, public IUILog
{
public:
  enum eGuiMode_t
  {
    eStartup,
    eCsc,
    eKomoot,
    eHybrid
  };
  UIMain(Adafruit_ST7735& tft, events::EventQueue& event_queue);
  virtual void Update(const ISinkCsc::CscData_t &data);
  virtual void Update(const ISinkKomoot::KomootData_t &data, bool force = false);
  virtual void Log(const char *str);
  void SetOperational();
  void SetGuiMode(eGuiMode_t mode);

//protected:
  Adafruit_ST7735 tft_;
  events::EventQueue& event_queue_;

  uint8_t csc_bat_;
  eGuiMode_t gui_mode_;

  void DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color = 0xFFFF);
  void DrawCadence(uint16_t x, uint16_t y, uint16_t cadence);
  void DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color = 0xFFFF);
  void DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color = 0xFFFF);
  void DrawKomootDistance(uint16_t distance_m);
  void SetCadenceColor(uint16_t cadence);
  void SwitchUI();
  ISinkKomoot::KomootData_t komoot_data_;
  uint8_t komoot_view_;
};

#endif