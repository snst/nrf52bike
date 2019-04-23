#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>
#include "tracer.h"
#include "BLEManager.h"
#include "BikeGui.h"

Serial pc(USBTX, USBRX); // tx, rx

int main(void)
{
    INFO("+main()\r\n");
    BLE &ble = BLE::Instance();
    BikeGUI gui;
    BLEManager mgr(ble, &gui);
    mgr.Start();

    return 0;
}
