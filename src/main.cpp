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
#include "val.h"
#include "csc.h"
#include "icons.h"

// https://github.com/jstiefel/esp32_komoot_ble

Serial pc(USBTX, USBRX); // tx, rx
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);
//PinName mosi, PinName miso, PinName sck, PinName CS, PinName RS, PinName RST

enum state_t
{
    eDeviceDiscovery,
    eFoundDevice,
    eConnecting,
    eConnected,
    eServiceDiscovery,
    eDiscoverCharacteristicDescriptors_komoot,
    eDiscoverCharacteristicDescriptors_csc,
    eDiscoverCharacteristicDescriptors_bat,
    eDiscoverCharacteristicDescriptorsEnd,
    eFoundCharacteristicDescriptor_komoot_0x2902,
    eFoundCharacteristicDescriptor_csc_0x2902,
    eFoundCharacteristicDescriptor_bat_0x2902,
    eRequestNotify,
    eRunning,
    eReadBat,
    eDisconnected
};

Gap::Handle_t connection_handle_bike = 0xFFFF;
static DiscoveredCharacteristic characteristic_komoot;
static DiscoveredCharacteristic characteristic_csc;
static DiscoveredCharacteristic characteristic_bat;
static DiscoveredCharacteristicDescriptor descriptor_komoot(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static DiscoveredCharacteristicDescriptor descriptor_csc(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static DiscoveredCharacteristicDescriptor descriptor_bat(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static BLEProtocol::AddressBytes_t device_addr_bike;
static BLEProtocol::AddressType_t device_addr_type_bike;
static BLEProtocol::AddressBytes_t device_addr_komoot;
state_t state = eDeviceDiscovery;
static bool found_char_komoot = false; //
static bool found_char_csc = false;    // 2a5b
static bool found_char_bat = false;    // 2a19

static bool found_desc_komoot = false;
static bool found_desc_csc = false;
static bool found_desc_bat = false;

static bool read_bat_result = false;
static bool have_addr_bike = false;
static bool have_addr_komoot = false;

val_uint8_t bat;
val_uint8_t speed;
Timer t;

void AdvertisementCB(const Gap::AdvertisementCallbackParams_t *params)
{
    if ((params->peerAddr[5] == 0xd4) && (params->peerAddr[4] == 0x3f))
        return;
    if ((params->peerAddr[5] == 0x00) && (params->peerAddr[4] == 0x1a))
        return;

    INFO("~AdvertisementCB peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u.\r\n",
         params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
         params->rssi, params->isScanResponse, params->type, params->peerAddrType);

    //if (params->peerAddr[5] == 0xf4)
    {
        INFO("  -> found bike\r\n");
        memcpy(&device_addr_bike, &params->peerAddr, sizeof(device_addr_bike));
        device_addr_type_bike = params->addressType;
        have_addr_bike = true;
    }

    if (params->peerAddr[5] == 0x58 || params->peerAddr[0] == 0x58)
    {
        INFO("  -> found komoot\r\n");
        memcpy(&device_addr_bike, &params->peerAddr, sizeof(device_addr_bike));
        have_addr_komoot = true;
    }

    if (have_addr_bike)
    {
        BLE::Instance().stopScan();
        state = eFoundDevice;
    }
}

void ServiceDiscoveryCB(const DiscoveredService *service)
{
    DBG("~ServiceDiscoveryCB() UUID-%x\r\n", service->getUUID().getShortUUID());
    DBG("~ServiceDiscoveryCB() LONG-%x %x %x %x %x\r\n", service->getUUID().getBaseUUID()[0], service->getUUID().getBaseUUID()[1], service->getUUID().getBaseUUID()[2], service->getUUID().getBaseUUID()[3], service->getUUID().getBaseUUID()[4]

    );
    // komoot UUID-e128
}

void ReadCB(const GattReadCallbackParams *params)
{
    DBG("~ReadCB() len=%u\r\n", params->len);
}

void ServiceCharacteristicsCB(const DiscoveredCharacteristic *param)
{
    FLOW("~ServiceCharacteristicsCB() UUID-%x valueHandle=%u, declHandle=%u, props[%x]\r\n", param->getUUID().getShortUUID(), param->getValueHandle(), param->getDeclHandle(), (uint8_t)param->getProperties().broadcast());
    DBG("~ServiceCharacteristicsCB() LONG-%x %x %x %x %x\r\n", param->getUUID().getBaseUUID()[0], param->getUUID().getBaseUUID()[1], param->getUUID().getBaseUUID()[2], param->getUUID().getBaseUUID()[3], param->getUUID().getBaseUUID()[4]

    );

    if (param->getUUID().getShortUUID() == 0xd605)
    {
        INFO(" -> found_char_komoot\r\n");
        characteristic_komoot = *param;
        found_char_komoot = true;
    }
    else if (param->getUUID().getShortUUID() == 0x2A5B)
    {
        INFO(" -> found_char_csc\r\n");
        characteristic_csc = *param;
        found_char_csc = true;
    }
    else if (param->getUUID().getShortUUID() == 0x2A19) // battery
    {
        INFO(" -> found_char_bat\r\n");
        characteristic_bat = *param;
        found_char_bat = true;
    }
}

static void DiscoveredDescCB(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    if (state == eDiscoverCharacteristicDescriptors_komoot)
    {
        DBG("~DiscoveredDescCB() komoot, UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
        if (params->descriptor.getUUID().getShortUUID() == 0x2902)
        {
            DBG("  -> eFoundCharacteristicDescriptor_komoot_0x2902\r\n");
            descriptor_komoot = params->descriptor;
            BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
            state = eFoundCharacteristicDescriptor_komoot_0x2902;
        }
    }
    else if (state == eDiscoverCharacteristicDescriptors_csc)
    {
        DBG("~DiscoveredDescCB(), UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
        if (params->descriptor.getUUID().getShortUUID() == 0x2902)
        {
            DBG("  -> eFoundCharacteristicDescriptor_csc_0x2902\r\n");
            descriptor_csc = params->descriptor;
            BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
            state = eFoundCharacteristicDescriptor_csc_0x2902;
        }
    }
    else if (state == eDiscoverCharacteristicDescriptors_bat)
    {
        DBG("~DiscoveredDescCB(), UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
        if (params->descriptor.getUUID().getShortUUID() == 0x2902)
        {
            DBG("  -> eFoundCharacteristicDescriptor_bat_0x2902\r\n");
            descriptor_bat = params->descriptor;
            BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
            state = eFoundCharacteristicDescriptor_bat_0x2902;
        }
    }
}

static void DiscoverCharacteristicDescriptorsEndCB(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~DiscoverCharacteristicDescriptorsEndCB()\r\n");
    //    if (state == eFoundCharacteristicDescriptor_csc_0x2902)
    if (state == eFoundCharacteristicDescriptor_komoot_0x2902)
    {
        state = eDiscoverCharacteristicDescriptorsEnd;
    }
}

void DiscoverCharacteristicDescriptors_bat()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_bat, DiscoveredDescCB, DiscoverCharacteristicDescriptorsEndCB);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_bat, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_bat;
    }
}

void DiscoverCharacteristicDescriptors_csc()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_csc, DiscoveredDescCB, DiscoverCharacteristicDescriptorsEndCB);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_csc, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_csc;
    }
}

