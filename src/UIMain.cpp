#include "UIMain.h"
#include "gfxfont.h"
#include "tracer.h"
#include "icons.h"
#include "../font/Open_Sans_Condensed_Bold_31.h"
//#include "../font/Open_Sans_Condensed_Bold_38.h"
#include "../font/Open_Sans_Condensed_Bold_49.h"
#include "../font/Open_Sans_Condensed_Bold_37.h"
#include "common.h"
#include "pin_config.h"

IUILog *uilog = NULL;

#define Y_LINE1 0u
#define Y_LINE2 (Y_LINE1 + 45u)
#define Y_LINE3 (Y_LINE2 + 45u)
#define Y_LINE4 (160u - 28u)

#define Y_KOMMOT_DISTANCE (62u)
#define Y_KOMMOT_STREET (105)

PwmOut display_led(BC_DISPLAY_LED);

UIMain::UIMain(GFX *tft, events::EventQueue &event_queue)
    : tft_(tft), event_queue_(event_queue), gui_mode_(IUIMode::eStartup),     //
      komoot_view_(0), enable_komoot_switch_(false), switched_to_csc_(false), //
      last_direction_color_(0), touch_consumed_(true),                        //
      uisettings_(tft), led_event_id_(0), ignore_touch_up_(false),            //
      switched_to_komoot_100_(false), switched_to_komoot_500_(false),         //
      csc_conn_state_(eDisconnected), longpress_id_(0)
{
    tft_->fillScreen(ST77XX_BLACK);
    tft_->setTextColor(COLOR_WHITE);
    tft_->setCursor(0, 5);
    tft_->setTextWrap(true);
    tft_->setFont(NULL);
    display_led.period_ms(1);
    SetBacklightOn();

    uilog = this;
    memset(&last_csc_, 0, sizeof(last_csc_));
    memset(&last_komoot_, 0, sizeof(last_komoot_));
}

#define LONGPRESS_MS (1000u)

void UIMain::HandleLongPress()
{
    touch_consumed_ = true;
    if (0 != longpress_id_)
    {
        longpress_id_ = 0;
        switch (gui_mode_)
        {
        case IUIMode::eSettings:
            uisettings_.HandleLongPress();
            break;
        default:
            SetUiMode(IUIMode::eSettings);
            break;
        }
    }
}

void UIMain::HandleShortPress()
{
    INFO("UIMain::HandleShortPress() %i %i\r\n", bike_computer_->IsAppAvailable(BC::eCsc), bike_computer_->IsAppAvailable(BC::eKomoot));
    switch (gui_mode_)
    {
    default:
    case IUIMode::eStartup:
        bike_computer_->StopScan();
        SetOperational();
        break;
    case IUIMode::eKomoot:
        if (bike_computer_->IsAppAvailable(BC::eCsc))
        {
            SetUiMode(IUIMode::eCsc);
        }
        break;
    case IUIMode::eCsc:
        if (bike_computer_->IsAppAvailable(BC::eKomoot))
        {
            SetUiMode(IUIMode::eKomoot);
        }
        break;
    case IUIMode::eSettings:
        uisettings_.ShortPress();
        break;
    }
}

void UIMain::TouchDown()
{
    //INFO("D\r\n");
    ignore_touch_up_ = (0 == led_event_id_);
    if (0 == longpress_id_)
    {
        longpress_id_ = event_queue_.call_in(LONGPRESS_MS, mbed::callback(this, &UIMain::HandleLongPress));
    }
    touch_consumed_ = false;
    SetBacklightOn();
}

bool UIMain::IgnoreShortTouchUp()
{
    return uisettings_.config_.light_up && ignore_touch_up_;
}

void UIMain::TouchUp()
{
    if (!touch_consumed_) // => short press
    {
        touch_consumed_ = true;
        if (0 != longpress_id_)
        {
            event_queue_.cancel(longpress_id_);
            longpress_id_ = 0;
        }

        if (!IgnoreShortTouchUp())
        {
            event_queue_.call(mbed::callback(this, &UIMain::HandleShortPress));
        }
    }
}

