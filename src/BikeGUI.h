#ifndef BIKE_GUI_H_
#define BIKE_GUI_H_

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include <Adafruit_ST7735.h>
#include "ICscGUI.h"
#include "IKomootGUI.h"
#include "val.h"

class BikeGUI : public ICscGUI, public IKomootGUI
{
public:
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
  
  uint8_t komoot_direction_;
  uint32_t komoot_distance_;
  uint8_t komoot_street_[MAX_KOMOOT_STREET_LEN];

  void ShowSpeed(uint8_t x, uint8_t y, uint16_t value);
  void UpdateAverageSpeed();

};

#endif