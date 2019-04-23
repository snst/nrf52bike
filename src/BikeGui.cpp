#include "BikeGUI.h"
#include "FreeMonoBold24pt7b.h"
#include "gfxfont.h"
#include "icons.h"
#include "val.h"


BikeGUI::BikeGUI()
{
    tft.initR(INITR_MINI160x80);
    tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));
}

void BikeGUI::ShowValue(uint8_t x, uint8_t y, uint32_t value)
{
    tft.fillRect(x, y, 80, 50, ST77XX_BLACK);
    tft.setCursor(x, y+40);
    tft.printf("%d", value);
}

void BikeGUI::UpdateSpeed(uint16_t speed)
{
    if (IsUint16Updated(csc_speed_, speed))
    {
        INFO("~BikeGUI::UpdateSpeed() => %u", speed);
        ShowValue(0,80, speed);
    }
}

void BikeGUI::UpdateCadence(uint16_t cadence)
{

}

void BikeGUI::UpdateDirection(uint8_t dir)
{
    if (IsUint8Updated(komoot_dir_, dir))
    {
        const uint8_t *ptr = GetNavIcon(dir);
        if (ptr)
        {
            INFO("~BikeGUI::UpdateDirection() => %u", dir);
            tft.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
        }
    }
}

void BikeGUI::UpdateDistance(uint32_t distance)
{
    if (IsUint32Updated(komoot_distance_, distance))
    {
        INFO("~BikeGUI::UpdateDistance() => %u", distance);
        ShowValue(0,80, distance);
    }    
}

void BikeGUI::UpdateStreet(uint8_t* street)
{
    if(IsStringUpdated(street_, street, sizeof(street_)))
    {
        INFO("~BikeGUI::UpdateStreet() => %s", street);
    }
}