void UIMain::Update(const IUICsc::CscData_t &data, bool force)
{
    FLOW("UIMain::Update(CSC), force=%d\r\n", force);
    SetOperational();

    switch (gui_mode_)
    {
    case IUIMode::eCsc:
        if (force || (last_csc_.filtered_speed_kmhX10 != data.filtered_speed_kmhX10))
        {
            FLOW("Update, eCsc, filtered speed: %u\r\n", data.filtered_speed_kmhX10);
            DrawSpeed(Y_LINE1, data.filtered_speed_kmhX10);
        }

        if (force || (last_csc_.average_speed_kmhX10 != data.average_speed_kmhX10))
        {
            INFO("Update, eCsc, average speed: %u\r\n", data.average_speed_kmhX10);
            DrawAverageSpeed(Y_LINE3, data.average_speed_kmhX10);
        }

        if (force || (last_csc_.cadence != data.cadence))
        {
            FLOW("Update, eCsc, cadence: %u\r\n", data.cadence);
            DrawCadence(0, Y_LINE2, data.filtered_cadence);
        }
        /*
        if (force || (last_csc_.average_cadence != data.average_cadence))
        {
            INFO("Update, eCsc, average_cadence: %u\r\n", data.average_cadence);
            DrawCadence(40, Y_LINE2, data.average_cadence);
        }
        */
        DrawCscToggleDisplay(data, force);

        break;
    case IUIMode::eKomoot:
    default:
        break;
    }
    last_csc_ = data;
}

void UIMain::Update(const IUIKomoot::KomootData_t &data, bool force)
{
    FLOW("UIMain::Update(Komoot), force=%d\r\n", force);
    SetOperational();
    if (IUIMode::eSettings == gui_mode_)
    {
        return;
    }

    if (data.distance_m <= uisettings_.config_.komoot_alert_dist)
    {
        bool switch_to_komoot = false;

        if (!switched_to_komoot_500_)
        {
            switched_to_komoot_500_ = true;
            switch_to_komoot = true;
        }

        if (!switched_to_komoot_100_ && (data.distance_m <= 100))
        {
            switched_to_komoot_100_ = true;
            switch_to_komoot = true;
        }

        if (last_komoot_.direction != data.direction)
        {
            switch_to_komoot = true;
        }

        if (0 != strcmp((const char *)last_komoot_.street, (const char *)data.street))
        {
            switch_to_komoot = true;
        }

        if (switch_to_komoot)
        {
            SetBacklightOn();
            switched_to_csc_ = false;

            if (uisettings_.config_.auto_switch)
            {
                SetUiMode(IUIMode::eKomoot);
            }
        }
    }
    else
    {
        if (!switched_to_csc_)
        {
            switched_to_csc_ = true;
            switched_to_komoot_500_ = false;
            switched_to_komoot_100_ = false;
            if (uisettings_.config_.auto_switch)
            {
                SetUiMode(IUIMode::eCsc);
            }
        }
    }

    switch (gui_mode_)
    {
    case IUIMode::eKomoot:

        if (force || (last_komoot_.direction != data.direction) || (last_direction_color_ != GetKomootDirectionColor(data.distance_m)))
        {
            DrawKomootDirection(data);
        }

        if (force || (0 != strcmp((const char *)last_komoot_.street, (const char *)data.street))) // new street => always show it
        {
            DrawKomootStreet(data);
            komoot_view_ = 0;
        }

        if (force || (last_komoot_.distance_m != data.distance_m))
        {
            DrawKomootDistance(data);
        }

    default:
        break;
    }
    last_komoot_ = data;
}

void UIMain::DrawKomootDistance(const IUIKomoot::KomootData_t &data)
{
    //tft_->fillRect(0, Y_KOMOOT_MIDDLE_START, 80, Y_KOMOOT_BOTTOM_START-Y_KOMOOT_MIDDLE_START, 0);
    char str[10] = {0};
    if (data.distance_m == 0)
    {
        strcpy(str, "now");
    }
    else if (data.distance_m <= 999)
    {
        sprintf(str, "%i", data.distance_m);
    }
    else
    {
        sprintf(str, "%i.%i", data.distance_m / 1000u, (data.distance_m % 1000u) / 100u);
    }
    tft_->setFont(&Open_Sans_Condensed_Bold_49);
    tft_->setTextColor(COLOR_WHITE);
    tft_->WriteStringLen(0, Y_KOMMOT_DISTANCE, 80, str, -1, 2, GFX::eCenter);
}

void UIMain::Log(const char *str)
{
    tft_->printf("%s", str);
}

void UIMain::SetOperational()
{
    if (NULL != uilog)
    {
        INFO("Disable log\r\n");
        uilog = NULL;
        SetUiMode(IUIMode::eCsc);
    }
}

