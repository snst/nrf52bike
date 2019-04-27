#include "BikeGUI.h"
//#include "FreeSansBold24pt7b.h"
//#include "Open_Sans_Condensed_Bold.h"
#include "digital7.h"
#include "gfxfont.h"
#include "icons.h"
#include "val.h"
#include "tracer.h"
#include "../font/f36.xbm"
#include "../font/f22.xbm"
#include "../font/dot.xbm"

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
    tft.fillRect(0, y, 80, f36_height/10, Adafruit_ST7735::Color565(0, 0, 0));
    ShowDigit('b', x, y, value<100 ? -1 : value / 100);
    ShowDigit('b', x+w, y, ((value/10) %10));
    ShowDot(x+w+w+2, 29);
    ShowDigit('m', x+w+w+2+5+2, y+11, value % 10);
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


const uint8_t* BikeGUI::GetDigit(int8_t digit, const unsigned char* data, uint16_t offset) 
{
    const uint8_t *p = NULL;
    if(digit>=0 && digit<=9) {
        return data+(offset*digit); // 850
    }
    return p;
}
// 14*21 = 294.. 42


void BikeGUI::ShowDigit(uint8_t ch, uint8_t x, uint8_t y, int8_t digit)
{
    const uint8_t *ptr = NULL;
    int16_t w, h;
    switch(ch) {
        case 'b': 
        ptr = GetDigit(digit, f36_bits, 136);
        w = f36_width;
        h = f36_height/10;
        break;
        case 'm': 
        ptr = GetDigit(digit, f22_bits, 42);
        w = f22_width;
        h = f22_height/10;
        break;
        default:
        break;
    }
    
    if (ptr)
    {
        tft.drawXBitmap2(x, y, ptr, w, h, Adafruit_ST7735::Color565(255, 255, 255));
    } 
    else 
    {
//        tft.fillRect(x, y, f36_width, f36_height/10, Adafruit_ST7735::Color565(0, 0, 0));
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