#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>

#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "tracer.h"
#include "val.h"
#include "csc.h"
#include "komoot.h"
#include "gap/AdvertisingDataParser.h"
#include "BLEAppBase.h"
#include "BLEManager.h"
#include "BikeGui.h"

// https://github.com/jstiefel/esp32_komoot_ble

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



#if 0

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

typedef enum state_e
{
    eDeviceDiscovery,
    eFoundDevice,
    eConnectingStart,
    eConnectingKomoot,
    eConnectingBike,
    eConnectedKomoot,
    eConnectedBike,
    eServiceDiscoveryKomoot,
    eServiceDiscoveryBike,
    eServiceDiscoveredKomoot,
    eServiceDiscoveredBike,
    eDiscoverCharacteristicDescriptors_komoot,
    eDiscoverCharacteristicDescriptors_csc,
    eDiscoverCharacteristicDescriptors_bat,
    eDiscoverCharacteristicDescriptorsEnd_komoot,
    eDiscoverCharacteristicDescriptorsEnd_csc,
    eDiscoverCharacteristicDescriptorsEnd_bat,
    eFoundCharacteristicDescriptor_komoot_0x2902,
    eFoundCharacteristicDescriptor_csc_0x2902,
    eFoundCharacteristicDescriptor_bat_0x2902,
    eRequestNotify,
    eRunning,
    eReadBat,
    eDisconnectedAll
} state_t;

typedef enum targetState_e
{
    eNotFound,
    eFound,
    eDisconnected,
    eConnecting,
    eConnected
} targetState_t;

