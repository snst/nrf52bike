#include "BikeGUI.h"
//#include "FreeSansBold24pt7b.h"
//#include "Open_Sans_Condensed_Bold.h"
#include "digital7.h"
#include "gfxfont.h"
#include "icons.h"
#include "val.h"
#include "tracer.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"

#define Y_SPEED 0u
#define Y_AVERAGE_SPEED (Y_SPEED + 40u)
#define Y_CADENCE (Y_AVERAGE_SPEED + 40u)
#define Y_TRAVEL_DISTANCE (Y_CADENCE + 26u)
#define Y_TRAVEL_TIME (Y_TRAVEL_DISTANCE + 26u)

BikeGUI::BikeGUI()
    : csc_is_riding_(false), csc_travel_time_riding_(false), csc_bat_(0xFF), csc_speed_kmhX10(0xFFFF), csc_cadence_(0xFFFF), csc_average_speed_kmhX10(0xFFFF), csc_total_distance_cm(0xFFFFFFFF), csc_travel_time_sec(0xFFFFFFFF)
{
    tft.initR(INITR_MINI160x80);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.setCursor(0, 5);
    tft.setTextWrap(true);
    tft.setFont(NULL);
}

void BikeGUI::ShowSpeed(uint8_t x, uint8_t y, uint16_t val)
{
    char str[5];
    uint8_t n = 0;
    tft.setFont(&Open_Sans_Condensed_Bold_49);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    if (val <= 999)
    {
        n = sprintf(str, "%i.%i", val / 10, val % 10);
    }
    tft.write_chars(x, y, 80, str, n);
}

void BikeGUI::UpdateSpeed(uint16_t speed_kmhX10)
{
    if (IsUint16Updated(csc_speed_kmhX10, speed_kmhX10))
    {
        INFO("~BikeGUI::UpdateSpeed() => %u\r\n", speed_kmhX10);
        ShowSpeed(0, Y_SPEED, speed_kmhX10);
    }
}

void BikeGUI::UpdateAverageSpeed()
{
    uint16_t average_kmhX10 = 0;
    if (csc_travel_time_sec > 0.0f)
    {
        average_kmhX10 = (uint16_t)(0.36f * csc_total_distance_cm / csc_travel_time_sec);
    }

    if (IsUint16Updated(csc_average_speed_kmhX10, average_kmhX10))
    {
        INFO("~BikeGUI::UpdateAverageSpeed() => %u\r\n", average_kmhX10);
        ShowSpeed(0, Y_AVERAGE_SPEED, average_kmhX10);
    }
}

void BikeGUI::UpdateIsRiding(bool active)
{
    csc_is_riding_ = active;
    INFO("UpdateIsRiding %d-%d\r\n", csc_travel_time_riding_, csc_is_riding_);
}

void BikeGUI::UpdateTravelDistance(uint32_t distance_cm)
{
    if (IsUint32Updated(csc_total_distance_cm, distance_cm))
    {
        INFO("~BikeGUI::UpdateTravelDistance() => %u\r\n", distance_cm);
        tft.setFont(&Open_Sans_Condensed_Bold_31);
        tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
        char str[10] = {0};
        uint8_t n = sprintf(str, "%i.%i", distance_cm / 100, (distance_cm % 100) / 10);
        tft.write_chars(0, Y_TRAVEL_DISTANCE, 80, str, n);
    }
}

void BikeGUI::UpdateTravelTime(uint32_t time_sec)
{
    // bit or!!
    if (IsUint32Updated(csc_travel_time_sec, time_sec) | IsBoolUpdated(csc_travel_time_riding_, csc_is_riding_))
    {
        INFO("~BikeGUI::UpdateTravelTime() => %u\r\n", time_sec);
        tft.setTextColor(csc_is_riding_ ? Adafruit_ST7735::Color565(255, 255, 255) : Adafruit_ST7735::Color565(255, 50, 0));
        tft.setFont(&Open_Sans_Condensed_Bold_31);
        char str[10] = {0};
        uint8_t n = sprintf(str, "%i:%.2i", time_sec / 60, time_sec % 60);
        tft.write_chars(0, Y_TRAVEL_TIME, 80, str, n);
    }

    UpdateAverageSpeed();
}

void BikeGUI::UpdateCadence(uint16_t cadence)
{
    if (IsUint16Updated(csc_cadence_, cadence))
    {
        INFO("~BikeGUI::UpdateCadence() => %u\r\n", cadence);
        if (cadence >= 100 && cadence <= 110)
            tft.setTextColor(Adafruit_ST7735::Color565(0, 255, 0));
        else
            tft.setTextColor(Adafruit_ST7735::Color565(255, 100, 0));

        tft.setFont(&Open_Sans_Condensed_Bold_31);
        char str[10] = {0};
        uint8_t n = sprintf(str, "%i", cadence);
        tft.write_chars(0, Y_CADENCE, 80, str, n);
    }
}

void BikeGUI::UpdateKomootDirection(uint8_t dir)
{
    if (IsUint8Updated(komoot_direction_, dir))
    {
        const uint8_t *ptr = GetNavIcon(dir);
        if (ptr)
        {
            INFO("~BikeGUI::UpdateKomootDirection() => %u\r\n", dir);
            tft.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
        }
    }
}

void BikeGUI::UpdateKomootDistance(uint32_t distance)
{
    if (IsUint32Updated(komoot_distance_, distance))
    {
        INFO("~BikeGUI::UpdateKomootDistance() => %u\r\n", distance);
    }
}

void BikeGUI::UpdateKomootStreet(uint8_t *street)
{
    if (IsStringUpdated(komoot_street_, street, sizeof(komoot_street_)))
    {
        INFO("~BikeGUI::UpdateKomootStreet() => %s\r\n", street);
    }
}

void BikeGUI::Log(const char *str)
{
    tft.printf("%s", str);
}

void BikeGUI::Operational()
{
    tft.fillScreen(ST77XX_BLACK);
    UpdateSpeed(123);
    UpdateAverageSpeed();
    UpdateCadence(132);
    UpdateTravelDistance(789);
    UpdateTravelTime(135);
}