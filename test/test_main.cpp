#include <gtest/gtest.h>
#include "mbed.h"
#include "TftEmu.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "IUICsc.h"
#include "AppCsc.h"
#include "IUILog.h"
#include "UIMain.h"
#include "common.h"
#include "UISettings.h"

TEST(font, font_h2)
{
    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    tft.setFont(&Open_Sans_Condensed_Bold_31);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.WriteStringLen(0, 0, 80, "h2", 2);
}

TEST(font, font_444_444)
{
    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    tft.setFont(&Open_Sans_Condensed_Bold_31);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.WriteStringLen(0, 0, 40, "158", 3, 0, GFX::eRight);
    tft.WriteStringLen(0, 0, 80, "133", 3, 0, GFX::eRight);
    tft.WriteStringLen(0, 0, 80, "Zielstrasse", 11, 0, GFX::eLeft);
    tft.WriteStringLen(0, 0, 80, "paqy", 4, 0, GFX::eLeft);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class CscTestData : public IUICsc
{
  public:
    CscTestData() {}
    virtual void Update(const CscData_t& data, bool force) {}
};

TEST(CSC, ProcessData)
{
    CscTestData d;
    AppCsc csc(&d);

    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027
    uint8_t d1[] = {0x03, 0x6a, 0x0a, 0x00, 0x00, 0xbc, 0x9c, 0xad, 0x00, 0x1b, 0xfa};
    uint8_t d2[] = {0x03, 0x77, 0x0b, 0x00, 0x00, 0xa5, 0xb9, 0xad, 0x00, 0x1b, 0xfa};

    csc.ProcessData(5000, d1, sizeof(d1));
    EXPECT_EQ(2666, csc.last_msg_.wheelCounter);

    csc.ProcessData(6000, d2, sizeof(d2));
}

TEST(CSC, AverageSpeed)
{
    CscTestData d;
    AppCsc csc(&d);
    csc.data_.trip_distance_cm = 23*1000*100;
    csc.data_.trip_time_ms = 30 * 60 * 1000;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(460, csc.data_.average_speed_kmhX10);
}

TEST(CSC, FilteredSpeed)
{
    CscTestData d;
    AppCsc csc(&d);
    EXPECT_EQ(0, csc.data_.filtered_speed_kmhX10);

    csc.data_.speed_kmhX10 = 0;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(0, csc.data_.filtered_speed_kmhX10);

    csc.data_.speed_kmhX10 = 100;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(100 / FILTER_VALUES_CNT, csc.data_.filtered_speed_kmhX10);

    csc.data_.speed_kmhX10 = 50;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(150 / FILTER_VALUES_CNT, csc.data_.filtered_speed_kmhX10);

    csc.data_.speed_kmhX10 = 200;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(350 / FILTER_VALUES_CNT, csc.data_.filtered_speed_kmhX10);

    csc.data_.speed_kmhX10 = 80;
    csc.CalculateAverageSpeed();
    EXPECT_EQ((50+200+80) / FILTER_VALUES_CNT, csc.data_.filtered_speed_kmhX10);
}

TEST(common, utf_0x01)
{
    uint8_t in[] = { 'a', 0x01, 'b', 'c', 0 };
    char out[100];
    uint16_t out_len = sizeof(out);
    ConvertUtf8toAscii(in, strlen((char*)in), out, out_len);
    EXPECT_EQ(0, memcmp("a?bc", out, 4));
//    printf("%s", out);
}

TEST(common, utf2_ss)
{
    uint8_t in[] = { 'a', 0xc3, 0x9f, 'b', 'c', 0 };
    char out[100];
    uint16_t out_len = sizeof(out);
    ConvertUtf8toAscii(in, strlen((char*)in), out, out_len);
    EXPECT_EQ(0, memcmp("assbc", out, 5));
//    printf("%s", out);
}

TEST(common, utf2_ae)
{
    uint8_t in[] = { 'a', 0xc3, 0xa4, 'b', 'c', 0 };
    char out[100];
    uint16_t out_len = sizeof(out);
    ConvertUtf8toAscii(in, strlen((char*)in), out, out_len);
    EXPECT_EQ(0, memcmp("aaebc", out, 5));
//    printf("%s", out);
}

TEST(common, utf2_ox)
{
    uint8_t in[] = { 'a', 0xc3, 0x01, 'b', 'c', 0 };
    char out[100];
    uint16_t out_len = sizeof(out);
    ConvertUtf8toAscii(in, strlen((char*)in), out, out_len);
    EXPECT_EQ(0, memcmp("a?bc", out, 4));
//    printf("%s", out);
}

TEST(UIMain, kommot_dist_1)
{
    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    events::EventQueue eq;
    IUIKomoot::KomootData_t d;
    UIMain ui(&tft, eq);
    d.distance_m = 1;
    ui.DrawKomootDistance(d);
    d.distance_m = 999;
    ui.DrawKomootDistance(d);
    d.distance_m = 2222;
    ui.DrawKomootDistance(d);
    d.distance_m = 33333;
    ui.DrawKomootDistance(d);
}

TEST(UISettings, test1)
{
    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    events::EventQueue eq;
    UIMain ui(&tft, eq);
    UISettings s(&tft, &ui);
    s.HandleLongPress();
    s.HandleEvent(eShort);
    s.HandleEvent(eShort);
    s.HandleEvent(eShort);
    s.HandleEvent(eLong);
    s.HandleEvent(eShort);
    s.HandleEvent(eLong);
}