Gap::Handle_t connection_handle_bike = 0xFFFF;
Gap::Handle_t connection_handle_komoot = 0xFFFF;
static DiscoveredCharacteristic characteristic_komoot;
static DiscoveredCharacteristic characteristic_csc;
static DiscoveredCharacteristic characteristic_bat;
static DiscoveredCharacteristicDescriptor descriptor_komoot(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static DiscoveredCharacteristicDescriptor descriptor_csc(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static DiscoveredCharacteristicDescriptor descriptor_bat(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));
static BLEProtocol::AddressBytes_t device_addr_bike;
static BLEProtocol::AddressType_t device_addr_type_bike;
static BLEProtocol::AddressBytes_t device_addr_komoot;
static BLEProtocol::AddressType_t device_addr_type_komoot;

targetState_t state_bike = eNotFound;
targetState_t state_komoot = eNotFound;

state_t state = eDeviceDiscovery;
static bool found_char_komoot = false; //
static bool found_char_csc = false;    // 2a5b
static bool found_char_bat = false;    // 2a19

static bool found_desc_komoot = false;
static bool found_desc_csc = false;
static bool found_desc_bat = false;

static bool read_bat_result = false;

val_uint8_t bat;
val_uint8_t speed;
val_uint8_t komoot_dir;
val_uint32_t komoot_dist;
Timer t;
uint32_t start_scan_ms;

DigitalOut display_led((PinName)11);

bool HasServiceId(mbed::Span<const uint8_t> &data, uint16_t id)
{
    //INFO("FIELD len: %u\r\n", data.size());
    const uint8_t *p = data.data();
    uint32_t val;
    for (size_t i = 0; i < data.size() / sizeof(id); i++)
    {
        val = *p++;
        val |= (*p++ << 8);
        if (val == id)
        {
            return true;
        }
    }
    return false;
}

bool IsServiceId128(mbed::Span<const uint8_t> &data, const uint8_t id[])
{
    if (data.size() < 16)
        return false;

    const uint8_t *p = data.data();
    for (uint8_t i = 0; i < 16; i++)
    {
        if (p[i] != id[15 - i])
        {
            return false;
        }
    }
    return true;
}

void AdvertisementCB(const Gap::AdvertisementCallbackParams_t *params)
{
    if ((params->peerAddr[5] == 0xd4) && (params->peerAddr[4] == 0x3f))
        return;
    if ((params->peerAddr[5] == 0x00) && (params->peerAddr[4] == 0x1a))
        return;

    INFO("~AdvertisementCB peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u, len=%u.\r\n",
         params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
         params->rssi, params->isScanResponse, params->type, params->peerAddrType, params->advertisingDataLen);

    mbed::Span<const uint8_t> ad(params->advertisingData, params->advertisingDataLen);

    ble::AdvertisingDataParser adv_parser(ad);

    bool isBat = false;
    bool isCsc = false;
    bool isKomoot = false;

    while (adv_parser.hasNext())
    {
        ble::AdvertisingDataParser::element_t field = adv_parser.next();
        if ((field.type == ble::adv_data_type_t::INCOMPLETE_LIST_16BIT_SERVICE_IDS) || (field.type == ble::adv_data_type_t::COMPLETE_LIST_16BIT_SERVICE_IDS))
        {
            isBat |= HasServiceId(field.value, GAT_SERVICE_BAT);
            isCsc |= HasServiceId(field.value, GAT_SERVICE_CSCS);
        }

        if ((field.type == ble::adv_data_type_t::INCOMPLETE_LIST_128BIT_SERVICE_IDS) || (field.type == ble::adv_data_type_t::COMPLETE_LIST_128BIT_SERVICE_IDS))
        {
            isKomoot |= IsServiceId128(field.value, GAT_SERVICE_KOMOOT);
        }
    }

    if (isCsc && (params->peerAddr[5] == 0xf4))
    {
        memcpy(&device_addr_bike, &params->peerAddr, sizeof(device_addr_bike));
        device_addr_type_bike = params->addressType;
        state_bike = eFound;
    }

    if (isKomoot)
    {
        memcpy(&device_addr_komoot, &params->peerAddr, sizeof(device_addr_komoot));
        device_addr_type_komoot = params->addressType;
        state_komoot = eFound;
    }

    if (((state_bike == eFound) && (state_komoot == eFound)) || ((t.read_ms() - start_scan_ms) > 10000))
    {
        INFO("found bike   : %d\r\n", state_bike);
        INFO("found komoot : %d\r\n", state_komoot);
        BLE::Instance().stopScan();
        state = eFoundDevice;
    }
}

void OnServiceFound(const DiscoveredService *service)
{
    DBG("~OnServiceFound() UUID-%x\r\n", service->getUUID().getShortUUID());
    DBG("~OnServiceFound() LONG-%x %x %x %x %x\r\n", service->getUUID().getBaseUUID()[0], service->getUUID().getBaseUUID()[1], service->getUUID().getBaseUUID()[2], service->getUUID().getBaseUUID()[3], service->getUUID().getBaseUUID()[4]);
    // komoot UUID-e128
}

void ReadCB(const GattReadCallbackParams *params)
{
    DBG("~ReadCB() len=%u\r\n", params->len);
}

void OnServiceCharacteristicFound(const DiscoveredCharacteristic *param)
{
    FLOW("~OnServiceCharacteristicFound() UUID-%x valueHandle=%u, declHandle=%u, props[%x]\r\n", param->getUUID().getShortUUID(), param->getValueHandle(), param->getDeclHandle(), (uint8_t)param->getProperties().broadcast());
    DBG("~OnServiceCharacteristicFound() LONG-%x %x %x %x %x\r\n", param->getUUID().getBaseUUID()[0], param->getUUID().getBaseUUID()[1], param->getUUID().getBaseUUID()[2], param->getUUID().getBaseUUID()[3], param->getUUID().getBaseUUID()[4]);

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

static void OnDescFoundKomoot(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    DBG("~OnDescFoundKomoot() UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
        DBG("  -> eFoundCharacteristicDescriptor_komoot_0x2902\r\n");
        descriptor_komoot = params->descriptor;
        BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
        state = eFoundCharacteristicDescriptor_komoot_0x2902;
    }
}

static void OnDescFoundBike(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    DBG("~OnDescFoundBike() UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
        DBG("  -> eFoundCharacteristicDescriptor_csc_0x2902\r\n");
        descriptor_csc = params->descriptor;
        BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
        state = eFoundCharacteristicDescriptor_csc_0x2902;
    }
}

static void OnDescFoundBat(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    DBG("~OnDescFoundBat(), UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
        DBG("  -> eFoundCharacteristicDescriptor_bat_0x2902\r\n");
        descriptor_bat = params->descriptor;
        BLE::Instance().gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
        state = eFoundCharacteristicDescriptor_bat_0x2902;
    }
}

static void OnDescEndBat(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~OnDescEndBat()\r\n");
    state = eDiscoverCharacteristicDescriptorsEnd_bat;
}

static void OnDescEndCsc(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~OnDescEndCsc()\r\n");
    state = eDiscoverCharacteristicDescriptorsEnd_csc;
}

static void OnDescEndKomoot(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~OnDescEndKomoot()\r\n");
    state = eDiscoverCharacteristicDescriptorsEnd_komoot;
}

void DiscoverCharacteristicDescriptors_bat()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_bat, OnDescFoundBat, OnDescEndBat);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_bat, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_bat;
    }
}

