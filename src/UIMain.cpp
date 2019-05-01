#include "UIMain.h"
#include "gfxfont.h"
#include "val.h"
#include "tracer.h"

DigitalOut display_led((PinName)11);

UIMain::UIMain()
    : csc_bat_(0xFF)
    , gui_mode_(eStartup), layout_csc_(tft), layout_komoot_(tft)
{
    tft.initR(INITR_MINI160x80);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.setCursor(0, 5);
    tft.setTextWrap(true);
    tft.setFont(NULL);
    layout_ = &layout_csc_;
    display_led = 0;
}

uint8_t UIMain::FormatSpeed(char* str, uint16_t speed_kmhX10)
{
    uint8_t len = 0;
    if (speed_kmhX10 <= 999)
    {
        len = sprintf(str, "%i.%i", speed_kmhX10 / 10, speed_kmhX10 % 10);
    }
    return len;
}

void UIMain::Update(const CscData_t& data)
{
    INFO("UIMain::Update, %u, %u\r\n", csc_data_.speed_kmhX10 , data.speed_kmhX10);
    char str[10];
    if (IsUint16Updated(csc_data_.speed_kmhX10 , data.speed_kmhX10))
    {
        INFO("~UIMain::UpdateSpeed() => %u\r\n", data.speed_kmhX10);
        uint8_t len = FormatSpeed(str, data.speed_kmhX10);
        layout_->UpdateSpeedStr(data.speed_kmhX10, str, len);
    }

    if (IsUint16Updated(csc_data_.filtered_speed_kmhX10 , data.filtered_speed_kmhX10))
    {
        INFO("~UIMain::UpdateFilteredSpeed() => %u\r\n", data.filtered_speed_kmhX10);
        uint8_t len = FormatSpeed(str, data.filtered_speed_kmhX10);
        layout_->UpdateAverageSpeedStr(data.filtered_speed_kmhX10, str, len);
    }
/*
    if (IsUint16Updated(csc_data_.average_speed_kmhX10 , data.average_speed_kmhX10))
    {
        INFO("~UIMain::UpdateAverageSpeed() => %u\r\n", data.average_speed_kmhX10);
        uint8_t len = FormatSpeed(str, data.average_speed_kmhX10);
        layout_->UpdateAverageSpeedStr(data.average_speed_kmhX10, str, len);
    }*/


    if (IsUint32Updated(csc_data_.distance_cm, data.distance_cm))
    {
        INFO("~UIMain::UpdateTravelDistance() => %u\r\n", data.distance_cm);
        uint8_t len = sprintf(str, "%i.%i", data.distance_cm / 100, (data.distance_cm % 100) / 10);
        layout_->UpdateTravelDistanceStr(data.distance_cm, str, len);
    }

    if (IsUint16Updated(csc_data_.cadence, data.cadence))
    {
        INFO("~UIMain::UpdateCadence() => %u\r\n", data.cadence);
        uint8_t len = sprintf(str, "%i", data.cadence);
        layout_->UpdateCadenceStr(data.cadence, str, len);
    }

    if (IsUint32Updated(csc_data_.time_ms, data.time_ms))
    {
        INFO("~UIMain::UpdateTravelTime() => %u\r\n", data.time_ms);
        uint8_t len = sprintf(str, "%i:%.2i", data.time_ms / 60000, (data.time_ms/1000) % 60);
        layout_->UpdateTravelTimeStr(data.time_ms / 1000, str, len);
    }

    if (IsBoolUpdated(csc_data_.is_riding, data.is_riding))
    {
        INFO("UpdateIsRiding %d\r\n", data.is_riding);
        layout_->UpdateIsRiding(data.is_riding);
    }
}


void UIMain::UpdateKomootDirection(uint8_t dir)
{
    if (IsUint8Updated(komoot_direction_, dir))
    {
        INFO("~UIMain::UpdateKomootDirection() => %u\r\n", dir);
        layout_->UpdateKomootDirection(dir);
    }
}

void UIMain::UpdateKomootDistance(uint32_t distance)
{
    if (IsUint32Updated(komoot_distance_, distance))
    {
        INFO("~UIMain::UpdateKomootDistance() => %u\r\n", distance);
        char str[10] = {0};
        uint8_t len = sprintf(str, "%i", distance);
        layout_->UpdateKomootDistanceStr(distance, str, len);
    }
}

void UIMain::UpdateKomootStreet(uint8_t *street)
{
    if (IsStringUpdated(komoot_street_, street, sizeof(komoot_street_)))
    {
        INFO("~UIMain::UpdateKomootStreet() => %s\r\n", street);
        layout_->UpdateKomootStreet(street);
    }
}

void UIMain::Log(const char *str)
{
    tft.printf("%s", str);
}

void UIMain::Operational()
{
    tft.fillScreen(ST77XX_BLACK);
}

void UIMain::SetGuiMode(eGuiMode_t mode)
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