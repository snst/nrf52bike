#include "BikeComputer.h"
#include "UIMain.h"

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

BikeComputer::BikeComputer(BLE &ble_interface, events::EventQueue& event_queue, UIMain *ui)
    : BleManagerBase(ble_interface, event_queue, ui), csc_app(event_queue_, ble_interface, ui), komoot_app(event_queue_, ble_interface, ui)
{
    csc_app.SetAppCallback(this);
    komoot_app.SetAppCallback(this);
}

BikeComputer::~BikeComputer() {}

void BikeComputer::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    //BleManagerBase::OnConnected(params);
    FLOW("~BikeComputer::OnConnected()\r\n");
    if (csc_app.HasAddress(params->peerAddr))
    {
        /*Gap::ConnectionParams_t new_params;
        new_params.minConnectionInterval = 1000;
        new_params.maxConnectionInterval = 1200;
        new_params.slaveLatency = 0;
        new_params.connectionSupervisionTimeout = 500;
        ble_error_t ret = ble_.gap().updateConnectionParams(params->handle, &new_params);
        INFO("UPDATE: 0x%x\r\n", ret);*/
        csc_app.OnConnected(params);
    }
    else if (komoot_app.HasAddress(params->peerAddr))
    {
        komoot_app.OnConnected(params);
    }
}

void BikeComputer::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    BleManagerBase::OnDisconnected(params);
    FLOW("~BikeComputer::OnDisconnected()\r\n");
    if (params->handle == csc_app.GetConnectionHandle())
    {
        csc_app.OnDisconnected(params);
    }
    else if (params->handle == komoot_app.GetConnectionHandle())
    {
        komoot_app.OnDisconnected(params);
    }
}

void BikeComputer::OnDataRead(const GattReadCallbackParams *params)
{
    BleManagerBase::OnDataRead(params);
}

BleAppBase *BikeComputer::GetAppWithConnectionHandle(Gap::Handle_t handle)
{
    BleAppBase *ret = NULL;
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

void BikeComputer::OnHVX(const GattHVXCallbackParams *params)
{
    BleManagerBase::OnHVX(params);
    BleAppBase *app = GetAppWithConnectionHandle(params->connHandle);
    FLOW("~BikeComputer::OnHVX app=%p\r\n", app);
    if (app)
    {
        app->OnHVX(params);
    } 
}

void BikeComputer::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
    INFO("~BikeComputer::OnServiceDiscoveryFinished() handle=0x%x\r\n", handle);
    BleAppBase *app = GetAppWithConnectionHandle(handle);
    if (app)
    {
        app->OnServiceDiscoveryFinished(handle);
    }
}

void BikeComputer::OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BikeComputer::OnFoundService16(0x%x)\r\n", id);
    if (id == GAT_SERVICE_CSCS)
    {
        INFO("FOUND CSC\r\n");
        if(!csc_app.HaveFoundDevice())
        {
            UILog("CSC\n");
        }
        csc_app.SetDeviceAddress(params);
        CheckScanStop();
    }
}

void BikeComputer::OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BikeComputer::OnFoundService128(0x%x%x%x%x)\r\n", id[0], id[1], id[2], id[3]);
    if (IsSameId128(id, GAT_SERVICE_KOMOOT))
    {
        INFO("FOUND Komoot\r\n");
        if(!komoot_app.HaveFoundDevice())
        {
            UILog("Komoot\n");
        }
        komoot_app.SetDeviceAddress(params);
        CheckScanStop();
    }
}

void BikeComputer::CheckScanStop()
{
    bool isTimeout = GetScanTimeMs() > 5000;
    INFO("~BikeComputer::CheckScanStop() CSC=%d, Komoot=%d, timeout=%d\r\n",
         csc_app.HaveFoundDevice(),
         komoot_app.HaveFoundDevice(),
         isTimeout);

    if (csc_app.HaveFoundDevice() && komoot_app.HaveFoundDevice())
    {
        Gui()->SetGuiMode(UIMain::eCsc);
        //Gui()->SetGuiMode(UIMain::eHybrid);
        StopScan();
        UILog("Stop scanning\n");
    }
    else if (isTimeout && (csc_app.HaveFoundDevice() || komoot_app.HaveFoundDevice()))
    {
        Gui()->SetGuiMode(UIMain::eCsc);
        /*
        if(csc_app.HaveFoundDevice()) {
            Gui()->SetGuiMode(UIMain::eCsc);
        } else if(komoot_app.HaveFoundDevice()) {
            Gui()->SetGuiMode(UIMain::eKomoot);
        } else {
            Gui()->SetGuiMode(UIMain::eStartup);
        }*/
        UILog("Scan timeout\n");
        StopScan();
    }
}

void BikeComputer::ConnectDevices()
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

void BikeComputer::OnScanStopped()
{
    INFO("~BikeComputer::OnScanStopped()\r\n");
    //Gui()->Operational();
    event_queue_.call_every(1000, mbed::callback(this, &BikeComputer::ConnectDevices));
}

void BikeComputer::OnAppReady(BleAppBase *app)
{
    INFO("~BikeComputer::OnAppReady() app=%p\r\n", app);
    /*
    if (NULL != uilog) {
        uilog = NULL;
        //Gui()->Operational();
    }*/
}