void DiscoverCharacteristicDescriptorsBike()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_csc, OnDescFoundBike, OnDescEndCsc);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_csc, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_csc;
    }
}

void DiscoverCharacteristicDescriptorsKomoot()
{
    if (!BLE::Instance().gattClient().isServiceDiscoveryActive())
    {
        ble_error_t err = BLE::Instance().gattClient().discoverCharacteristicDescriptors(characteristic_komoot, OnDescFoundKomoot, OnDescEndKomoot);
        FLOW("~DiscoverCharacteristicDescriptors() -> eDiscoverCharacteristicDescriptors_komoot, err=0x%x\r\n", err);
        state = eDiscoverCharacteristicDescriptors_komoot;
    }
}

void OnServiceDiscoveryBikeFinished(Gap::Handle_t handle)
{
    DBG("~OnServiceDiscoveryBikeFinished()\r\n");
    state = eServiceDiscoveredBike;
}

void OnServiceDiscoveryKomootFinished(Gap::Handle_t handle)
{
    DBG("~OnServiceDiscoveryKomootFinished()\r\n");
    state = eServiceDiscoveredKomoot;
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

void OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    FLOW("~OnConnected() handle=0x%x, state=%d\r\n", params->handle, state);
    if (params->role == Gap::CENTRAL)
    {
        if (state = eConnectingBike)
        {
            connection_handle_bike = params->handle;
            state_bike = eConnected;
            state = eConnectedBike;
            FLOW("Connected Bike\r\n");
        }
        else if (state = eConnectingKomoot)
        {
            connection_handle_komoot = params->handle;
            state_komoot = eConnected;
            state = eConnectedKomoot;
            FLOW("Connected Komoot\r\n");
        }
    }
}

void StartServiceDiscoveryBike()
{
    FLOW("~StartServiceDiscoveryBike()\r\n")
    BLE::Instance().gattClient().onServiceDiscoveryTermination(OnServiceDiscoveryBikeFinished);
    BLE::Instance().gattClient().launchServiceDiscovery(connection_handle_bike, OnServiceFound, OnServiceCharacteristicFound);
    state = eServiceDiscoveryBike;
}

void StartServiceDiscoveryKomoot()
{
    FLOW("~StartServiceDiscoveryKomoot()\r\n")
    BLE::Instance().gattClient().onServiceDiscoveryTermination(OnServiceDiscoveryKomootFinished);
    BLE::Instance().gattClient().launchServiceDiscovery(connection_handle_komoot, OnServiceFound, OnServiceCharacteristicFound);
    state = eServiceDiscoveryKomoot;
}

void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
{
    INFO("~OnDisconnected() reason=0x%x\r\n", param->reason);
    state = eDisconnectedAll;
}

