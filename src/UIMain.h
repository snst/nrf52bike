#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "ISinkCsc.h"
#include "ISinkKomoot.h"

class UIMain : public ISinkCsc, public ISinkKomoot
{
public:
  enum eGuiMode_t
  {
    eStartup,
    eCsc,
    eKomoot,
    eHybrid
  };
  UIMain();
  virtual void Update(const ISinkCsc::CscData_t &data);
  virtual void Update(const ISinkKomoot::KomootData_t &data);
  virtual void Log(const char *str);
  void Operational();
  void SetGuiMode(eGuiMode_t mode);

protected:
  Adafruit_ST7735 tft_ = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
  //PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

  uint8_t csc_bat_;
  eGuiMode_t gui_mode_;

  void DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color = 0xFFFF);
  void DrawCadence(uint16_t y, uint16_t cadence);
  void DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color = 0xFFFF);
  void DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color = 0xFFFF);
};

#endif