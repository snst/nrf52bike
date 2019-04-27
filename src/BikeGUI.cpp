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

BikeGUI::BikeGUI()
{
    tft.initR(INITR_MINI160x80);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.setCursor(0, 5);
    tft.setTextWrap(true);
    tft.setFont(NULL);
}

void BikeGUI::SetCursor(uint8_t x, uint8_t y, uint8_t size)
{
    tft.setTextSize(size);
    y = y * 40;
    tft.fillRect(x, y, 80, 40, Adafruit_ST7735::Color565(0, 0, 0));
    tft.setTextWrap(false);
    //tft.setFont(&FreeSansBold24pt7b);
    tft.setFont(&digital_732pt7b);

    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));
    tft.setCursor(x, y + 40);
}

void BikeGUI::ShowValue(uint8_t x, uint8_t y, uint32_t value)
{
    SetCursor(x,y,1);
    tft.printf("%.2d", value);
}

void BikeGUI::ShowSpeed(uint8_t x, uint8_t y, uint16_t value)
{
    uint8_t w = 25;
	tft.setFont(&Open_Sans_Condensed_Bold_49);
    char str[5] = {0};
    uint8_t n = sprintf(str, "%i", value);
    tft.write_chars(0,0,80,str, n);
}

void BikeGUI::UpdateSpeed(uint16_t val)
{
    if (IsUint16Updated(csc_speed_, val))
    {
        INFO("~BikeGUI::UpdateSpeed() => %u\r\n", val);
        ShowSpeed(3, 2, val);
    }
}

void BikeGUI::UpdateAverageSpeed(uint16_t val)
{
    if (IsUint16Updated(csc_average_speed_, val))
    {
        INFO("~BikeGUI::UpdateAverageSpeed() => %u\r\n", val);
        //ShowValue(0, 1, val);
    }
}

void BikeGUI::UpdateTravelDistance(uint16_t val)
{
    if (IsUint16Updated(csc_distance_, val))
    {
        INFO("~BikeGUI::UpdateTravelDistance() => %u\r\n", val);
        /*
        uint8_t y = 70;
        uint8_t x = 2;
        uint8_t w = f22_width;
        uint8_t h = f22_height/10;
        tft.fillRect(0, y, 80, h, Adafruit_ST7735::Color565(0, 0, 0));
        ShowDigit('m', x, y, val<1000 ? -1 : val / 1000);
        x += (w+2);
        ShowDigit('m', x, y, val<100 ? -1 : ((val/100) %10));
        x += (w+2);
        ShowDigit('m', x, y, ((val/10) %10));
        x += (w + 1);
        ShowDot(x, y + h-dot_height);
        x += (dot_width + 1);
        ShowDigit('m', x, y, val % 10);
*/
    }
}

void BikeGUI::UpdateTravelTime(uint16_t val)
{
    if (IsUint16Updated(csc_travel_time_, val))
    {
        INFO("~BikeGUI::UpdateTravelTime() => %u\r\n", val);
        //ShowValue(0, 3, val);
    }
}

void BikeGUI::UpdateCadence(uint16_t val)
{
    if (IsUint16Updated(csc_cadence_, val))
    {
        INFO("~BikeGUI::UpdateCadence() => %u\r\n", val);
        //ShowValue(0, 4, val);
    }
}

void BikeGUI::ShowDot(uint8_t x, uint8_t y)
{
    tft.drawXBitmap2(x, y, dot_bits, 5, 5, Adafruit_ST7735::Color565(255, 255, 255));
}

void BikeGUI::UpdateDirection(uint8_t dir)
{
    if (IsUint8Updated(komoot_direction_, dir))
    {
        const uint8_t *ptr = GetNavIcon(dir);
        if (ptr)
        {
            INFO("~BikeGUI::UpdateDirection() => %u\r\n", dir);
            tft.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
        }
    }
}


void BikeGUI::UpdateDistance(uint32_t distance)
{
    if (IsUint32Updated(komoot_distance_, distance))
    {
        INFO("~BikeGUI::UpdateDistance() => %u\r\n", distance);
        ShowValue(0, 160 - 70, distance);
    }
}

void BikeGUI::UpdateStreet(uint8_t *street)
{
    if (IsStringUpdated(komoot_street_, street, sizeof(komoot_street_)))
    {
        INFO("~BikeGUI::UpdateStreet() => %s\r\n", street);
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
    UpdateCadence(456);
    UpdateTravelDistance(789);
    UpdateTravelTime(110);
    UpdateAverageSpeed(220);
}