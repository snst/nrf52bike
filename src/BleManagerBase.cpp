#include "BleManagerBase.h"
#include "UIMain.h"

BleManagerBase::BleManagerBase(BLE &ble_interface, UIMain *gui) : ble_(ble_interface), gui_(gui), scanning_active_(false)
{
}

BleManagerBase::~BleManagerBase()
{
}

void BleManagerBase::OnAppReady(BleAppBase *app)
{
    INFO("~BleManagerBase::OnAppReady() app=%p\r\n", app);
}
/*
void Connect(BLEProtocol::AddressBytes_t &device_addr, BLEProtocol::AddressType_t &device_addr_type)
{
    INFO("~BleManagerBase::Connect()\r\n");
    ble_.gap().connect(device_addr, device_addr_type, NULL, NULL);
}
*/
void BleManagerBase::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    FLOW("~BleManagerBase::OnConnected() handle=0x%x\r\n", params->handle);
}

void BleManagerBase::OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
{
    INFO("~BleManagerBase::OnDisconnected() reason=0x%x\r\n", param->reason);
}

void BleManagerBase::OnDataRead(const GattReadCallbackParams *params)
{
    FLOW("~BleManagerBase::OnDataRead(): handle %u, len=%u, ", params->handle, params->len);
    for (unsigned index = 0; index < params->len; index++)
    {
        FLOW(" %02x", params->data[index]);
    }
    FLOW("\r\n");
}

void BleManagerBase::OnHVX(const GattHVXCallbackParams *params)
{
    FLOW("~BleManagerBase::OnHVX(): handle %u; type %s, ", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
    for (unsigned index = 0; index < params->len; index++)
    {
        FLOW(" %02x", params->data[index]);
    }
    FLOW("\r\n");
}

void BleManagerBase::OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params)
{
}

void BleManagerBase::OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params)
{
}

void BleManagerBase::OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params)
{
    DBG("~BleManagerBase::OnAdvertisement addr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, advType %u, addrType %u, len=%u.\r\n",
        params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
        params->rssi, params->isScanResponse, params->type, params->peerAddrType, params->advertisingDataLen);

    mbed::Span<const uint8_t> ad(params->advertisingData, params->advertisingDataLen);

    ble::AdvertisingDataParser adv_parser(ad);

    while (adv_parser.hasNext())
    {
        ble::AdvertisingDataParser::element_t field = adv_parser.next();
        if ((field.type == ble::adv_data_type_t::INCOMPLETE_LIST_16BIT_SERVICE_IDS) || (field.type == ble::adv_data_type_t::COMPLETE_LIST_16BIT_SERVICE_IDS))
        {
            const uint8_t *p = field.value.data();
            uint16_t val;
            for (size_t i = 0; i < field.value.size() / sizeof(uint16_t); i++)
            {
                val = *p++;
                val |= (*p++ << 8);
                OnFoundService16(val, params);
            }
        }

        if ((field.type == ble::adv_data_type_t::INCOMPLETE_LIST_128BIT_SERVICE_IDS) || (field.type == ble::adv_data_type_t::COMPLETE_LIST_128BIT_SERVICE_IDS))
        {
            if (field.value.size() >= 16)
            {
                OnFoundService128(field.value.data(), params);
            }
        }
    }
}

bool BleManagerBase::IsScanActive()
{
    return scanning_active_;
}

void BleManagerBase::StartScan(uint32_t timeout)
{
    INFO("~BleManagerBase::StartScan(), timeout=%u, active=%d\r\n", timeout, scanning_active_);
    if (!scanning_active_)
    {
        start_scan_ms_ = timer_.read_ms();
        scanning_active_ = true;
        ble_.gap().setScanParams(400, 400, 0, true);
        ble_.gap().startScan(this, &BleManagerBase::OnAdvertisement);
        /*    if (timeout > 0)
        {
            event_queue_.call_in(timeout, mbed::callback(this, &BleManagerBase::OnScanTimeout));
        }*/
    }
}

uint32_t BleManagerBase::GetScanTimeMs()
{
    return timer_.read_ms() - start_scan_ms_;
}

void BleManagerBase::StopScan()
{
    INFO("~BleManagerBase::StopScan() active=%d\r\n", scanning_active_);
    if (scanning_active_)
    {
        scanning_active_ = false;
        ble_.stopScan();
        event_queue_.call_in(100, mbed::callback(this, &BleManagerBase::OnScanStopped));
    }
}

void BleManagerBase::OnScanStopped()
{
    INFO("~BleManagerBase::OnScanStopped()\r\n");
}

void BleManagerBase::OnInitDone()
{
    INFO("~BleManagerBase::OnInitDone()\r\n");
    Gui()->Log("Scanning\n");
    StartScan();
}

void BleManagerBase::OnInitialized(BLE::InitializationCompleteCallbackContext *event)
{
    INFO("~BleManagerBase::OnInitialized() err=0x%x\r\n", event->error);
    OnInitDone();
}

void BleManagerBase::ScheduleBleEvents(BLE::OnEventsToProcessCallbackContext *event)
{
    event_queue_.call(mbed::callback(&event->ble, &BLE::processEvents));
}

void BleManagerBase::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
    INFO("~BleManagerBase::OnServiceDiscoveryFinished() handle=0x%x\r\n", handle);
}

void BleManagerBase::Start()
{
    INFO("~BleManagerBase::Start()\r\n");
    Gui()->Log("Startup\n");
    ble_.onEventsToProcess(makeFunctionPointer(this, &Self::ScheduleBleEvents));
    timer_.start();

    ble_error_t err = ble_.init(this, &BleManagerBase::OnInitialized);

    ble_.gap().onConnection(as_cb(&Self::OnConnected));
    ble_.gap().onDisconnection(as_cb(&Self::OnDisconnected));
    ble_.gattClient().onHVX(as_cb(&Self::OnHVX));
    ble_.gattClient().onDataRead(as_cb(&Self::OnDataRead));
    ble_.gattClient().onServiceDiscoveryTermination(as_cb(&Self::OnServiceDiscoveryFinished));

    event_queue_.dispatch_forever();
}

bool BleManagerBase::IsSameId128(const uint8_t *a, const uint8_t *b)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        if (a[i] != b[15 - i])
        {
            return false;
        }
    }
    return true;
}

UIMain *BleManagerBase::Gui()
{
    return gui_;
}