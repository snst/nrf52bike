#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"

#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"

#define ENABLE_DEBUG_PRINT
#ifdef ENABLE_DEBUG_PRINT
#define DBG(...)             \
    {                        \
        printf(__VA_ARGS__); \
    }
#else
#define DBG(...)
#endif

Serial pc(USBTX, USBRX); // tx, rx

Gap::Handle_t connection_handle = 0xFFFF;

static bool found_characteristic = false;
static DiscoveredCharacteristic found_characteristic_obj;

static bool found_descriptor = false;
static DiscoveredCharacteristicDescriptor found_descriptor_obj(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));

//static DiscoveredCharacteristicDescriptor desc_of_Characteristic_values(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));

static bool found_device = false;
static bool do_connect = false;
static bool is_connected = false;
static bool send_tick = false;
bool rr = true;

static BLEProtocol::AddressBytes_t device_addr;

struct cscflags
{
    uint8_t _wheel_revolutions_present : 1;
    uint8_t _crank_revolutions_present : 1;
    uint8_t _reserved : 6;
};

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

void process_data(const uint8_t *data, uint32_t len)
{
    //static const uint16_t MAX_BYTES = (1 + 4 + 2 + 2 + 2);

    //00 04 00 20 e9 08 00 00 7d 05 00 00 c9 08

    //03 be 09 00 00 c9 4d 9e 00 b5 50

    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027

    if (len == 11)
    {
        uint8_t flags = data[0];
        uint32_t wheelCounter = 0;
        uint16_t lastWheelEvent = 0;
        uint16_t crankCounter = 0;
        uint16_t lastCrankEvent = 0;
        uint8_t p = 1;

        if (flags & FLAG_WHEEL_PRESENT)
        {
            wheelCounter = data[p++];
            wheelCounter |= data[p++] << 8;
            wheelCounter |= data[p++] << 16;
            wheelCounter |= data[p++] << 24;
            lastWheelEvent = data[p++];
            lastWheelEvent |= data[p++] << 8;
        }
        if (flags & FLAG_CRANK_PRESENT)
        {
            crankCounter = data[p++];
            crankCounter |= data[p++] << 8;
            lastCrankEvent = data[p++];
            lastCrankEvent |= data[p++] << 8;
        }
        printf("**** wc=%u, we=%u, cc=%u, ce=%u\r\n", wheelCounter, lastWheelEvent, crankCounter, lastCrankEvent);
    }
}

void PeriodicCB(void)
{
    //DBG("#\r\n");
    send_tick = true;
    /*
    */
}

void AdvertisementCB(const Gap::AdvertisementCallbackParams_t *params)
{
    if (params->peerAddr[5] == 0xf4)
    {
        DBG("~AdvertisementCB peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u.",
            params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
            params->rssi, params->isScanResponse, params->type, params->peerAddrType);

        DBG(" -> found 0xf4 -> stopScan -> connect");
        BLE::Instance().stopScan();

        /*
        Gap::ConnectionParams_t connection_parameters = {
            50,  // min connection interval
            100, // max connection interval
            0,   // slave latency
            600  // connection supervision timeout
        };
        // scan parameter used to find the device to connect to
        GapScanningParams scanning_params(
            100,  // interval
            100,  // window
            3000,    // timeout
            false // active
        );

        BLE::Instance().gap().connect(params->peerAddr, BLEProtocol::AddressType::PUBLIC, &connection_parameters,
                                      &scanning_params);
                                      */

        memcpy(&device_addr, &params->peerAddr, sizeof(device_addr));
        found_device = true;
        do_connect = true;

        //        BLE::Instance().gap().connect(params->peerAddr, BLEProtocol::AddressType::PUBLIC, NULL, NULL);
        DBG("\r\n");
    }
}

void ServiceDiscoveryCB(const DiscoveredService *service)
{
#if 0
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT)
    {
        DBG("~ServiceDiscoveryCB() S UUID-%x", service->getUUID().getShortUUID());
    }
    else
    {
        DBG("~ServiceDiscoveryCB() L UUID-");
        const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
        for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++)
        {
            DBG("%02x", longUUIDBytes[i]);
        }
    }
    DBG(", attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
#endif
}

