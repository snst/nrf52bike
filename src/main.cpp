#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>
#include "tracer.h"
#include "BikeComputer.h"
#include "UIMain.h"
#include "common.h"

// BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST

Serial pc(USBTX, USBRX);
InterruptIn touch(P0_26);


int main(void)
{
    INFO("+main()\r\n");
    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;

    StartTimer();
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
    tft.initR(INITR_MINI160x80);

    UIMain ui(&tft, event_queue);
    touch.fall(event_queue.event(mbed::callback(&ui, &UIMain::TouchDown)));
    touch.rise(event_queue.event(mbed::callback(&ui, &UIMain::TouchUp)));
    
    BikeComputer mgr(ble, event_queue, &ui);
    
    mgr.Start();
    
    event_queue.dispatch_forever();

    return 0;
}
