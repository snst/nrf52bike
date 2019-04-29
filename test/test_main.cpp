#include <gtest/gtest.h>
#include "mbed.h"
#include "TftEmu.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
 
TEST(font, font1) { 

    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    tft.setFont(&Open_Sans_Condensed_Bold_31);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.write_chars(0, 0, 80, "h2", 2);

}
 
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}