void ReadCB(const GattReadCallbackParams *params)
{
    DBG("~ReadCB() len=%u\r\n", params->len);
}

void ServiceCharacteristicsCB(const DiscoveredCharacteristic *param)
{
    if (param->getUUID().getShortUUID() == 0x2A5B)
    {
        DBG("~ServiceCharacteristicsCB() UUID-%x valueAttr[%u] props[%x]", param->getUUID().getShortUUID(), param->getValueHandle(), (uint8_t)param->getProperties().broadcast());
        DBG(" -> found 0x2A5B -> save characteristic");
        found_characteristic = true;
        found_characteristic_obj = *param;
        DBG("\r\n");
    }
}

static void DiscoveredDescCB(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
        DBG("~DiscoveredDescCB(), UUID %x, ", params->descriptor.getUUID().getShortUUID());
        DBG(" found 0x2902, save descriptor");
        found_descriptor = true;
        found_descriptor_obj = params->descriptor;
        DBG("\r\n");
    }
}

static void DiscoveredDescTerminationCB(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    DBG("~DiscoveredDescTerminationCB()");
    if (found_descriptor)
    {
        DBG(", descriptor already found, write notify, h1=0x%x, h2=0x%x",
            found_characteristic_obj.getConnectionHandle(),
            found_descriptor_obj.getConnectionHandle());

        uint16_t value = BLE_HVX_NOTIFICATION;
        ble_error_t err;

        /* THIS
        err = BLE::Instance().gattClient().write(
            GattClient::GATT_OP_WRITE_REQ,
            found_descriptor_obj.getConnectionHandle(),
            found_descriptor_obj.getAttributeHandle(),
            sizeof(uint16_t),
            (uint8_t *)&value);
*/

        /*
        err = BLE::Instance().gattClient().write(
            GattClient::GATT_OP_WRITE_REQ,
            connection_handle,
            found_characteristic_obj.getValueHandle() + 1,
            sizeof(uint16_t),
            (uint8_t *)&value);
*/
        printf("write err=0x%x\r\n", err);

        is_connected = true;

        //        DBG("write: 0x%x\r\n", err);
        //        err = BLE::Instance().gattClient().read(
        //            found_descriptor_obj.getConnectionHandle(),
        //            found_descriptor_obj.getAttributeHandle(), 0);

        //ble_error_t err = found_characteristic_obj.write(sizeof(uint16_t),(uint8_t *)&value);
        //        DBG(", err=%u", err);
    }
    DBG("\r\n");
}

void ServiceDiscoveryTerminationCB(Gap::Handle_t handle)
{
    DBG("~ServiceDiscoveryTerminationCB() for handle %u", handle);
    if (found_characteristic)
    {
        DBG(", characteristic already found -> discoverCharacteristicDescriptors");
        BLE::Instance().gattClient().discoverCharacteristicDescriptors(found_characteristic_obj, DiscoveredDescCB, DiscoveredDescTerminationCB);
    }
    DBG("\r\n");
}

void DataReadCB(const GattReadCallbackParams *params)
{
    DBG("~DataReadCB: handle %u, len=%u, ", params->handle, params->len);
    for (unsigned index = 0; index < params->len; index++)
    {
        DBG(" %02x", params->data[index]);
    }
    /*
    uint16_t value = BLE_HVX_NOTIFICATION;
    ble_error_t err;

    err = BLE::Instance().gattClient().write(
        GattClient::GATT_OP_WRITE_REQ,
        found_descriptor_obj.getConnectionHandle(),
        found_descriptor_obj.getAttributeHandle(),
        sizeof(uint16_t),
        (uint8_t *)&value);
    DBG("write: 0x%x\r\n", err);
        */
    DBG("\r\n");
}

