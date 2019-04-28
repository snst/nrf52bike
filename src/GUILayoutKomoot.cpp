#include "GUILayoutKomoot.h"
#include "icons.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"

GUILayoutKomoot::GUILayoutKomoot(Adafruit_ST7735 & tft)
: tft_(tft)
{}


void GUILayoutKomoot::UpdateSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len)
{
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    //tft_.write_chars(0, 130, 80, str, len);
}


void GUILayoutKomoot::UpdateKomootDirection(uint8_t dir)
{
    const uint8_t *ptr = GetNavIcon(dir);
    if (ptr)
    {
        tft_.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
    }
}


void GUILayoutKomoot::UpdateKomootDistanceStr(uint32_t distance, const char* str, uint8_t len)
{
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.write_chars(0, 130, 80, str, len);
}


void GUILayoutKomoot::UpdateKomootStreet(uint8_t *street)
{
    /*
    tft_.setFont(NULL);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.setCursor(0, 120);
    tft_.printf((const char*)street);*/
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.WriteString(0,80,80,80,(char*)street);
}
