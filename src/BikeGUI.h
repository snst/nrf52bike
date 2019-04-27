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
  virtual void UpdateSpeed(uint16_t val);
  virtual void UpdateCadence(uint16_t val);
  virtual void UpdateTravelDistance(uint16_t val);
  virtual void UpdateTravelTime(uint16_t val);
  virtual void UpdateAverageSpeed(uint16_t val);
  virtual void UpdateDirection(uint8_t dir);
  virtual void UpdateDistance(uint32_t distance);
  virtual void UpdateStreet(uint8_t *street);
  virtual void Log(const char *str);
  void Operational();

protected:
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
  //PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

  uint8_t csc_bat_;
  uint16_t csc_speed_;
  uint16_t csc_cadence_;
  uint16_t csc_average_speed_;
  uint16_t csc_distance_;
  uint16_t csc_travel_time_;
  
  uint8_t komoot_direction_;
  uint32_t komoot_distance_;
  uint8_t komoot_street_[MAX_KOMOOT_STREET_LEN];

  void ShowValue(uint8_t x, uint8_t y, uint32_t value);
  void SetCursor(uint8_t x, uint8_t y, uint8_t size);
  void ShowDigit(uint8_t ch, uint8_t x, uint8_t y, int8_t digit);
  const uint8_t* GetDigit(int8_t digit, const unsigned char* data, uint16_t offset);
  void ShowSpeed(uint8_t x, uint8_t y, uint16_t value);
  void ShowDot(uint8_t x, uint8_t y);

};

#endif