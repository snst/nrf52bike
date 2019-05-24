#include "BikeComputer.h"
#include "UIMain.h"

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

BikeComputer::BikeComputer(BLE &ble_interface, events::EventQueue &event_queue, UIMain *ui)
    : BleManagerBase(ble_interface, event_queue), csc_app(event_queue_, ble_interface, ui), komoot_app(event_queue_, ble_interface, ui)
{
    ui->SetBikeComputer(this);
    RegisterApp(&csc_app);
    RegisterApp(&komoot_app);
}

BikeComputer::~BikeComputer() {}

void BikeComputer::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    INFO("~BikeComputer::OnConnected()\r\n");

    for (std::vector<BleAppBase *>::iterator it = apps_.begin(); it != apps_.end(); ++it)
    {
        INFO("check: %s\r\n", (*it)->getName());

        if ((*it)->HasAddress(params->peerAddr))
        {
            INFO("~BikeComputer::OnConnected(%s)\r\n", (*it)->getName());
            (*it)->OnConnected(params);
            break;
        }
    }
}

void BikeComputer::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    //    BleManagerBase::OnDisconnected(params);
    BleAppBase *app = GetAppWithConnectionHandle(params->handle);
    if (NULL != app)
    {
        FLOW("~BikeComputer::OnDisconnected(%s)\r\n", app->getName());
        app->OnDisconnected(params);
        ConnectApp(app);
    }
}

void BikeComputer::OnDataRead(const GattReadCallbackParams *params)
{
    BleManagerBase::OnDataRead(params);
    BleAppBase *app = GetAppWithConnectionHandle(params->connHandle);
    FLOW("~BikeComputer::OnDataRead app=%p\r\n", app);
    if (app)
    {
        app->OnDataRead(params);
    }
}

BleAppBase *BikeComputer::GetAppWithConnectionHandle(Gap::Handle_t handle)
{
    for (std::vector<BleAppBase *>::iterator it = apps_.begin(); it != apps_.end(); ++it)
    {
        if (handle == (*it)->GetConnectionHandle())
        {
            return (*it);
        }
    }
    return NULL;
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
        if (!csc_app.HaveFoundDevice())
        {
            UILog("CSC\n");
            ConnectApp(&csc_app);
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
        if (!komoot_app.HaveFoundDevice())
        {
            UILog("Komoot\n");
            ConnectApp(&komoot_app);
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
        StopScan();
        UILog("Stop scanning\n");
    }
    else if (isTimeout && (csc_app.HaveFoundDevice() || komoot_app.HaveFoundDevice()))
    {
        UILog("Scan timeout\n");
        StopScan();
    }
}

void BikeComputer::ConnectDevices()
{
    /*
    if (csc_app.HaveFoundDevice() && !csc_app.IsConnected())
    {
        csc_app.Connect();
    }
    else if (komoot_app.HaveFoundDevice() && !komoot_app.IsConnected())
    {
        komoot_app.Connect();
    }*/
}

void BikeComputer::OnScanStopped()
{
    INFO("~BikeComputer::OnScanStopped()\r\n");
    //event_queue_.call_every(5000, mbed::callback(this, &BikeComputer::ConnectDevices));
    /*
    if (csc_app.HaveFoundDevice())
        app_connect_queue_.push_back(&csc_app);

    if (komoot_app.HaveFoundDevice())
        app_connect_queue_.push_back(&komoot_app);
*/
}

void BikeComputer::OnAppReady(BleAppBase *app)
{
    INFO("~BikeComputer::OnAppReady() app=%p\r\n", app);
}

void BikeComputer::ConnectApp(BleAppBase *app)
{
    //app_connect_queue_.push_back(app);
    INFO("BikeComputer::ConnectApp(%s)\t\n", app->getName());
    event_queue_.call_in(500, mbed::callback(app, &BleAppBase::Connect));
}

void BikeComputer::RegisterApp(BleAppBase *app)
{
    apps_.push_back(app);
    app->SetAppCallback(this);
}

void BikeComputer::ConnectCsc()
{
    ConnectApp(&csc_app);
}

void BikeComputer::ConnectKomoot()
{
    ConnectApp(&komoot_app);
}