void DiscoverCharacteristicDescriptors_komoot()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_komoot, DiscoveredDescCB, DiscoverCharacteristicDescriptorsEndCB);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_komoot, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_komoot;
    }
}

void ServiceDiscoveryTerminationCB(Gap::Handle_t handle)
{
    DBG("~ServiceDiscoveryTerminationCB()\r\n");
    //    DiscoverCharacteristicDescriptors_csc();
    DiscoverCharacteristicDescriptors_komoot();
}

void RequestNotify(DiscoveredCharacteristicDescriptor &desc, bool enable)
{
    uint16_t value = enable ? BLE_HVX_NOTIFICATION : 0;
    ble_error_t err = BLE::Instance().gattClient().write(
        GattClient::GATT_OP_WRITE_CMD,
        desc.getConnectionHandle(),
        desc.getAttributeHandle(),
        sizeof(uint16_t),
        (uint8_t *)&value);
    DBG("~RequestNotify(%d) attrHandle=%u, ret=0x%x\r\n", value, desc.getAttributeHandle(), err);
}

void ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc)
{
    ble_error_t err = BLE::Instance().gattClient().read(
        desc.getConnectionHandle(),
        desc.getAttributeHandle(),
        0);
    DBG("~ReadNotifyStatus() ret=0x%x\r\n", err);
}

void DataReadCB(const GattReadCallbackParams *params)
{
    FLOW("~DataReadCB(): handle %u, len=%u, ", params->handle, params->len);
    for (unsigned index = 0; index < params->len; index++)
    {
        FLOW(" %02x", params->data[index]);
    }
    FLOW("\r\n");

    if ((params->handle == characteristic_bat.getValueHandle()) && (1 == params->len))
    {
        static int i = 0;
        bat.current = params->data[0];
        INFO("Got Battery #%u: %u\r\n", i++, bat.current);
        read_bat_result = true;
    }
}

