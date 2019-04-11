#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"

#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"

Serial pc(USBTX, USBRX); // tx, rx


Gap::Handle_t connectionHandle = 0xFFFF;


static uint8_t characteristic_is_fond = 0;
static DiscoveredCharacteristic Characteristic_values;

static uint8_t descriptor_is_found = 0;
static DiscoveredCharacteristicDescriptor desc_of_Characteristic_values(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));

void periodicCallback(void)
{
    pc.printf("#");
}

void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params)
{
    if (params->peerAddr[5] != 0xf4)
    { 
        return;
    }
    BLE::Instance().stopScan();
    printf("adv peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u\r\n",
           params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
           params->rssi, params->isScanResponse, params->type, params->peerAddrType);

    printf("+connecting..\r\n");
    BLE::Instance().gap().connect(params->peerAddr, BLEProtocol::AddressType::PUBLIC /*params->peerAddrType*/, NULL, NULL);
    printf("-connecting..\r\n");
}

void serviceDiscoveryCallback(const DiscoveredService *service)
{
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT)
    {
        printf("S UUID-%x attrs[%u %u]\r\n", service->getUUID().getShortUUID(), service->getStartHandle(), service->getEndHandle());
    }
    else
    {
        printf("S UUID-");
        const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
        for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++)
        {
            printf("%02x", longUUIDBytes[i]);
        }
        printf(" attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
    }
}

void readCallback(const GattReadCallbackParams *params)
{
    printf("read: len=%u\r\n", params->len);
}

void characteristicDiscoveryCallback(const DiscoveredCharacteristic *characteristicP)
{

    printf("  C UUID-%x valueAttr[%u] props[%x]\r\n", characteristicP->getUUID().getShortUUID(), characteristicP->getValueHandle(), (uint8_t)characteristicP->getProperties().broadcast());

    //  printf("  C UUID-%x valueAttr[%u] props[%x]\r\n", characteristicP->getShortUUID(), characteristicP->getValueHandle(), (uint8_t)characteristicP->getProperties().broadcast());
    if (characteristicP->getUUID().getShortUUID() == 0x2A5B)
    { /* !ALERT! Update this filter to suit your device. */
        printf("found ch\r\n");
        //buttonCharacteristic      = *characteristicP;
        //foundButtonCharacteristic = true;
        //ble_error_t e = characteristicP->read(0, readCallback);
        //printf("found ch, ret=%u\r\n", e);

        characteristic_is_fond = 1;
        Characteristic_values = *characteristicP;

        /*
            uint16_t value = BLE_HVX_NOTIFICATION;
        BLE::Instance().gattClient().write(GattClient::GATT_OP_WRITE_REQ,
                                           connectionHandle,
                                           Characteristic_values.getValueHandle() + 1, // HACK Alert. We're assuming that CCCD descriptor immediately follows the value attribute. 
                                           sizeof(uint16_t),                          // HACK Alert! size should be made into a BLE_API constant. 
                                           reinterpret_cast<const uint8_t *>(&value));
                                           */
    }
}

static void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
    printf("+discoveredCharsDescriptorCallBack, UUID %x\r\n", params->descriptor.getUUID().getShortUUID());
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
        // Save characteristic info
        printf("..descriptor_is_found=1\r\n");
        descriptor_is_found = 1;
        desc_of_Characteristic_values = params->descriptor;
    }
}

static void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    printf("+discoveredDescTerminationCallBack\r\n");
    if (descriptor_is_found)
    {
        printf("write value 1\r\n");
        uint16_t value = BLE_HVX_NOTIFICATION;
        //uint16_t value = BLE_HVX_INDICATION;
        ble_error_t err = BLE::Instance().gattClient().write(
            GattClient::GATT_OP_WRITE_REQ,
            Characteristic_values.getConnectionHandle(),
            desc_of_Characteristic_values.getAttributeHandle(),
            sizeof(uint16_t),
            (uint8_t *)&value);
            printf("write err=%u\r\n", err);
    }
}

void discoveryTerminationCallback(Gap::Handle_t connectionHandle)
{
    printf("terminated SD for handle %u\r\n", connectionHandle);
    if (characteristic_is_fond == 1)
    {
        printf("..discoverCharacteristicDescriptors\r\n");
        BLE::Instance().gattClient().discoverCharacteristicDescriptors(Characteristic_values, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
    }
}

void onDataReadCallBack(const GattReadCallbackParams *params)
{

    printf("onDataReadCallBack: handle %u\r\n", params->handle);
    for (unsigned index = 0; index < params->len; index++)
    {
        printf(" %02x", params->data[index]);
    }
    printf("\r\n");
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
    printf("+connectionCallback\r\n");
    if (params->role == Gap::CENTRAL)
    {
        printf("..launchServiceDiscovery\r\n");
        connectionHandle = params->handle;
        BLE::Instance().gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
        BLE::Instance().gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback, characteristicDiscoveryCallback /*, 0xa000, 0xa001*/);
    }
    printf("-connectionCallback\r\n");
}
/*
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason) {
    printf("disconnected\r\n");
}
*/
void disconnectionCallback(const Gap::DisconnectionCallbackParams_t * param)
{
    //BLE::Instance().gap().startAdvertising();
    printf("disconnected 0x%x\r\n", param->reason);
}

void hvxCallback(const GattHVXCallbackParams *params)
{
    printf("hvxCallback: handle %u; type %s\r\n", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
    for (unsigned index = 0; index < params->len; index++)
    {
        printf(" %02x", params->data[index]);
    }
    printf("\r\n");
}

//GattClient::ReadCallback_t

int main(void)
{
    printf("+main()\r\n");
    //    Ticker ticker;
    //    ticker.attach(periodicCallback, 1);
    BLE &ble = BLE::Instance();

    ble.init();
    ble.gap().onConnection(connectionCallback);
    ble.gap().onDisconnection(disconnectionCallback);

    ble.gap().setScanParams(500, 400);
    ble.gap().startScan(advertisementCallback);

    ble.gattClient().onHVX(hvxCallback);
    ble.gattClient().onDataRead(onDataReadCallBack);

    while (true)
    {
#if 0
        if (foundButtonCharacteristic && !ble.gattClient().isServiceDiscoveryActive()) {
            //foundButtonCharacteristic = false; /* need to do the following only once */
 
            /* Note: Yuckiness alert! The following needs to be encapsulated in a neat API.
             * It isn't clear whether we should provide a DiscoveredCharacteristic::enableNoticiation() or
             * DiscoveredCharacteristic::discoverDescriptors() followed by DiscoveredDescriptor::write(...). */
            
            uint16_t value = BLE_HVX_NOTIFICATION;
            ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ,
                                   connectionHandle,
                                   buttonCharacteristic.getValueHandle() + 1, /* HACK Alert. We're assuming that CCCD descriptor immediately follows the value attribute. */
                                   sizeof(uint16_t),                          /* HACK Alert! size should be made into a BLE_API constant. */
                                   reinterpret_cast<const uint8_t *>(&value));
            
            printf("+read\r\n");
            buttonCharacteristic.read(0, readCallback);
            printf("-read\r\n");
            
        }
#endif
        ble.waitForEvent();
    }
}