void UIMain::SetUiMode(IUIMode::eUiMode_t mode)
{
    INFO("SetUiMode %d\r\n", mode);
    if (gui_mode_ != mode)
    {
        gui_mode_ = mode;
        tft_->fillScreen(ST77XX_BLACK);
        switch (gui_mode_)
        {
        case IUIMode::eKomoot:
            Update(last_komoot_, true);
            break;
        case IUIMode::eCsc:
            csc_toggle_view_ = eView2;
            Update(last_csc_, true);
            DrawCscConnState();
            event_queue_.call_in(uisettings_.config_.toggle_sec * 1000u, mbed::callback(this, &UIMain::ToggleCscDisplay));
            break;
        case IUIMode::eSettings:
            uisettings_.Activate();
            break;
        default:
            break;
        }
    }
}

void UIMain::DrawCscConnState()
{
    if (gui_mode_ == IUIMode::eCsc)
    {
        char str[10] = {0};
        switch (csc_conn_state_)
        {
        case eDisconnected:
            sprintf(str, "x%u", bike_computer_->GetCscDisconnects());
            break;
        case eConnecting:
            sprintf(str, ">%u", bike_computer_->GetCscDisconnects());
            break;
        case eConnected:
            sprintf(str, "c%u", bike_computer_->GetCscDisconnects());
            last_csc_.filtered_speed_kmhX10 = 0xFFFF;
            break;
        default:
            break;
        }

        if (0 != str[0])
        {
            tft_->fillRect(0, 0, 80, 40, 0);
            tft_->setFont(&Open_Sans_Condensed_Bold_31);
            tft_->setTextColor(COLOR_WHITE);
            tft_->WriteStringLen(0, 5, 80, str, -1, 0, GFX::eCenter);
        }
    }
}

void UIMain::DrawAverageSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color)
{
    char str[10];
    //speed_kmhX10 = 1444;
    sprintf(str, "%i.%i", speed_kmhX10 / 10, speed_kmhX10 % 10);

    tft_->setFont(&Open_Sans_Condensed_Light_37);
    tft_->setTextColor(color);
    tft_->WriteStringLen(0, y, 80, str, -1, 0, GFX::eRight);
}

void UIMain::DrawSpeed(uint16_t y, uint16_t speed_kmhX10, uint16_t color)
{
    char str[10];
    //speed_kmhX10 = 444;
    tft_->setTextColor(color);

    sprintf(str, " %i", speed_kmhX10 % 10);
    tft_->setFont(&Open_Sans_Condensed_Light_37);
    tft_->WriteStringLen(80 - 18, y + 0, 18, str, -1, 0, GFX::eLeft);

    sprintf(str, "%i", speed_kmhX10 / 10);
    tft_->setFont(&Open_Sans_Condensed_Bold_49);
    tft_->WriteStringLen(0, y, 60, str, -1, 1);
}

void UIMain::SetCadenceColor(uint16_t cadence)
{
    if (cadence < 80)
        tft_->setTextColor(COLOR_WHITE);
    else if (cadence > 95)
        tft_->setTextColor(COLOR_WARN);
    else
        tft_->setTextColor(COLOR_GREEN);
}

void UIMain::DrawCadence(uint16_t x, uint16_t y, uint16_t cadence)
{
    char str[10];
    sprintf(str, "%i", cadence);
    SetCadenceColor(cadence);
    tft_->setFont(&Open_Sans_Condensed_Light_37);
    tft_->WriteStringLen(0, y, 80, str);
}

void UIMain::DrawTime(uint16_t y, uint32_t trip_time_sec, uint16_t color)
{
    char str[14];
    uint8_t hour = trip_time_sec / 3660;
    uint8_t min = (trip_time_sec / 60) % 60;
    uint8_t sec = trip_time_sec % 60;
    sprintf(str, "%i:%.2i", hour, min);
    tft_->setTextColor(color);
    tft_->setFont(&Open_Sans_Condensed_Light_37);
    tft_->WriteStringLen(0, y, 80, str);
    //    tft_->WriteStringLen(0, y, 80, "144 144", -1);
}