void DataWriteCB(const GattWriteCallbackParams *params)
{
    FLOW("~DataWriteCB(): handle %u, len=%u, status=0x%x, err=0x%x", params->handle, params->len, params->status, params->error_code);
    for (unsigned index = 0; index < params->len; index++)
    {
        FLOW(" %02x", params->data[index]);
    }
    FLOW("\r\n");
}

void ConnectionCB(const Gap::ConnectionCallbackParams_t *params)
{
    FLOW("~ConnectionCB() handle=0x%x\r\n", params->handle);
    if (params->role == Gap::CENTRAL)
    {
        connection_handle_bike = params->handle;
        state = eConnected;
    }
}

void StartServiceDiscovery()
{
    FLOW("~StartServiceDiscovery()\r\n")
    BLE::Instance().gattClient().onServiceDiscoveryTermination(ServiceDiscoveryTerminationCB);
    BLE::Instance().gattClient().launchServiceDiscovery(connection_handle_bike, ServiceDiscoveryCB, ServiceCharacteristicsCB /*, 0xa000, 0xa001*/);
    state = eServiceDiscovery;
}

void DisconnectionCB(const Gap::DisconnectionCallbackParams_t *param)
{
    INFO("~DisconnectionCB() reason=0x%x\r\n", param->reason);
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

    if ((params->type == BLE_HVX_NOTIFICATION) && (params->handle == characteristic_csc.getValueHandle()))
    {
        process_csc_data(params->data, params->len, t, speed);
    }
}

static void Connect()
{
    FLOW("~Connect()\r\n")
    BLE::Instance().gap().connect(device_addr_bike, device_addr_type_bike, NULL, NULL);
    state = eConnecting;
}

static void ReadBat()
{
    DBG("+ReadBat(), handle=%u\r\n", characteristic_bat.getValueHandle());
    ble_error_t err = characteristic_bat.read(0);
    DBG("ReadBat(), err=0x%x\r\n", err);
    state = eReadBat;
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
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));

    BLE &ble = BLE::Instance();
    ble.init();
    ble.gap().onConnection(ConnectionCB);
    ble.gap().onDisconnection(DisconnectionCB);

    ble.gap().setScanParams(400, 400, 0, true);
    ble.gap().startScan(AdvertisementCB);

    ble.gattClient().onHVX(hvxCB);
    ble.gattClient().onDataRead(DataReadCB);
    ble.gattClient().onDataWrite(DataWriteCB);

    uint32_t lms = t.read_ms();
    uint8_t ic = 0;

    while (true)
    {
        uint32_t now = t.read_ms();
        if (now - lms > 1000)
        {
            lms = now;
            const uint8_t* ptr;
            do {
                ptr = GetNavIcon(ic);
                ic++;
            } while(!ptr);
            if (ptr)
            {
                //tft.fillScreen(ST77XX_BLACK);
                tft.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
            }
        }

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
            if (!found_char_csc || !found_char_bat)
            {
                StartServiceDiscovery();
            }
            else
            {
                state = eRequestNotify;
            }
            break;
        case eServiceDiscovery:
            //            if (found_char_csc && found_char_bat)
            if (found_char_komoot)
            {
                BLE::Instance().gattClient().terminateServiceDiscovery();
            }
            break;
        case eDiscoverCharacteristicDescriptors_csc:
            break;
        case eDiscoverCharacteristicDescriptors_bat:
            break;
        case eFoundCharacteristicDescriptor_csc_0x2902:
            break;
        case eDiscoverCharacteristicDescriptorsEnd:
            RequestNotify(descriptor_komoot, true);
            state = eRunning;
            //ReadBat();
            break;
        case eReadBat:
            if (read_bat_result)
            {
                state = eRequestNotify;
            }
            break;
        case eRequestNotify:
            RequestNotify(descriptor_csc, true);
            state = eRunning;
            break;
        case eRunning:
        {
        }
        break;
        case eDisconnected:
            Connect();
            break;
        }

        if (new_val(speed))
        {
            tft.fillRect(0, 0, 80, 50, ST77XX_BLACK);
            tft.setCursor(10, 40);
            tft.printf("%d", speed.shown);
        }

        ble.waitForEvent();
    }
}
