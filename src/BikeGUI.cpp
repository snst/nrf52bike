#include "BikeGUI.h"
#include "gfxfont.h"
#include "val.h"
#include "tracer.h"


BikeGUI::BikeGUI()
    : csc_is_riding_(false), csc_travel_time_riding_(false), csc_bat_(0xFF), csc_speed_kmhX10(0xFFFF), csc_cadence_(0xFFFF), csc_average_speed_kmhX10(0xFFFF), csc_total_distance_cm(0xFFFFFFFF), csc_travel_time_sec(0xFFFFFFFF)
    , gui_mode_(eStartup), layout_csc_(tft), layout_komoot_(tft)
{
    tft.initR(INITR_MINI160x80);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.setCursor(0, 5);
    tft.setTextWrap(true);
    tft.setFont(NULL);
    layout_ = &layout_csc_;
}

uint8_t BikeGUI::FormatSpeed(char* str, uint16_t speed_kmhX10)
{
    uint8_t len = 0;
    if (speed_kmhX10 <= 999)
    {
        len = sprintf(str, "%i.%i", speed_kmhX10 / 10, speed_kmhX10 % 10);
    }
    return len;
}

void BikeGUI::UpdateSpeed(uint16_t speed_kmhX10)
{
    if (IsUint16Updated(csc_speed_kmhX10, speed_kmhX10))
    {
        INFO("~BikeGUI::UpdateSpeed() => %u\r\n", speed_kmhX10);
        char str[5];
        uint8_t len = FormatSpeed(str, speed_kmhX10);
        layout_->UpdateSpeedStr(speed_kmhX10, str, len);
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
        char str[5];
        uint8_t len = FormatSpeed(str, average_kmhX10);
        layout_->UpdateAverageSpeedStr(average_kmhX10, str, len);
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
        char str[10] = {0};
        uint8_t len = sprintf(str, "%i.%i", distance_cm / 100, (distance_cm % 100) / 10);
        layout_->UpdateTravelDistanceStr(distance_cm, str, len);
    }
}

void BikeGUI::UpdateTravelTime(uint32_t time_sec)
{
    if (IsUint32Updated(csc_travel_time_sec, time_sec))
    {
        INFO("~BikeGUI::UpdateTravelTime() => %u\r\n", time_sec);
        char str[10] = {0};
        uint8_t len = sprintf(str, "%i:%.2i", time_sec / 60, time_sec % 60);
        layout_->UpdateTravelTimeStr(time_sec, str, len);
    }

    UpdateAverageSpeed();
}

void BikeGUI::UpdateCadence(uint16_t cadence)
{
    if (IsUint16Updated(csc_cadence_, cadence))
    {
        INFO("~BikeGUI::UpdateCadence() => %u\r\n", cadence);
        char str[10] = {0};
        uint8_t len = sprintf(str, "%i", cadence);
        layout_->UpdateCadenceStr(cadence, str, len);
    }
}

void BikeGUI::UpdateKomootDirection(uint8_t dir)
{
    if (IsUint8Updated(komoot_direction_, dir))
    {
        INFO("~BikeGUI::UpdateKomootDirection() => %u\r\n", dir);
        layout_->UpdateKomootDirection(dir);
    }
}

void BikeGUI::UpdateKomootDistance(uint32_t distance)
{
    if (IsUint32Updated(komoot_distance_, distance))
    {
        INFO("~BikeGUI::UpdateKomootDistance() => %u\r\n", distance);
        char str[10] = {0};
        uint8_t len = sprintf(str, "%i", distance);
        layout_->UpdateKomootDistanceStr(distance, str, len);
    }
}

void BikeGUI::UpdateKomootStreet(uint8_t *street)
{
    if (IsStringUpdated(komoot_street_, street, sizeof(komoot_street_)))
    {
        INFO("~BikeGUI::UpdateKomootStreet() => %s\r\n", street);
        layout_->UpdateKomootStreet(street);
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

void BikeGUI::SetGuiMode(eGuiMode_t mode)
{
    switch(mode) {
        case eCsc:
        layout_ = &layout_csc_;
        break;
        case eKomoot:
        default:
        layout_ = &layout_komoot_;
        break;
    }
    gui_mode_ = mode;
}