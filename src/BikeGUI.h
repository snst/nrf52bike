#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "IGUILayout.h"
#include "val.h"
#include "IGUILayout.h"
#include "GUILayoutCSC.h"
#include "GUILayoutKomoot.h"
#include "ICscGUI.h"
#include "IKomootGUI.h"

class BikeGUI : public ICscGUI, public IKomootGUI
{
public:
  enum eGuiMode_t { eStartup, eCsc, eKomoot, eHybrid };
  BikeGUI();
  virtual void UpdateSpeed(uint16_t speed);
  virtual void UpdateCadence(uint16_t cadence);
  virtual void UpdateTravelDistance(uint32_t distance_cm);
  virtual void UpdateTravelTime(uint32_t time_sec);
  virtual void UpdateIsRiding(bool active);
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
  uint16_t csc_speed_kmhX10;
  uint16_t csc_cadence_;
  uint16_t csc_average_speed_kmhX10;
  uint32_t csc_total_distance_cm;
  uint32_t csc_travel_time_sec;
  bool csc_is_riding_;
  bool csc_travel_time_riding_;
  eGuiMode_t gui_mode_;
  
  uint8_t komoot_direction_;
  uint32_t komoot_distance_;
  uint8_t komoot_street_[MAX_KOMOOT_STREET_LEN];

  void UpdateAverageSpeed();
  uint8_t FormatSpeed(char* str, uint16_t speed_kmhX10);
  IGUILayout* layout_;
  GUILayoutCSC layout_csc_;
  GUILayoutKomoot layout_komoot_;
};

#endif