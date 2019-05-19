#include "UIMain.h"
#include "gfxfont.h"
#include "tracer.h"
#include "icons.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "common.h"

//DigitalOut display_led((PinName)11);
IUILog *uilog = NULL;

#define Y_CSC_SPEED 0u
#define Y_CSC_AVERAGE_SPEED (Y_CSC_SPEED + 40u)
#define Y_CSC_CADENCE (Y_CSC_AVERAGE_SPEED + 40u)
#define Y_CSC_TRAVEL_DISTANCE (Y_CSC_CADENCE + 26u)
#define Y_CSC_TRAVEL_TIME (Y_CSC_TRAVEL_DISTANCE + 26u)

#define Y_KOMMOT_DISTANCE (62u)
#define Y_KOMMOT_STREET (105)

PwmOut display_led((PinName)11);

UIMain::UIMain(GFX *tft, events::EventQueue &event_queue)
    : tft_(tft), event_queue_(event_queue)
    , gui_mode_(eStartup), komoot_view_(0)
    , enable_komoot_switch_(false)
    , switched_to_csc_(false)
    , last_distance_bar_(0xFF), last_direction_color_(0)
    , longpress_handled_(false)
    , uisettings_(tft, this)
    , led_event_id_(0)
    , ignore_touch_up_(false)
    , switched_to_komoot_100_(false)
    , switched_to_komoot_500_(false)
    , csc_conn_state_(eDisconnected)
    , csc_watchdog_event_id_(0)
{
    tft_->fillScreen(ST77XX_BLACK);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    tft_->setCursor(0, 5);
    tft_->setTextWrap(true);
    tft_->setFont(NULL);
    display_led.period_ms(1);  
    LedOn();

    uilog = this;
    memset(&last_csc_, 0, sizeof(last_csc_));
    memset(&last_komoot_, 0, sizeof(last_komoot_));
}

#define LONGPRESS_MS (1000u)


void UIMain::LongPress()
{
//    INFO("LONGPRESS EVENT\r\n");
    longpress_handled_ = true;
    switch (gui_mode_)
        {
            case eSettings:
            uisettings_.LongPress();
            break;
            default:
            SetUiMode(eSettings);    
            break;
        }
}

void UIMain::TouchDown()
{
    ignore_touch_up_ = (0 == led_event_id_);
    //INFO("D\r\n");
    touch_down_ms_ = GetMillis();
    longpress_id_ = event_queue_.call_in(500, mbed::callback(this, &UIMain::LongPress));
    longpress_handled_ = false;
    LedOn();
}

void UIMain::TouchUp()
{
    if (!longpress_handled_) {
        event_queue_.cancel(longpress_id_);
//        INFO("SHORTPRESS UP\r\n");
        if (!ignore_touch_up_) {
            switch (gui_mode_)
            {
                default:
                case eKomoot:
                    SetUiMode(eCsc);
                break;
                case eCsc:
                    SetUiMode(eKomoot);
                break;
                case eSettings:
                    uisettings_.ShortPress();
                    break;
            }
        }
    }
    else {
//        INFO("LONGPRESS UP\r\n");
    }
}


