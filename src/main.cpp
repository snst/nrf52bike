#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>
#include "tracer.h"
#include "BLEManager.h"
#include "BikeGUI.h"

Serial pc(USBTX, USBRX);

int main(void)
{
    INFO("+main()\r\n");
    BLE &ble = BLE::Instance();
    BikeGUI gui;
    BLEManager mgr(ble, &gui);
    mgr.Start();

    return 0;
}
