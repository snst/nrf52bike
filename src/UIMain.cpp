#include "UIMain.h"
#include "gfxfont.h"
#include "tracer.h"
#include "icons.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"

DigitalOut display_led((PinName)11);
IUILog *uilog = NULL;

#define Y_CSC_SPEED 0u
#define Y_CSC_AVERAGE_SPEED (Y_CSC_SPEED + 40u)
#define Y_CSC_CADENCE (Y_CSC_AVERAGE_SPEED + 40u)
#define Y_CSC_TRAVEL_DISTANCE (Y_CSC_CADENCE + 26u)
#define Y_CSC_TRAVEL_TIME (Y_CSC_TRAVEL_DISTANCE + 26u)

#define Y_KOMOOT_MIDDLE_START (72u)
#define Y_KOMOOT_BOTTOM_START (160u-22u-2u)
#define Y_KOMOOT_TRIP_DISTANCE Y_KOMOOT_BOTTOM_START
#define Y_KOMOOT_SPEED Y_KOMOOT_BOTTOM_START
#define Y_KOMMOT_DISTANCE (Y_KOMOOT_MIDDLE_START+7u)
#define Y_KOMMOT_STREET Y_KOMOOT_MIDDLE_START

UIMain::UIMain(GFX *tft, events::EventQueue &event_queue)
    : tft_(tft), event_queue_(event_queue), csc_bat_(0xFF), gui_mode_(eStartup), komoot_view_(0), last_komoot_view_switch_ms_(0)
    , last_distance_bar_(0xFF), last_direction_color_(0)
{
    tft_->fillScreen(ST77XX_BLACK);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_->setCursor(0, 5);
    tft_->setTextWrap(true);
    tft_->setFont(NULL);
    display_led = 0; // enable display led
    uilog = this;
}

void UIMain::Update(const ISinkCsc::CscData_t &data)
{
    FLOW("UIMain::Update(CSC)\r\n");
    SetOperational();

    switch (gui_mode_)
    {
    case eCsc:
        if (data.filtered_speed_kmhX10_updated)
        {
            INFO("Update, eCsc, filtered speed: %u\r\n", data.filtered_speed_kmhX10);
            DrawSpeed(Y_CSC_SPEED, data.filtered_speed_kmhX10);
        }
        if (data.average_speed_kmhX10_updated)
        {
            INFO("Update, eCsc, average speed: %u\r\n", data.average_speed_kmhX10);
            DrawSpeed(Y_CSC_AVERAGE_SPEED, data.average_speed_kmhX10);
        }
        if (data.cadence_updated)
        {
            INFO("Update, eCsc, cadence: %u\r\n", data.cadence);
            DrawCadence(0, Y_CSC_CADENCE, data.cadence);
        }
        if (data.average_cadence_updated)
        {
            INFO("Update, eCsc, average_cadence: %u\r\n", data.average_cadence);
            DrawCadence(40, Y_CSC_CADENCE, data.average_cadence);
        }
        if (data.trip_distance_cm_updated)
        {
            INFO("Update, eCsc, trip distance: %u\r\n", data.trip_distance_cm);
            DrawDistance(Y_CSC_TRAVEL_DISTANCE, data.trip_distance_cm / 100);
        }
        if (data.trip_time_ms_updated || data.is_riding_updated)
        {
            uint16_t color = data.is_riding ? Adafruit_ST7735::Color565(255, 255, 255) : Adafruit_ST7735::Color565(255, 0, 0);
            INFO("Update, eCsc, trip time: %u\r\n", data.trip_time_ms);
            DrawTime(Y_CSC_TRAVEL_TIME, data.trip_time_ms / 1000, color);
        }
        break;
    case eHybrid:
    case eKomoot:
        if (data.filtered_speed_kmhX10_updated)
        {
            INFO("Update, eCsc, filtered speed: %u\r\n", data.filtered_speed_kmhX10);
            char str[10] = {0};
            uint8_t len = sprintf(str, "%i", data.filtered_speed_kmhX10 / 10);
            tft_->setFont(&Open_Sans_Condensed_Bold_31);
            tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
            tft_->WriteStringLen(0, Y_KOMOOT_SPEED, 30, str, len, 2, GFX::eLeft);
        }
        if (data.trip_distance_cm_updated)
        {
            INFO("Update, eCsc, fdistance: %u\r\n", data.trip_distance_cm);
            char str[10] = {0};
            uint8_t len = sprintf(str, "%i", data.trip_distance_cm / 100000);
            tft_->setFont(&Open_Sans_Condensed_Bold_31);
            tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
            tft_->WriteStringLen(30, Y_KOMOOT_TRIP_DISTANCE, 50, str, len, 2, GFX::eRight);
        }
    default:
        break;
    }
}

void UIMain::Update(const ISinkKomoot::KomootData_t &data, bool force)
{
    FLOW("UIMain::Update(Komoot)\r\n");

    SetOperational();
    switch (gui_mode_)
    {
    case eCsc:
        break;
    case eHybrid:
    case eKomoot:
        if (data.direction_updated || (last_direction_color_ != GetKomootDirectionColor(data.distance_m)))
        {
            DrawKomootDirection(data);
        }

        if (data.street_updated) // new street => always show it
        {
            DrawKomootStreet(data);
            last_komoot_view_switch_ms_ = GetMillis();
            komoot_view_ = 0;
        }
        else if (GetMillis() - last_komoot_view_switch_ms_ > 2000u)
        {
            last_komoot_view_switch_ms_ = GetMillis();
            komoot_view_ = (komoot_view_ + 1u) % 2u;
            if (0u == komoot_view_)
            {
                DrawKomootStreet(data);
            }
            else
            {
                DrawKomootDistance(data);
            }
        }
        else if ((1 == komoot_view_) && data.distance_m_updated)
        {
            DrawKomootDistance(data);
        }

        if (data.distance_m_updated) 
        {
            DrawKomootDistanceBar(data);
        }

    default:
        break;
    }
}