void UIMain::Update(const ISinkCsc::CscData_t &data, bool force)
{
    FLOW("UIMain::Update(CSC), force=%d\r\n", force);
    SetOperational();

    if (!force && (0 != csc_watchdog_event_id_)) {
        event_queue_.cancel(csc_watchdog_event_id_);
        csc_watchdog_event_id_ = event_queue_.call_in(5000u, mbed::callback(this, &UIMain::OnCscWatchdog));
    }

    switch (gui_mode_)
    {
    case eCsc:
        if (force || (last_csc_.filtered_speed_kmhX10 != data.filtered_speed_kmhX10))
        {
            INFO("Update, eCsc, filtered speed: %u\r\n", data.filtered_speed_kmhX10);
            DrawSpeed(Y_CSC_SPEED, data.filtered_speed_kmhX10);
        }
        if (force || (last_csc_.average_speed_kmhX10 != data.average_speed_kmhX10))
        {
            INFO("Update, eCsc, average speed: %u\r\n", data.average_speed_kmhX10);
            DrawSpeed(Y_CSC_AVERAGE_SPEED, data.average_speed_kmhX10);
        }
        if (force || (last_csc_.cadence != data.cadence))
        {
            INFO("Update, eCsc, cadence: %u\r\n", data.cadence);
            DrawCadence(0, Y_CSC_CADENCE, data.cadence);
        }
        if (force || (last_csc_.average_cadence != data.average_cadence))
        {
            INFO("Update, eCsc, average_cadence: %u\r\n", data.average_cadence);
            DrawCadence(40, Y_CSC_CADENCE, data.average_cadence);
        }
        if (force || (last_csc_.trip_distance_cm != data.trip_distance_cm))
        {
            INFO("Update, eCsc, trip distance: %u\r\n", data.trip_distance_cm);
            DrawDistance(Y_CSC_TRAVEL_DISTANCE, data.trip_distance_cm / 100);
        }
        if (force || (last_csc_.trip_time_ms != data.trip_time_ms) || (last_csc_.is_riding != data.is_riding))
        {
            uint16_t color = data.is_riding ? Adafruit_ST7735::Color565(255, 255, 255) : Adafruit_ST7735::Color565(255, 0, 0);
            INFO("Update, eCsc, trip time: %u\r\n", data.trip_time_ms);
            DrawTime(Y_CSC_TRAVEL_TIME, data.trip_time_ms / 1000, color);
        }
        break;
    case eKomoot:
    default:
        break;
    }
    last_csc_ = data;
}


void UIMain::Update(const ISinkKomoot::KomootData_t &data, bool force)
{
    FLOW("UIMain::Update(Komoot), force=%d\r\n", force);

    SetOperational();

    if (data.distance_m <= uisettings_.settings_.komoot_alert_dist)
    {
        bool switch_to_komoot = false;

        if (!switched_to_komoot_500_) {
            switched_to_komoot_500_ = true;
            switch_to_komoot = true;
        }

        if (!switched_to_komoot_100_ && (data.distance_m <= 100))
        {
            switched_to_komoot_100_ = true;
            switch_to_komoot = true;
        }

        if (last_komoot_.direction != data.direction) {
            switch_to_komoot = true;
        }

        if (0 != strcmp((const char*)last_komoot_.street, (const char*)data.street)) {
            switch_to_komoot = true;
        }

        if (switch_to_komoot)
        {
            LedOn();
            SetUiMode(eKomoot);
            switched_to_csc_ = false;
        }
    } 
    else 
    {
        if (!switched_to_csc_) {
            switched_to_csc_ = true;
            SetUiMode(eCsc);
            switched_to_komoot_500_ = false;
            switched_to_komoot_100_ = false;
        }
    }

    switch (gui_mode_)
    {
    case eKomoot:

        if (force || (last_komoot_.direction != data.direction) || (last_direction_color_ != GetKomootDirectionColor(data.distance_m)))
        {
            DrawKomootDirection(data);
        }

        if (force || (0 != strcmp((const char*)last_komoot_.street, (const char*)data.street))) // new street => always show it
        {
            DrawKomootStreet(data);
            komoot_view_ = 0;
        }

        if (force || (last_komoot_.distance_m != data.distance_m))
        {
            DrawKomootDistance(data);
//            DrawKomootDistanceBar(data);
        }

    default:
        break;
    }
    last_komoot_ = data;
}

void UIMain::DrawKomootDistance(const ISinkKomoot::KomootData_t &data)
{
    //tft_->fillRect(0, Y_KOMOOT_MIDDLE_START, 80, Y_KOMOOT_BOTTOM_START-Y_KOMOOT_MIDDLE_START, 0);
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
    //INFO("SetOperational()1\r\n");
    if (NULL != uilog)
    {
        INFO("Disable log\r\n");
        uilog = NULL;
        SetUiMode(eCsc);
    }
}

