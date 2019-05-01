#include "UIMain.h"
#include "gfxfont.h"
#include "tracer.h"
#include "icons.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"

DigitalOut display_led((PinName)11);

#define Y_CSC_SPEED 0u
#define Y_CSC_AVERAGE_SPEED (Y_CSC_SPEED + 40u)
#define Y_CSC_CADENCE (Y_CSC_AVERAGE_SPEED + 40u)
#define Y_CSC_TRAVEL_DISTANCE (Y_CSC_CADENCE + 26u)
#define Y_CSC_TRAVEL_TIME (Y_CSC_TRAVEL_DISTANCE + 26u)

UIMain::UIMain()
    : csc_bat_(0xFF), gui_mode_(eStartup)
{
    tft_.initR(INITR_MINI160x80);
    tft_.fillScreen(ST77XX_BLACK);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.setCursor(0, 5);
    tft_.setTextWrap(true);
    tft_.setFont(NULL);
    display_led = 0; // enable display led
}

void UIMain::Update(const ISinkCsc::CscData_t &data)
{
    switch (gui_mode_)
    {
    case eCsc:
        if (data.filtered_speed_kmhX10_updated)
        {
            DrawSpeed(Y_CSC_SPEED, data.filtered_speed_kmhX10);
        }
        if (data.average_speed_kmhX10_updated)
        {
            DrawSpeed(Y_CSC_AVERAGE_SPEED, data.average_speed_kmhX10);
        }
        if (data.cadence_updated)
        {
            DrawCadence(Y_CSC_CADENCE, data.cadence);
        }
        if (data.trip_distance_cm_updated)
        {
            DrawDistance(Y_CSC_TRAVEL_DISTANCE, data.trip_distance_cm / 100);
        }
        if (data.trip_time_ms_updated || data.is_riding_updated)
        {
            uint16_t color = data.is_riding ? Adafruit_ST7735::Color565(255, 255, 255) : Adafruit_ST7735::Color565(255, 0, 0);
            DrawTime(Y_CSC_TRAVEL_TIME, data.trip_time_ms / 1000, color);
        }
        break;
    case eKomoot:
    default:
        break;
    }
}

void UIMain::Update(const ISinkKomoot::KomootData_t &data)
{
    switch (gui_mode_)
    {
    case eCsc:
        break;
    case eHybrid:
    case eKomoot:
        if (data.direction_updated)
        {
            const uint8_t *ptr = GetNavIcon(data.direction);
            if (ptr)
            {
                tft_.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
            }
        }
        if (data.distance_m_updated)
        {
            char str[10] = {0};
            uint8_t len = sprintf(str, "%i", data.distance_m);
            tft_.setFont(&Open_Sans_Condensed_Bold_31);
            tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
            tft_.write_chars(0, 130, 80, str, len);
        }
        if (data.street_updated)
        {
            tft_.setFont(&Open_Sans_Condensed_Bold_31);
            tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
            tft_.WriteString(0, 80, 80, 80, (char *)data.street);
        }
    default:
        break;
    }
}

void UIMain::Log(const char *str)
{
    tft_.printf("%s", str);
}

void UIMain::Operational()
{
    tft_.fillScreen(ST77XX_BLACK);
}

void UIMain::SetGuiMode(eGuiMode_t mode)
{
    gui_mode_ = mode;
}

void UIMain::DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color)
{
    char str[10];
    uint16_t len = sprintf(str, ".%i", speed_kmhX10 % 10);
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.setTextColor(color);
    tft_.write_chars(56, y + 12, 20, str, len, 0, false);

    len = sprintf(str, "%i", speed_kmhX10 / 10);
    tft_.setFont(&Open_Sans_Condensed_Bold_49);
    tft_.write_chars(0, y, 56, str, len);

    /*
    tft_.setFont(&Open_Sans_Condensed_Bold_49);
    tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_.write_chars(0, Y_SPEED, 80, str, len);
    */
}

void UIMain::DrawCadence(uint16_t y, uint16_t cadence)
{
    char str[10];
    uint16_t len = sprintf(str, "%i", cadence);

    if (cadence < 80)
        tft_.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    else if (cadence > 95)
        tft_.setTextColor(Adafruit_ST7735::Color565(255, 0, 0));
    else
        tft_.setTextColor(Adafruit_ST7735::Color565(0, 255, 0));

    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.write_chars(0, y, 80, str, len);
}

void UIMain::DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color)
{
    char str[14];
    uint8_t hour = trip_time_sec / 3660;
    uint8_t min = (trip_time_sec / 60) % 60;
    uint8_t sec = trip_time_sec % 60;
    uint8_t len = sprintf(str, "%i:%.2i", hour, min);
    tft_.setTextColor(color);
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.write_chars(0, y, 80, str, len);
}

void UIMain::DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color)
{
    char str[10];
    uint16_t km = trip_distance_m / 1000;
    uint8_t len = sprintf(str, "%i.%.2i", km, (trip_distance_m % 1000) / 10);
    tft_.setTextColor(color);
    tft_.setFont(&Open_Sans_Condensed_Bold_31);
    tft_.write_chars(0, y, 80, str, len);
}
