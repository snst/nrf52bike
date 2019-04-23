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
    virtual void UpdateDirection(uint8_t dir);
    virtual void UpdateDistance(uint32_t distance);
    virtual void UpdateStreet(uint8_t* street);

  protected:
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
    //PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

    uint8_t csc_bat_;
    uint16_t csc_speed_;
    uint16_t csc_cadence_;
    uint8_t komoot_dir_;
    uint32_t komoot_dist_;
    uint8_t komoot_street_[MAX_KOMOOT_STREET_LEN];

    void ShowValue(uint8_t x, uint8_t y, uint32_t value);

};

#endif