void UIMain::SetUiMode(eUiMode_t mode)
{
    INFO("SetUiMode %d\r\n", mode);
    if (gui_mode_ != mode) 
    {
        gui_mode_ = mode;
        tft_->fillScreen(ST77XX_BLACK);
        switch (gui_mode_)
        {
            case eKomoot:
                Update(last_komoot_, true);
            break;
            case eCsc:
                Update(last_csc_, true);
            break;
            case eSettings:
                uisettings_.Draw();
                break;
            default:
            break;
        }   
    } 
}

void UIMain::DrawCscConnState()
{
    if (gui_mode_ == eCsc) {
        const char* str = NULL;
        switch (csc_conn_state_) {
            case eOffline:
            str = "off";
            break;
            case eDisconnected:
            str = "dis";
            break;
            case eConnecting:
            str = "c..";
            break;
            case eConnected:
            str = "con";
            break;
            default:
            break;
        }

        if (NULL != str) {
    //    tft_->fillRect(0, y, 80, 40, 0);
            tft_->setFont(&Open_Sans_Condensed_Bold_31);
            tft_->setTextColor(0xFFFF);
            tft_->WriteStringLen(0, 0, 80, str, -1, 0, GFX::eCenter);
        }
    }
}

void UIMain::DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color)
{
    char str[10];
    uint16_t len = sprintf(str, ".%i", speed_kmhX10 % 10);
  
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(color);
    tft_->WriteStringLen(56, y + 12, 24, str, len, 0, GFX::eLeft);

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
    tft_->fillRect(0, Y_KOMMOT_STREET, 80, 160-Y_KOMMOT_STREET, 0);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(Adafruit_ST7735::Color565(255, 255, 255));
    char street_ascii[20];
    ConvertUtf8toAscii(data.street, strlen((char *)data.street), street_ascii, sizeof(street_ascii));
    tft_->WriteString(0, Y_KOMMOT_STREET, 80, 160-Y_KOMMOT_STREET, street_ascii);
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
    uint16_t color = distance_m <= 100u ? Adafruit_ST7735::Color565(0, 255, 0) : 0xFFFF;
    return color;
}

void UIMain::DrawKomootDirection(const ISinkKomoot::KomootData_t &data)
{
    const uint8_t *ptr = GetNavIcon(data.direction);
    if (ptr)
    {
        last_direction_color_ = GetKomootDirectionColor(data.distance_m);
        tft_->drawXBitmap2(10, 0, ptr, 60, 60, last_direction_color_);
    }
}

void UIMain::DrawKomootDistanceBar(const ISinkKomoot::KomootData_t &data)
{
    uint16_t warn_distance = 400;
    uint8_t w = 14;
    uint8_t h = 60;
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

void UIMain::UpdateBat(uint8_t val)
{
    uisettings_.UpdateBat(val);
}

void UIMain::LedOff()
{
    led_event_id_ = 0;
    SetUiBrightness(uisettings_.settings_.display_brightness_off);
}

void UIMain::LedOn()
{
    if( 0 != led_event_id_) {
        event_queue_.cancel(led_event_id_);
    }
    led_event_id_ = event_queue_.call_in(uisettings_.settings_.display_time*1000,  mbed::callback(this, &UIMain::LedOff));
    SetUiBrightness(uisettings_.settings_.display_brightness_on);
}

void UIMain::SetUiBrightness(uint8_t val)
{
    display_led = (float)val / 10.0f;
}

void UIMain::UpdateConnState(ConState_t state)
{
    csc_conn_state_ = state;
    event_queue_.call(mbed::callback(this, &UIMain::DrawCscConnState));
}

void UIMain::OnCscWatchdog()
{
    UpdateConnState(ISinkCsc::eOffline);
}