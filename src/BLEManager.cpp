#include "BLEManager.h"

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

BLEManager::BLEManager(BLE &ble_interface, BikeGUI* gui) 
: BLEManagerBase(ble_interface, gui), csc_app(event_queue_, timer_, ble_interface) 
{
    csc_app.SetGUI(gui);
}

BLEManager::~BLEManager() {}

void BLEManager::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    BLEManagerBase::OnConnected(params);
    if(csc_app.HasAddress(params->peerAddr)) {
        csc_app.OnConnected(params);
    }
}
void BLEManager::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    BLEManagerBase::OnDisconnected(params);
    if(params->handle == csc_app.GetConnectionHandle()) {
        csc_app.OnDisconnected(params);
    }
}
void BLEManager::OnDataRead(const GattReadCallbackParams *params)
{
    BLEManagerBase::OnDataRead(params);
}
void BLEManager::OnHVX(const GattHVXCallbackParams *params)
{
    BLEManagerBase::OnHVX(params);
    if(params->connHandle == csc_app.GetConnectionHandle()) {
        csc_app.OnHVX(params);
    }
}
//void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params) {
//    ::OnAdvertisement(params);
//}

void BLEManager::OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params)
{
    INFO("~OnFoundService16(0x%x)\r\n", id);
    if (id == GAT_SERVICE_CSCS)
    {
        StopScan();
        INFO("FOUND CSC\r\n");
        csc_app.SetDeviceAddress(params);
        csc_app.Connect();
    }
}
void BLEManager::OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params)
{
    INFO("~OnFoundService128(0x%x%x%x%x)\r\n", id[0], id[1], id[2], id[3]);
}
