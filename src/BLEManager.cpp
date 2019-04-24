#include "BLEManager.h"

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

BLEManager::BLEManager(BLE &ble_interface, BikeGUI *gui)
    : BLEManagerBase(ble_interface, gui), csc_app(event_queue_, timer_, ble_interface), komoot_app(event_queue_, timer_, ble_interface)
{
    csc_app.SetGUI(gui);
    komoot_app.SetGUI(gui);
    csc_app.SetAppCallback(this);
    komoot_app.SetAppCallback(this);
}

BLEManager::~BLEManager() {}

void BLEManager::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    //BLEManagerBase::OnConnected(params);
    FLOW("~BLEManager::OnConnected()\r\n");
    if (csc_app.HasAddress(params->peerAddr))
    {
        csc_app.OnConnected(params);
    }
    else if (komoot_app.HasAddress(params->peerAddr))
    {
        komoot_app.OnConnected(params);
    }
}

void BLEManager::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    BLEManagerBase::OnDisconnected(params);
    FLOW("~BLEManager::OnDisconnected()\r\n");
    if (params->handle == csc_app.GetConnectionHandle())
    {
        csc_app.OnDisconnected(params);
    }
    else if (params->handle == komoot_app.GetConnectionHandle())
    {
        komoot_app.OnDisconnected(params);
    }
}

void BLEManager::OnDataRead(const GattReadCallbackParams *params)
{
    BLEManagerBase::OnDataRead(params);
}

BLEAppBase *BLEManager::GetAppWithConnectionHandle(Gap::Handle_t handle)
{
    BLEAppBase *ret = NULL;
    if (handle == csc_app.GetConnectionHandle())
    {
        ret = &csc_app;
    }
    else if (handle == komoot_app.GetConnectionHandle())
    {
        ret = &komoot_app;
    }
    return ret;
}

void BLEManager::OnHVX(const GattHVXCallbackParams *params)
{
    BLEManagerBase::OnHVX(params);
    BLEAppBase *app = GetAppWithConnectionHandle(params->connHandle);
    INFO("~BLEManager::OnHVX app=%p\r\n", app);
    if (app)
    {
        app->OnHVX(params);
    } 
    //event_queue_.call_in(100, mbed::callback(this, &BLEManager::ConnectDevices));
}
//void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params) {
//    ::OnAdvertisement(params);
//}

void BLEManager::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
    INFO("~BLEManager::OnServiceDiscoveryFinished() handle=0x%x\r\n", handle);
    BLEAppBase *app = GetAppWithConnectionHandle(handle);
    if (app)
    {
        app->OnServiceDiscoveryFinished(handle);
    }
}

void BLEManager::OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BLEManager::OnFoundService16(0x%x)\r\n", id);
    if (id == GAT_SERVICE_CSCS)
    {
        INFO("FOUND CSC\r\n");
        csc_app.SetDeviceAddress(params);
        CheckScanStop();
    }
}

void BLEManager::OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BLEManager::OnFoundService128(0x%x%x%x%x)\r\n", id[0], id[1], id[2], id[3]);
    if (IsSameId128(id, GAT_SERVICE_KOMOOT))
    {
        INFO("FOUND Komoot\r\n");
        komoot_app.SetDeviceAddress(params);
        CheckScanStop();
    }
}

void BLEManager::CheckScanStop()
{
    bool isTimeout = GetScanTimeMs() > 10000;
    INFO("~BLEManager::CheckScanStop() CSC=%d, Komoot=%d, timeout=%d\r\n",
         csc_app.HaveFoundDevice(),
         komoot_app.HaveFoundDevice(),
         isTimeout);

    if (csc_app.HaveFoundDevice() && komoot_app.HaveFoundDevice())
    {
        StopScan();
    }
    else if (isTimeout && (csc_app.HaveFoundDevice() || komoot_app.HaveFoundDevice()))
    {
        StopScan();
    }
}

void BLEManager::ConnectDevices()
{
    if (csc_app.HaveFoundDevice() && !csc_app.IsConnected())
    {
        csc_app.Connect();
    }
    else if (komoot_app.HaveFoundDevice() && !komoot_app.IsConnected())
    {
        komoot_app.Connect();
    }
}

void BLEManager::OnScanStopped()
{
    INFO("~BLEManager::OnScanStopped()\r\n");
    event_queue_.call_every(1000, mbed::callback(this, &BLEManager::ConnectDevices));
    //csc_app.Connect();
    //komoot_app.Connect();
    //event_queue_.call_in(5000, mbed::callback(&csc_app, &BLEAppCSC::Connect));
}

void BLEManager::OnAppReady(BLEAppBase *app)
{
    INFO("~BLEManager::OnAppReady() app=%p\r\n", app);
    /*
    INFO("~BLEManager::OnAppReady() app=%p\r\n", app);
    if (app == &csc_app)
    {
        INFO("==> CSC ready\r\n");
    }
    else if (app == &komoot_app)
    {
        INFO("==> komoot ready\r\n");
    }*/
}
