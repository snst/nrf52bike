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

//#define ENABLE_DEBUG_PRINT
//#define ENABLE_FLOW_PRINT
#define ENABLE_INFO_PRINT

#ifdef ENABLE_DEBUG_PRINT
#define DBG(...)             \
    {                        \
        printf(__VA_ARGS__); \
    }
#else
#define DBG(...)
#endif

#ifdef ENABLE_FLOW_PRINT
#define FLOW(...)            \
    {                        \
        printf(__VA_ARGS__); \
    }
#else
#define FLOW(...)
#endif

#ifdef ENABLE_INFO_PRINT
#define INFO(...)            \
    {                        \
        printf(__VA_ARGS__); \
    }
#else
#define INFO(...)
#endif

PinName TFT_CS = (PinName)8;
PinName TFT_DC = (PinName)6;
PinName TFT_MOSI = (PinName)2;
PinName TFT_MISO = (PinName)27; // disable
PinName TFT_SCLK = (PinName)3;
PinName TFT_RST = (PinName)7;

Serial pc(USBTX, USBRX); // tx, rx
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
//PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

Ticker ticker;
uint8_t speed_current = 0;
uint8_t speed_shown = 99;

enum state_t
{
    eDeviceDiscovery,
    eFoundDevice,
    eConnecting,
    eConnected,
    eServiceDiscovery,
    eFoundServiceCharacteristic_0x2A5B,
    eDiscoverCharacteristicDescriptors,
    eFoundCharacteristicDescriptor_0x2902,
    eRequestNotify,
    eRunning,
    eDisconnected
};

Gap::Handle_t connection_handle = 0xFFFF;
static DiscoveredCharacteristic characteristic_0x2A5B;
static DiscoveredCharacteristicDescriptor descriptor_0x2902(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static BLEProtocol::AddressBytes_t device_addr;
state_t state = eDeviceDiscovery;

struct bikeResponse_t
{
    uint8_t flags;
    uint32_t wheelCounter;
    uint16_t lastWheelEvent;
    uint16_t crankCounter;
    uint16_t lastCrankEvent;
};

static struct bikeResponse_t bikeResponse;

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

Timer t;

void process_data(const uint8_t *data, uint32_t len)
{
    //static const uint16_t MAX_BYTES = (1 + 4 + 2 + 2 + 2);

    //00 04 00 20 e9 08 00 00 7d 05 00 00 c9 08

    //03 be 09 00 00 c9 4d 9e 00 b5 50

    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027

    if (len == 11)
    {
        static struct bikeResponse_t r = {0};

        uint8_t p = 0;
        r.flags = data[p++];

        if (r.flags & FLAG_WHEEL_PRESENT)
        {
            r.wheelCounter = data[p++];
            r.wheelCounter |= data[p++] << 8;
            r.wheelCounter |= data[p++] << 16;
            r.wheelCounter |= data[p++] << 24;
            r.lastWheelEvent = data[p++];
            r.lastWheelEvent |= data[p++] << 8;
        }
        if (r.flags & FLAG_CRANK_PRESENT)
        {
            r.crankCounter = data[p++];
            r.crankCounter |= data[p++] << 8;
            r.lastCrankEvent = data[p++];
            r.lastCrankEvent |= data[p++] << 8;
        }

        static uint32_t last_timestamp = 0;
        uint32_t now = t.read_ms();
        uint32_t delta_ms = now - last_timestamp;

        uint32_t crank_counter_diff = r.crankCounter - bikeResponse.crankCounter;
        uint32_t wheel_counter_diff = r.wheelCounter - bikeResponse.wheelCounter;

        INFO("now: %ums, diff=%ums\r\n", now, delta_ms);
        FLOW("val: wc=%u, we=%u, cc=%u, ce=%u\r\n", r.wheelCounter, r.lastWheelEvent, r.crankCounter, r.lastCrankEvent);
        FLOW("dif: wc=%u, we=%u, cc=%u, ce=%u\r\n",
             wheel_counter_diff,
             r.lastWheelEvent - bikeResponse.lastWheelEvent,
             crank_counter_diff,
             r.lastCrankEvent - bikeResponse.lastCrankEvent);

        float wheel = 2.170f;

        float crank_per_min = ((float)crank_counter_diff) * 60000.0f / delta_ms;

        float wheel_per_min = (((float)wheel_counter_diff) * 60000.0f / delta_ms);
        float kmh = wheel * wheel_per_min * 60.0f / 1000.0f;
        speed_current = (uint8_t) kmh;

        INFO("result: %f kmh, cadence=%f/min\r\n\r\n", kmh, wheel_per_min, crank_per_min);

        last_timestamp = now;
        bikeResponse = r;

        //BLE::Instance().disconnect(Gap::REMOTE_USER_TERMINATED_CONNECTION);
    }
}

void AdvertisementCB(const Gap::AdvertisementCallbackParams_t *params)
{
    if (state == eDeviceDiscovery)
    {
        DBG("~AdvertisementCB peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u.\r\n",
            params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
            params->rssi, params->isScanResponse, params->type, params->peerAddrType);

        if (params->peerAddr[5] == 0xf4)
        {
            FLOW("~AdvertisementCB() -> eFoundDevice\r\n");
            BLE::Instance().stopScan();
            memcpy(&device_addr, &params->peerAddr, sizeof(device_addr));
            state = eFoundDevice;
        }
    }
}

void ServiceDiscoveryCB(const DiscoveredService *service)
{
    DBG("~ServiceDiscoveryCB()\r\n");
}

void ReadCB(const GattReadCallbackParams *params)
{
    FLOW("~ReadCB() len=%u\r\n", params->len);
}

void ServiceCharacteristicsCB(const DiscoveredCharacteristic *param)
{
    if (state == eServiceDiscovery)
    {
        DBG("~ServiceCharacteristicsCB() UUID-%x valueAttr[%u] props[%x]\r\n", param->getUUID().getShortUUID(), param->getValueHandle(), (uint8_t)param->getProperties().broadcast());
        if (param->getUUID().getShortUUID() == 0x2A5B)
        {
            FLOW("~ServiceCharacteristicsCB() -> eFoundServiceCharacteristic_0x2A5B\r\n");
            characteristic_0x2A5B = *param;
            BLE::Instance().gattClient().terminateServiceDiscovery();
            state = eFoundServiceCharacteristic_0x2A5B;
        }
    }
}

static void DiscoveredDescCB(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    if (state == eDiscoverCharacteristicDescriptors)
    {
        DBG("~DiscoveredDescCB(), UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
        if (params->descriptor.getUUID().getShortUUID() == 0x2902)
        {
            FLOW("~DiscoveredDescCB() -> eFoundCharacteristicDescriptor_0x2902\r\n");
            descriptor_0x2902 = params->descriptor;
            BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
            state = eFoundCharacteristicDescriptor_0x2902;
        }
    }
}

static void DiscoveredDescTerminationCB(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~DiscoveredDescTerminationCB()\r\n");
    if (state == eFoundCharacteristicDescriptor_0x2902)
    {
        FLOW("~DiscoveredDescTerminationCB() [eFoundCharacteristicDescriptor_0x2902] => eRequestNotify\r\n");
        state = eRequestNotify;
    }
}

void DiscoverCharacteristicDescriptors()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_0x2A5B, DiscoveredDescCB, DiscoveredDescTerminationCB);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors;
    }
}

