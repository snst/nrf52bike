#include "BikeComputer.h"
#include "UIMain.h"

#define GAT_SERVICE_CSCS (0x1816u)
#define GAT_SERVICE_BAT (0x180Fu)
const uint8_t GAT_SERVICE_KOMOOT[] = {0x71, 0xC1, 0xE1, 0x28, 0xD9, 0x2F, 0x4F, 0xA8, 0xA2, 0xB2, 0x0F, 0x17, 0x1D, 0xB3, 0x43, 0x6C};

BikeComputer::BikeComputer(BLE &ble_interface, events::EventQueue &event_queue, UIMain *ui)
    : BleManagerBase(ble_interface, event_queue), csc_app(event_queue_, ble_interface, ui), komoot_app(event_queue_, ble_interface, ui), ui_(ui)
{
    ui->SetBikeComputer(this);
    RegisterApp(&csc_app);
    RegisterApp(&komoot_app);
}

BikeComputer::~BikeComputer() {}

void BikeComputer::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    FLOW("~BikeComputer::OnConnected()\r\n");
    for (std::vector<BleAppBase *>::iterator it = apps_.begin(); it != apps_.end(); ++it)
    {
        if ((*it)->HasAddress(params->peerAddr))
        {
            INFO("~BikeComputer::OnConnected(%s)\r\n", (*it)->GetName());
            (*it)->OnConnected(params);
            break;
        }
    }
}

void BikeComputer::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    BleAppBase *app = GetAppWithConnHandle(params->handle);
    if (NULL != app)
    {
        FLOW("~BikeComputer::OnDisconnected(%s)\r\n", app->GetName());
        app->OnDisconnected(params);
        Connect(app->getId(), 1000);
    }
}

void BikeComputer::OnDataRead(const GattReadCallbackParams *params)
{
    BleManagerBase::OnDataRead(params);
    BleAppBase *app = GetAppWithConnHandle(params->connHandle);
    if (NULL != app)
    {
        FLOW("~BikeComputer::OnDataRead(%s)\r\n", app->GetName());
        app->OnDataRead(params);
    }
}

BleAppBase *BikeComputer::GetAppWithConnHandle(Gap::Handle_t handle)
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
    BleAppBase *app = GetAppWithConnHandle(params->connHandle);
    if (NULL != app)
    {
        FLOW("~BikeComputer::OnHVX(%s)\r\n", app->GetName());
        app->OnHVX(params);
    }
}

void BikeComputer::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
    BleAppBase *app = GetAppWithConnHandle(handle);
    if (app)
    {
        INFO("~BikeComputer::OnServiceDiscoveryFinished(%s) handle=0x%x\r\n", app->GetName(), handle);
        app->OnServiceDiscoveryFinished(handle);
    }
}

void BikeComputer::OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BikeComputer::OnFoundService16(0x%x)\r\n", id);
    if (id == GAT_SERVICE_CSCS)
    {
        if (!csc_app.FoundDevice())
        {
            INFO("FOUND CSC\r\n");
            UILog("CSC\n");
            csc_app.SetDeviceAddress(params);
            //Connect(BC::eCsc);
        }
        CheckScanStop();
    }
}

void BikeComputer::OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params)
{
    FLOW("~BikeComputer::OnFoundService128(0x%x%x%x%x)\r\n", id[0], id[1], id[2], id[3]);
    if (IsSameId128(id, GAT_SERVICE_KOMOOT))
    {
        if (!komoot_app.FoundDevice())
        {
            INFO("FOUND Komoot\r\n");
            UILog("Komoot\n");
            komoot_app.SetDeviceAddress(params);
            //Connect(BC::eKomoot);
        }
        CheckScanStop();
    }
}

void BikeComputer::CheckScanStop()
{
    bool isTimeout = GetScanTimeMs() > 5000;
    FLOW("~BikeComputer::CheckScanStop() CSC=%d, Komoot=%d, timeout=%d\r\n",
         csc_app.FoundDevice(),
         komoot_app.FoundDevice(),
         isTimeout);

    if (csc_app.FoundDevice() && komoot_app.FoundDevice())
    {
        UILog("Found all\n");
        StopScan();
    }
    else if (isTimeout && (csc_app.FoundDevice() || komoot_app.FoundDevice()))
    {
        UILog("Scan timeout\n");
        StopScan();
    }
}

void BikeComputer::StopScan()
{
    if (IsScanActive()) 
    {
        INFO("Stop scanning\r\n");
        UILog("Stop scanning\n");
        BleManagerBase::StopScan();
        Connect(BC::eCsc, 500);
        Connect(BC::eKomoot, 1500);
    }
}

void BikeComputer::RegisterApp(BleAppBase *app)
{
    apps_.push_back(app);
}

BleAppBase *BikeComputer::GetAppWithId(BC::eApp_t app_id)
{
    BleAppBase *app;
    switch (app_id)
    {
    case BC::eCsc:
        app = &csc_app;
        break;
    case BC::eKomoot:
        app = &komoot_app;
        break;
    default:
        app = NULL;
    }
    return app;
}

void BikeComputer::Connect(BC::eApp_t app_id, uint32_t delay)
{
    BleAppBase *app = GetAppWithId(app_id);
    if (NULL != app)
    {
        INFO("BikeComputer::Connect(%s)\r\n", app->GetName());
        event_queue_.call_in(delay, mbed::callback(app, &BleAppBase::Connect));
    }
}

void BikeComputer::SetUiMode(IUIMode::eUiMode_t mode)
{
    ui_->SetUiMode(mode);
}

void BikeComputer::SetBacklightBrightness(uint8_t val)
{
    ui_->SetBacklightBrightness(val);
}

uint32_t BikeComputer::GetCscDisconnects()
{
    return csc_app.GetDisconnects();
}

bool BikeComputer::IsAppAvailable(BC::eApp_t app_id)
{
    BleAppBase* app = GetAppWithId(app_id);
    return (NULL != app) && (app->FoundDevice());
}

uint8_t BikeComputer::GetCscBat()
{
    return csc_app.GetBatteryPercent();
}

IUICsc::CscData_t* BikeComputer::GetCscData()
{
    return &ui_->last_csc_;
}

events::EventQueue* BikeComputer::GetEventQueue()
{
    return &event_queue_;
}