void UIMain::DrawDistance(uint16_t y, uint32_t trip_distance_m, uint16_t color)
{
    char str[10];
    //trip_distance_m = 144440;
    //trip_distance_m = 44544;
    uint16_t km = trip_distance_m / 1000;
    if (trip_distance_m < 100000)
    {
        sprintf(str, "%i.%.2i", km, (trip_distance_m % 1000) / 10);
    }
    else
    {
        sprintf(str, "%i.%i", km, (trip_distance_m % 1000) / 100);
    }

    tft_->setTextColor(color);
    tft_->setFont(&Open_Sans_Condensed_Light_37);
    tft_->WriteStringLen(0, y, 80, str);
}

void UIMain::DrawKomootStreet(const IUIKomoot::KomootData_t &data)
{
    tft_->fillRect(0, Y_KOMMOT_STREET, 80, 160 - Y_KOMMOT_STREET, 0);
    tft_->setFont(&Open_Sans_Condensed_Bold_31);
    tft_->setTextColor(COLOR_WHITE);
    char street_ascii[20];
    ConvertUtf8toAscii(data.street, strlen((char *)data.street), street_ascii, sizeof(street_ascii));
    tft_->WriteString(0, Y_KOMMOT_STREET, 80, 160 - Y_KOMMOT_STREET, street_ascii);
}

uint16_t UIMain::GetKomootDirectionColor(uint16_t distance_m)
{
    uint16_t color = distance_m <= 100u ? COLOR_GREEN : COLOR_WHITE;
    return color;
}

void UIMain::DrawKomootDirection(const IUIKomoot::KomootData_t &data)
{
    const uint8_t *ptr = GetNavIcon(data.direction);
    if (ptr)
    {
        last_direction_color_ = GetKomootDirectionColor(data.distance_m);
        tft_->drawXBitmap2(10, 0, ptr, 60, 60, last_direction_color_);
    }
}

void UIMain::SetBacklightOff()
{
    led_event_id_ = 0;
    SetBacklightBrightness(uisettings_.config_.display_brightness_off);
}

void UIMain::SetBacklightOn()
{
    if (0 != led_event_id_)
    {
        event_queue_.cancel(led_event_id_);
    }
    led_event_id_ = event_queue_.call_in(uisettings_.config_.display_time * 1000, mbed::callback(this, &UIMain::SetBacklightOff));
    SetBacklightBrightness(uisettings_.config_.display_brightness_on);
}

void UIMain::SetBacklightBrightness(uint8_t val)
{
    display_led = (float)val / 10.0f;
}

void UIMain::UpdateCscConnState(ConState_t state)
{
    csc_conn_state_ = state;
    event_queue_.call(mbed::callback(this, &UIMain::DrawCscConnState));
}

void UIMain::SetBikeComputer(IBikeComputer *bike_computer)
{
    bike_computer_ = bike_computer;
    uisettings_.SetBikeComputer(bike_computer);
}

void UIMain::ToggleCscDisplay()
{
    if (IUIMode::eCsc == gui_mode_)
    {
        switch (csc_toggle_view_)
        {
        case eView1:
            csc_toggle_view_ = eView2;
            break;
        case eView2:
        default:
            csc_toggle_view_ = eView1;
            break;
        }
        DrawCscToggleDisplay(last_csc_, true);
        event_queue_.call_in(uisettings_.config_.toggle_sec * 1000u, mbed::callback(this, &UIMain::ToggleCscDisplay));
    }
}

void UIMain::DrawCscToggleDisplay(const IUICsc::CscData_t &data, bool force)
{
    uint16_t color = last_csc_.is_riding ? COLOR_WHITE : COLOR_WARN;
    uint16_t bar_color = COLOR_BLACK;

    switch (csc_toggle_view_)
    {
    case eView1:
        if (force || (last_csc_.trip_time_ms != data.trip_time_ms) || (last_csc_.is_riding != data.is_riding))
        {
            FLOW("Update, eCsc, trip time: %u\r\n", data.trip_time_ms);
            DrawTime(Y_LINE4, data.trip_time_ms / 1000, color);
        }
        break;

    case eView2:
    default:
        if (force || (last_csc_.trip_distance_cm != data.trip_distance_cm))
        {
            FLOW("Update, eCsc, trip distance: %u\r\n", data.trip_distance_cm);
            DrawDistance(Y_LINE4, data.trip_distance_cm / 100, color);
        }
        bar_color = COLOR_WHITE;
        break;
    }

    tft_->fillRect(0, Y_LINE4 - 6, 80, 2, bar_color);
}