void UIMain::DrawKomootDistance(const ISinkKomoot::KomootData_t &data)
{
    tft_->fillRect(0, Y_KOMOOT_MIDDLE_START, 80, Y_KOMOOT_BOTTOM_START-Y_KOMOOT_MIDDLE_START, 0);
    char str[10] = {0};
    uint8_t len;
    if (data.distance_m <= 999)
    {
        len = sprintf(str, "%i", data.distance_m);
    }
    else
    {
        len = sprintf(str, "%i.%i", data.distance_m / 1000u, (data.distance_m % 1000u) / 100u);
    }
    tft_->setFont(&Open_Sans_Condensed_Bold_49);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_->WriteStringLen(0, Y_KOMMOT_DISTANCE, 80, str, len, 2, GFX::eCenter);
}

void UIMain::Log(const char *str)
{
    tft_->printf("%s", str);
}

void UIMain::SetOperational()
{
    INFO("SetOperational()1\r\n");
    if (NULL != uilog)
    {
        INFO("Disable log\r\n");
        uilog = NULL;
        tft_->fillScreen(ST77XX_BLACK);
    }
}

void UIMain::SetGuiMode(eGuiMode_t mode)
{
    gui_mode_ = mode;
}

void UIMain::DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color)
{
    char str[10];
    uint16_t len = sprintf(str, ".%i", speed_kmhX10 % 10);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(color);
    tft_->WriteStringLen(56, y + 12, 20, str, len, 0, GFX::eLeft);

    len = sprintf(str, "%i", speed_kmhX10 / 10);
    tft_->setFont(&Open_Sans_Condensed_Bold_49);
    tft_->WriteStringLen(0, y, 56, str, len);

    /*
    tft_->setFont(&Open_Sans_Condensed_Bold_49);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_->WriteStringLen(0, Y_SPEED, 80, str, len);
    */
}
void UIMain::SetCadenceColor(uint16_t cadence)
{
    if (cadence < 80)
        tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    else if (cadence > 95)
        tft_->setTextColor(Adafruit_ST7735::Color565(255, 0, 0));
    else
        tft_->setTextColor(Adafruit_ST7735::Color565(0, 255, 0));
}


void UIMain::DrawCadence(uint16_t x, uint16_t y, uint16_t cadence)
{
    char str[10];
    uint16_t len = sprintf(str, "%i", cadence);
    SetCadenceColor(cadence);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->WriteStringLen(x, y, 40, str, len);
}

void UIMain::DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color)
{
    char str[14];
    uint8_t hour = trip_time_sec / 3660;
    uint8_t min = (trip_time_sec / 60) % 60;
    uint8_t sec = trip_time_sec % 60;
    uint8_t len = sprintf(str, "%i:%.2i", hour, min);
    tft_->setTextColor(color);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->WriteStringLen(0, y, 80, str, len);
}

void UIMain::DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color)
{
    char str[10];
    uint16_t km = trip_distance_m / 1000;
    uint8_t len = sprintf(str, "%i.%.2i", km, (trip_distance_m % 1000) / 10);
    tft_->setTextColor(color);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->WriteStringLen(0, y, 80, str, len);
}

void UIMain::DrawKomootStreet(const ISinkKomoot::KomootData_t &data)
{
    tft_->fillRect(0, Y_KOMMOT_STREET, 80, Y_KOMOOT_SPEED-Y_KOMMOT_STREET, 0);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    char street_ascii[20];
    ConvertUtf8toAscii(data.street, strlen((char *)data.street), street_ascii, sizeof(street_ascii));
    tft_->WriteString(0, Y_KOMMOT_STREET, 80, Y_KOMOOT_BOTTOM_START-Y_KOMOOT_MIDDLE_START, street_ascii);
}

uint16_t UIMain::GetKomootDistanceBarColor(uint16_t distance_m)
{
    uint16_t color;
    if (distance_m <= 100u)
    {
        color = Adafruit_ST7735::Color565(255, 0, 0);
    }
    else if (distance_m <= 200u)
    {
        color = Adafruit_ST7735::Color565(255, 255, 0);
    }
    else 
    {
        color = Adafruit_ST7735::Color565(0, 255, 0);
    }
    return color;
}

uint16_t UIMain::GetKomootDirectionColor(uint16_t distance_m)
{
    uint16_t color = distance_m <= 50u ? Adafruit_ST7735::Color565(255, 0, 0) : 0xFFFF;
    return color;
}

void UIMain::DrawKomootDirection(const ISinkKomoot::KomootData_t &data)
{
    const uint8_t *ptr = GetNavIcon(data.direction);
    if (ptr)
    {
        last_direction_color_ = GetKomootDirectionColor(data.distance_m);
        tft_->drawXBitmap2(10, 0, ptr, 70, 70, last_direction_color_);
    }
}

void UIMain::DrawKomootDistanceBar(const ISinkKomoot::KomootData_t &data)
{
    uint16_t warn_distance = 400;
    uint8_t w = 12;
    uint8_t h = 70;
    uint16_t color = 0;
    uint16_t k = h;
    if (data.distance_m < warn_distance) {
        k = data.distance_m * h / warn_distance;
        color = Adafruit_ST7735::Color565(10, 10, 10);
    }
    if (last_distance_bar_ != k) {

        last_distance_bar_ = k;
        tft_->fillRect(0, 0, w, k, color);

        if (k < h) {
            color = GetKomootDistanceBarColor(data.distance_m);
            tft_->fillRect(0, k, w, h-k, color);
        }
    }
}