void ServiceDiscoveryTerminationCB(Gap::Handle_t handle)
{
    DBG("~ServiceDiscoveryTerminationCB()\r\n");
    if (state == eFoundServiceCharacteristic_0x2A5B)
    {
        FLOW("~ServiceDiscoveryTerminationCB() [eFoundServiceCharacteristic_0x2A5B] => DiscoverCharacteristicDescriptors()\r\n");
        DiscoverCharacteristicDescriptors();
    }
}

void RequestNotify()
{
    if (!BLE::Instance().gattClient().isCharacteristicDescriptorDiscoveryActive(characteristic_0x2A5B))
    {
        uint16_t value = BLE_HVX_NOTIFICATION;
        ble_error_t err = BLE::Instance().gattClient().write(
            GattClient::GATT_OP_WRITE_CMD,
            descriptor_0x2902.getConnectionHandle(),
            descriptor_0x2902.getAttributeHandle(),
            sizeof(uint16_t),
            (uint8_t *)&value);
        FLOW("~RequestNotify() ret=0x%x\r\n", err);
    }
}

void ReadNotifyStatus()
{
    ble_error_t err = BLE::Instance().gattClient().read(
        descriptor_0x2902.getConnectionHandle(),
        descriptor_0x2902.getAttributeHandle(),
        0);
    FLOW("~ReadNotifyStatus() ret=0x%x\r\n", err);
}

void DataReadCB(const GattReadCallbackParams *params)
{
    INFO("~DataReadCB(): handle %u, len=%u, ", params->handle, params->len);
    for (unsigned index = 0; index < params->len; index++)
    {
        INFO(" %02x", params->data[index]);
    }
    INFO("\r\n");
}

void DataWriteCB(const GattWriteCallbackParams *params)
{
    INFO("~DataWriteCB(): handle %u, len=%u, status=0x%x, err=0x%x", params->handle, params->len, params->status, params->error_code);
    for (unsigned index = 0; index < params->len; index++)
    {
        INFO(" %02x", params->data[index]);
    }
    INFO("\r\n");
}

void ConnectionCB(const Gap::ConnectionCallbackParams_t *params)
{
    DBG("~ConnectionCB() handle=0x%x\r\n", params->handle);
    if (params->role == Gap::CENTRAL)
    {
        connection_handle = params->handle;
        INFO("~ConnectionCB() handle=0x%x -> eConnected\r\n", connection_handle);
        state = eConnected;
    }
}

