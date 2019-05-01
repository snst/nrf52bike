#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IUIMain.h"
#include "val.h"
#include "IUIMain.h"
#include "UICsc.h"
#include "UIKomoot.h"
#include "IDataCsc.h"
#include "IDataKomoot.h"

class UIMain : public IDataCsc, public IDataKomoot
{
public:
  enum eGuiMode_t { eStartup, eCsc, eKomoot, eHybrid };
  UIMain();
  virtual void Update(const CscData_t& data);
  /*
  virtual void UpdateSpeed(uint16_t speed);
  virtual void UpdateCadence(uint16_t cadence);
  virtual void UpdateTravelDistance(uint32_t distance_cm);
  virtual void UpdateTravelTime(uint32_t time_sec);
  virtual void UpdateIsRiding(bool active);
  */
  virtual void UpdateKomootDirection(uint8_t dir);
  virtual void UpdateKomootDistance(uint32_t distance);
  virtual void UpdateKomootStreet(uint8_t *street);
  virtual void Log(const char *str);
  void Operational();
  void SetGuiMode(eGuiMode_t mode);

protected:
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
  //PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

  uint8_t csc_bat_;
  CscData_t csc_data_;
  eGuiMode_t gui_mode_;
  
  uint8_t komoot_direction_;
  uint32_t komoot_distance_;
  uint8_t komoot_street_[MAX_KOMOOT_STREET_LEN];

  void UpdateAverageSpeed();
  uint8_t FormatSpeed(char* str, uint16_t speed_kmhX10);
  IUIMain* layout_;
  UICsc layout_csc_;
  UIKomoot layout_komoot_;
};

#endif