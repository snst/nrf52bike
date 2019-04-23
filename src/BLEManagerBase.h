#ifndef BLEMANAGERBASE_H_
#define BLEMANAGERBASE_H_

#include "mbed.h"
#include <stdint.h>
#include <stdio.h>

#include "events/EventQueue.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GapAdvertisingParams.h"
#include "ble/GapAdvertisingData.h"
#include "ble/FunctionPointerWithContext.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "gap/AdvertisingDataParser.h"
#include "IAppCallback.h"

#include "tracer.h"

class BikeGUI;

class BLEManagerBase : private mbed::NonCopyable<BLEManagerBase>, public IAppCallback
{

    typedef BLEManagerBase Self;

  private:
  public:
    events::EventQueue event_queue_;
    BLE &ble_;
    Timer timer_;
    BikeGUI *gui_;
    bool scanning_active_;
    uint32_t start_scan_ms_;


    template <typename ContextType>
    FunctionPointerWithContext<ContextType> as_cb(
        void (Self::*member)(ContextType context))
    {
        return makeFunctionPointer(this, member);
    }

    virtual void OnAppReady(BLEAppBase *app)
    {
        INFO("~BLEManagerBase::OnAppReady() app=%p\r\n", app);
    }
    /*
    void Connect(BLEProtocol::AddressBytes_t &device_addr, BLEProtocol::AddressType_t &device_addr_type)
    {
        INFO("~BLEManagerBase::Connect()\r\n");
        ble_.gap().connect(device_addr, device_addr_type, NULL, NULL);
    }
*/
    virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params)
    {
        FLOW("~BLEManagerBase::OnConnected() handle=0x%x\r\n", params->handle);
    }

    virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
    {
        INFO("~BLEManagerBase::OnDisconnected() reason=0x%x\r\n", param->reason);
    }

    virtual void OnDataRead(const GattReadCallbackParams *params)
    {
        FLOW("~BLEManagerBase::OnDataRead(): handle %u, len=%u, ", params->handle, params->len);
        for (unsigned index = 0; index < params->len; index++)
        {
            FLOW(" %02x", params->data[index]);
        }
        FLOW("\r\n");
    }

    virtual void OnHVX(const GattHVXCallbackParams *params)
    {
        FLOW("~BLEManagerBase::OnHVX(): handle %u; type %s, ", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
        for (unsigned index = 0; index < params->len; index++)
        {
            FLOW(" %02x", params->data[index]);
        }
        FLOW("\r\n");
    }

    virtual void OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params) {}
    virtual void OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params) {}

    void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params)
    {
        DBG("~BLEManagerBase::OnAdvertisement addr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, advType %u, addrType %u, len=%u.\r\n",
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


    bool IsScanActive()
    {
        return scanning_active_;
    }

    void StartScan(uint32_t timeout = 0)
    {
        INFO("~BLEManagerBase::StartScan(), timeout=%u, active=%d\r\n", timeout, scanning_active_);
        if (!scanning_active_)
        {
            start_scan_ms_ = timer_.read_ms();
            scanning_active_ = true;
            ble_.gap().setScanParams(400, 400, 0, true);
            ble_.gap().startScan(this, &BLEManagerBase::OnAdvertisement);
        /*    if (timeout > 0)
            {
                event_queue_.call_in(timeout, mbed::callback(this, &BLEManagerBase::OnScanTimeout));
            }*/
        }
    }

    uint32_t GetScanTimeMs()
    {
        return timer_.read_ms() - start_scan_ms_;
    }

    void StopScan()
    {
        INFO("~BLEManagerBase::StopScan() active=%d\r\n", scanning_active_);
        if (scanning_active_)
        {
            scanning_active_ = false;
            ble_.stopScan();
            event_queue_.call_in(100, mbed::callback(this, &BLEManagerBase::OnScanStopped));
        }
    }

    virtual void OnScanStopped()
    {
        INFO("~BLEManagerBase::OnScanStopped()\r\n");
    }

    virtual void OnInitDone()
    {
        INFO("~BLEManagerBase::OnInitDone()\r\n");
        StartScan();
    }

    void OnInitialized(BLE::InitializationCompleteCallbackContext *event)
    {
        INFO("~BLEManagerBase::OnInitialized() err=0x%x\r\n", event->error);
        OnInitDone();
    }

    void ScheduleBleEvents(BLE::OnEventsToProcessCallbackContext *event)
    {
        event_queue_.call(mbed::callback(&event->ble, &BLE::processEvents));
    }

    void Start()
    {
        INFO("~BLEManagerBase::Start()\r\n");
        ble_.onEventsToProcess(makeFunctionPointer(this, &Self::ScheduleBleEvents));
        timer_.start();

        ble_error_t err = ble_.init(this, &BLEManagerBase::OnInitialized);

        ble_.gap().onConnection(as_cb(&Self::OnConnected));
        ble_.gap().onDisconnection(as_cb(&Self::OnDisconnected));
        ble_.gattClient().onHVX(as_cb(&Self::OnHVX));
        ble_.gattClient().onDataRead(as_cb(&Self::OnDataRead));

        event_queue_.dispatch_forever();
    }

    BLEManagerBase(BLE &ble_interface, BikeGUI *gui) : ble_(ble_interface), gui_(gui)
        , scanning_active_(false)
    {
    }

    virtual ~BLEManagerBase()
    {
    }

    bool IsSameId128(const uint8_t *a, const uint8_t *b)
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
};

#endif