void DataWriteCB(const GattWriteCallbackParams *params)
{
    DBG("~DataWriteCB: handle %u, len=%u, statux=0x%x, err=0x%x", params->handle, params->len, params->status, params->error_code);
    for (unsigned index = 0; index < params->len; index++)
    {
        DBG(" %02x", params->data[index]);
    }

    DBG("\r\n");
}

void ConnectionCB(const Gap::ConnectionCallbackParams_t *params)
{
    DBG("~ConnectionCB()");
    if (params->role == Gap::CENTRAL)
    {
        connection_handle = params->handle;
        DBG(", Role:Central, connectionHandle=0x%x -> launchServiceDiscovery", connection_handle);
        BLE::Instance().gattClient().onServiceDiscoveryTermination(ServiceDiscoveryTerminationCB);
        BLE::Instance().gattClient().launchServiceDiscovery(params->handle, ServiceDiscoveryCB, ServiceCharacteristicsCB /*, 0xa000, 0xa001*/);
    }
    DBG("\r\n");
}

void DisconnectionCB(const Gap::DisconnectionCallbackParams_t *param)
{
    //BLE::Instance().gap().startAdvertising();
    DBG("~DisconnectionCB(), reason=0x%x\r\n", param->reason);
    do_connect = true;
    is_connected = false;
    rr = true;
}

void hvxCB(const GattHVXCallbackParams *params)
{
    DBG("~hvxCB: handle %u; type %s, ", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
    for (unsigned index = 0; index < params->len; index++)
    {
        DBG(" %02x", params->data[index]);
    }

    if (params->type == BLE_HVX_NOTIFICATION)
    {
        process_data(params->data, params->len);
    }
    DBG("\r\n");
}

int main(void)
{
    DBG("+main()\r\n");
    Ticker ticker;
    ticker.attach(PeriodicCB, 1);

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

    // now, actually set your preferences
    ble.gap().setPreferredConnectionParams(&connParams);

    ble.gap().setScanParams(500, 400, 0, false);
    ble.gap().startScan(AdvertisementCB);

    ble.gattClient().onHVX(hvxCB);
    ble.gattClient().onDataRead(DataReadCB);
    ble.gattClient().onDataWrite(DataWriteCB);

    while (true)
    {
        if (found_device && do_connect)
        {
            do_connect = false;
            BLE::Instance().gap().connect(device_addr, BLEProtocol::AddressType::PUBLIC, NULL, NULL);
        }

        if (send_tick && is_connected)
        {
            send_tick = false;
            ble_error_t err;

            if (rr)
            {
                uint16_t value = BLE_HVX_NOTIFICATION;
                err = BLE::Instance().gattClient().write(
                    GattClient::GATT_OP_WRITE_REQ,
                    found_descriptor_obj.getConnectionHandle(),
                    found_descriptor_obj.getAttributeHandle(),
                    sizeof(uint16_t),
                    (uint8_t *)&value);
                rr = false;
                printf("WRITE NOTY 0x%x", err);
            }
            else
            {

                err = BLE::Instance().gattClient().read(
                    found_descriptor_obj.getConnectionHandle(),
                    found_descriptor_obj.getAttributeHandle(),
                    0);
            }
            //DBG("read: 0x%x\r\n", err);
        }

        if (found_characteristic && !ble.gattClient().isServiceDiscoveryActive())
        {
            //    found_characteristic = false;
            //    uint16_t value = BLE_HVX_NOTIFICATION;
            /*            ble_error_t err = BLE::Instance().gattClient().write(
                GattClient::GATT_OP_WRITE_REQ,
                found_descriptor_obj.getConnectionHandle(),
                found_descriptor_obj.getAttributeHandle(),
                sizeof(uint16_t),
                (uint8_t *)&value);
*/
            /*
            ble_error_t err = BLE::Instance().gattClient().write(
                GattClient::GATT_OP_WRITE_REQ,
                connection_handle,
                found_characteristic_obj.getValueHandle() + 1,
                sizeof(uint16_t),
                (uint8_t *)&value);


                DBG("write BLE_HVX_NOTIFICATION, err=0x%x\r\n", err);*/
        }

        ble.waitForEvent();
    }
}
