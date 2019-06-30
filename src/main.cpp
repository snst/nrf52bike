#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>
#include "tracer.h"
#include "BikeComputer.h"
#include "UIMain.h"
#include "common.h"
#include "pin_config.h"

// BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST

Serial pc(BC_TX, BC_RX);
InterruptIn touch(BC_TOUCH);

int main(void)
{
    INFO("+main()\r\n");
    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;

    StartTimer();
    Adafruit_ST7735 tft = Adafruit_ST7735(BC_TFT_MOSI, BC_TFT_MISO, BC_TFT_SCLK, BC_TFT_CS, BC_TFT_DC, BC_TFT_RST);
    tft.initR(INITR_MINI160x80);

    UIMain ui(&tft, event_queue);
    touch.fall(event_queue.event(mbed::callback(&ui, &UIMain::TouchDown)));
    touch.rise(event_queue.event(mbed::callback(&ui, &UIMain::TouchUp)));
    
    BikeComputer bc(ble, event_queue, ui);
    bc.Start();
    
    event_queue.dispatch_forever();

    return 0;
}