uint8_t street[32];

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
        if (params->handle == characteristic_csc.getValueHandle())
        {
            process_csc_data(params->data, params->len, t, speed);
        }
        else if (params->handle == characteristic_komoot.getValueHandle())
        {
            process_komoot_data(params->data, params->len, komoot_dir, komoot_dist, (uint8_t *)street, sizeof(street));
        }
    }
}

static bool ConnectBike()
{
    if ((state_bike >= eFound) && (state_bike < eConnecting))
    {
        FLOW("~ConnectBike()\r\n")
        BLE::Instance().gap().connect(device_addr_bike, device_addr_type_bike, NULL, NULL);
        state_bike = eConnecting;
        state = eConnectingBike;
        return true;
    }
    return false;
}

static bool ConnectKomoot()
{
    if ((state_komoot >= eFound) && (state_komoot < eConnecting))
    {
        FLOW("~ConnectKomoot()\r\n")
        BLE::Instance().gap().connect(device_addr_komoot, device_addr_type_komoot, NULL, NULL);
        state_komoot = eConnecting;
        state = eConnectingKomoot;
        return true;
    }
    return false;
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

    tft.initR(INITR_MINI160x80);
    tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(Adafruit_ST7735::Color565(255, 255, 0));

    BLE &ble = BLE::Instance();
    ble.init();
    ble.gap().onConnection(OnConnected);
    ble.gap().onDisconnection(OnDisconnected);

    ble.gap().setScanParams(400, 400, 0, true);
    ble.gap().startScan(AdvertisementCB);
    start_scan_ms = t.read_ms();

    ble.gattClient().onHVX(hvxCB);
    ble.gattClient().onDataRead(DataReadCB);
    ble.gattClient().onDataWrite(DataWriteCB);

    uint32_t lms = t.read_ms();
    uint8_t ic = 0;

    while (true)
    {
        switch (state)
        {
        case eDeviceDiscovery:
            break;
        case eFoundDevice:
            state = eConnectingStart;
            break;
        case eConnectingStart:
        {
            //if (state_bike != eNotFound)
            //    ConnectBike();
            //else 
            if (state_komoot != eNotFound)
                ConnectKomoot();
            else
                INFO("NOTHING TO CONNECT\r\n");
        }
        break;
        case eConnectedBike:
            if (!found_char_csc || !found_char_bat)
            {
                StartServiceDiscoveryBike();
            }
            else
            {
                //state = eRequestNotify;
            }
            break;
        case eConnectedKomoot:
            if (!found_char_komoot)
            {
                StartServiceDiscoveryKomoot();
            }
            else
            {
                //state = eRequestNotify;
            }
            break;
        case eServiceDiscoveredBike:
            DiscoverCharacteristicDescriptorsBike();
            break;
        case eServiceDiscoveredKomoot:
            DiscoverCharacteristicDescriptorsKomoot();
            break;

            /*
        case eServiceDiscovery:
            //            if (found_char_csc && found_char_bat)
            if (found_char_komoot)
            {
                BLE::Instance().gattClient().terminateServiceDiscovery();
            }
            break;*/
        case eDiscoverCharacteristicDescriptors_csc:
            break;
        case eDiscoverCharacteristicDescriptors_bat:
            break;
        case eFoundCharacteristicDescriptor_csc_0x2902:
            break;
        case eDiscoverCharacteristicDescriptorsEnd_komoot:
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
        case eDisconnectedAll:
            break;
        }

        if (new_val(komoot_dir))
        {
            const uint8_t *ptr = GetNavIcon(komoot_dir.shown);
            if (ptr)
            {
                tft.drawXBitmap2(0, 0, ptr, 80, 80, Adafruit_ST7735::Color565(255, 255, 255));
            }
        }
        if (new_val32(komoot_dist))
        {
            tft.fillRect(0, 80, 80, 50, ST77XX_BLACK);
            tft.setCursor(10, 120);
            tft.printf("%d", komoot_dist.shown);
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
#endif