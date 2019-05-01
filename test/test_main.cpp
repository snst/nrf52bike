#include <gtest/gtest.h>
#include "mbed.h"
#include "TftEmu.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "IDataCsc.h"
#include "AppCsc.h"

TEST(font, font1)
{

    TftEmu tft;
    tft.initR(INITR_MINI160x80);
    tft.setFont(&Open_Sans_Condensed_Bold_31);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft.write_chars(0, 0, 80, "h2", 2);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class CscTestData : public IDataCsc
{
  public:
    CscTestData() {}
    virtual void Update(const CscData_t& data) {}
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
    EXPECT_EQ(2666, csc.last_csc_.wheelCounter);

    csc.ProcessData(6000, d2, sizeof(d2));
}

TEST(CSC, AverageSpeed)
{
    CscTestData d;
    AppCsc csc(&d);
    csc.csc_data_.distance_cm = 23*1000*100;
    csc.csc_data_.time_ms = 30 * 60 * 1000;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(460, csc.csc_data_.average_speed_kmhX10);
}

TEST(CSC, FilteredSpeed)
{
    CscTestData d;
    AppCsc csc(&d);
    EXPECT_EQ(0, csc.csc_data_.filtered_speed_kmhX10);

    csc.csc_data_.speed_kmhX10 = 0;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(0, csc.csc_data_.filtered_speed_kmhX10);

    csc.csc_data_.speed_kmhX10 = 100;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(100 / SPEED_FILTER_VALUES_CNT, csc.csc_data_.filtered_speed_kmhX10);

    csc.csc_data_.speed_kmhX10 = 50;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(150 / SPEED_FILTER_VALUES_CNT, csc.csc_data_.filtered_speed_kmhX10);

    csc.csc_data_.speed_kmhX10 = 200;
    csc.CalculateAverageSpeed();
    EXPECT_EQ(350 / SPEED_FILTER_VALUES_CNT, csc.csc_data_.filtered_speed_kmhX10);

    csc.csc_data_.speed_kmhX10 = 80;
    csc.CalculateAverageSpeed();
    EXPECT_EQ((50+200+80) / SPEED_FILTER_VALUES_CNT, csc.csc_data_.filtered_speed_kmhX10);


    // SPEED_FILTER_VALUES_CNT
}

