#include "GUILayoutCSC.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"

#define Y_SPEED 0u
#define Y_AVERAGE_SPEED (Y_SPEED + 40u)
#define Y_CADENCE (Y_AVERAGE_SPEED + 40u)
#define Y_TRAVEL_DISTANCE (Y_CADENCE + 26u)
#define Y_TRAVEL_TIME (Y_TRAVEL_DISTANCE + 26u)

GUILayoutCSC::GUILayoutCSC(Adafruit_ST7735 & tft)
: tft_(tft)
{}


void GUILayoutCSC::UpdateSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len)
{
    tft_.setFont(&Open_Sans_Condensed_Bold_49);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.write_chars(0, Y_SPEED, 80, str, len);
}


void GUILayoutCSC::UpdateAverageSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len)
{
    tft_.setFont(&Open_Sans_Condensed_Bold_49);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.write_chars(0, Y_AVERAGE_SPEED, 80, str, len);
}


void GUILayoutCSC::UpdateCadenceStr(uint16_t cadence, const char* str, uint8_t len)
{
   if (cadence >= 100 && cadence <= 110)
        tft_.setTextColor(Adafruit_ST7735::Color565(0, 255, 0));
    else
        tft_.setTextColor(Adafruit_ST7735::Color565(255, 100, 0));

    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.write_chars(0, Y_CADENCE, 80, str, len);
}


void GUILayoutCSC::UpdateTravelDistanceStr(uint32_t distance_cm, const char* str, uint8_t len)
{
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.write_chars(0, Y_TRAVEL_DISTANCE, 80, str, len);
}


void GUILayoutCSC::UpdateTravelTimeStr(uint32_t time_sec, const char* str, uint8_t len)
{
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.write_chars(0, Y_TRAVEL_TIME, 80, str, len);
}


void GUILayoutCSC::UpdateIsRiding(bool active)
{
    if(!active) {
        tft_.fillRect(0,Y_TRAVEL_DISTANCE, 10,10, Adafruit_ST7735::Color565(255, 0, 0) );
    }
}