void StartServiceDiscovery()
{
    FLOW("~StartServiceDiscovery() -> eServiceDiscovery\r\n")
    BLE::Instance().gattClient().onServiceDiscoveryTermination(ServiceDiscoveryTerminationCB);
    BLE::Instance().gattClient().launchServiceDiscovery(connection_handle, ServiceDiscoveryCB, ServiceCharacteristicsCB /*, 0xa000, 0xa001*/);
    state = eServiceDiscovery;
}

void DisconnectionCB(const Gap::DisconnectionCallbackParams_t *param)
{
    INFO("~DisconnectionCB() reason=0x -> eDisconnected, reason = 0x%x\r\n", param->reason);
    state = eDisconnected;
}

void hvxCB(const GattHVXCallbackParams *params)
{
    FLOW("~hvxCB(): handle %u; type %s, ", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
    for (unsigned index = 0; index < params->len; index++)
    {
        FLOW(" %02x", params->data[index]);
    }
    FLOW("\r\n");

    if (params->type == BLE_HVX_NOTIFICATION)
    {
        process_data(params->data, params->len);
    }
    FLOW("\r\n");
}

static void Connect()
{
    FLOW("~Connect() -> eConnecting\r\n")
    BLE::Instance().gap().connect(device_addr, BLEProtocol::AddressType::PUBLIC, NULL, NULL);
    state = eConnecting;
}

int32_t g_i = 0;
void Tick()
{
    //tft.printf("%i",i);
    printf("%i\r\n", g_i);
    g_i++;
}

int main(void)
{
    INFO("+main()\r\n");
    t.start();

    //tft.initB();
    tft.initR(INITR_MINI160x80);
    //  tft.fillRect(0,0,80,160, ST77XX_GREEN);
    //tft.fillRect(1,80,81,2, ST77XX_RED);
    //tft.fillRect(0,82,81,2, ST77XX_BLUE);

    tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont(&FreeMonoBold24pt7b);
    //tft.setTextSize(3);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));

//    ticker.attach(&Tick, 1);

    BLE &ble = BLE::Instance();
    ble.init();
    ble.gap().onConnection(ConnectionCB);
    ble.gap().onDisconnection(DisconnectionCB);

    Gap::ConnectionParams_t connParams;
    connParams.minConnectionInterval = 0xFFFF; //BLE_GAP_CP_MIN_CONN_INTVL_MIN;

    // set the maximum we'd prefer such that we can get 3 packets accross in time
    // 1000 ms * 4 samples/packet * 3 packets per interval / (1.25 ms units * Freq)
    // at 300 Hz, this gives 32 units of 1.25ms (so 40ms, or 25 connection events/s)
    connParams.maxConnectionInterval = 0xFFFF;
    //        ((100000ull * 4 * 3) / (125 * SAMPLING_FREQUENCY_HZ));

    // other stuff... whatevs
    connParams.slaveLatency = 0x01F3;                 //BLE_GAP_CP_SLAVE_LATENCY_MAX;
    connParams.connectionSupervisionTimeout = 0x0C80; //BLE_GAP_CP_CONN_SUP_TIMEOUT_MAX / 2;
    connParams.connectionSupervisionTimeout = 0;      //BLE_GAP_CP_CONN_SUP_TIMEOUT_MAX / 2;

    // now, actually set your preferences
    //ble.gap().setPreferredConnectionParams(&connParams);

    ble.gap().setScanParams(500, 400, 0, false);
    ble.gap().startScan(AdvertisementCB);

    ble.gattClient().onHVX(hvxCB);
    ble.gattClient().onDataRead(DataReadCB);
    ble.gattClient().onDataWrite(DataWriteCB);

    while (true)
    {
        switch (state)
        {
        case eDeviceDiscovery:
            break;
        case eFoundDevice:
            Connect();
            break;
        case eConnecting:
            break;
        case eConnected:
            StartServiceDiscovery();
            break;
        case eServiceDiscovery:
            break;
        case eFoundServiceCharacteristic_0x2A5B:
            // Wait for ServiceDiscoveryTerminationCB
            break;
        case eDiscoverCharacteristicDescriptors:
            break;
        case eFoundCharacteristicDescriptor_0x2902:
            break;
        case eRequestNotify:
            RequestNotify();
            state = eRunning;
            break;
        case eRunning:
            break;
        case eDisconnected:
            Connect();
            break;
        }

        if (speed_shown != speed_current)
        {
            speed_shown = speed_current;
            tft.fillRect(0,0,80,50, ST77XX_BLACK);
            tft.setCursor(10, 40);
            tft.printf("%d", speed_shown);
        }

        ble.waitForEvent();
    }
}
