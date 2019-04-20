#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>

#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "gfxfont.h"
#include "FreeMonoBold24pt7b.h"
#include <Adafruit_ST7735.h>
#include "tracer.h"
#include "BikeComputer.h"

Serial pc(USBTX, USBRX); // tx, rx
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
//PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

void initDisplay()
{
    tft.initR(INITR_MINI160x80);
    //  tft.fillRect(0,0,80,160, ST77XX_GREEN);
    //tft.fillRect(1,80,81,2, ST77XX_RED);
    //tft.fillRect(0,82,81,2, ST77XX_BLUE);

    tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont(&FreeMonoBold24pt7b);
    //tft.setTextSize(3);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));
}

int main()
{
    INFO("+main()\r\n");
    initDisplay();

    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;

    GattClientProcess gatt_client_process(ble);
    gatt_client_process.init(&event_queue);

    // Process the event queue.
    event_queue.dispatch_forever();
    return 0;
}
