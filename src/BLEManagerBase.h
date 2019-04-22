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

#include "tracer.h"

class BikeGUI;

class BLEManagerBase : private mbed::NonCopyable<BLEManagerBase>
{

    typedef BLEManagerBase Self;

  private:
  public:
    events::EventQueue event_queue_;
    BLE &ble_;
    Timer timer_;
    BikeGUI *gui_;

    template <typename ContextType>
    FunctionPointerWithContext<ContextType> as_cb(
        void (Self::*member)(ContextType context))
    {
        return makeFunctionPointer(this, member);
    }

    void Connect(BLEProtocol::AddressBytes_t &device_addr, BLEProtocol::AddressType_t &device_addr_type)
    {
        INFO("~Connect()\r\n");
        ble_.gap().connect(device_addr, device_addr_type, NULL, NULL);
    }

    virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params)
    {
        FLOW("~OnConnected() handle=0x%x\r\n", params->handle);
        if (params->role == Gap::CENTRAL)
        {
        }
    }

    virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
    {
        INFO("~OnDisconnected() reason=0x%x\r\n", param->reason);
    }

    virtual void OnDataRead(const GattReadCallbackParams *params)
    {
        FLOW("~OnDataRead(): handle %u, len=%u, ", params->handle, params->len);
        for (unsigned index = 0; index < params->len; index++)
        {
            FLOW(" %02x", params->data[index]);
        }
        FLOW("\r\n");
    }

    virtual void OnHVX(const GattHVXCallbackParams *params)
    {
        FLOW("~OnHVX(): handle %u; type %s, ", params->handle, (params->type == BLE_HVX_NOTIFICATION) ? "notification" : "indication");
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
        INFO("~OnAdvertisement addr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, advType %u, addrType %u, len=%u.\r\n",
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

    void StartScan()
    {
        INFO("~StatScan()\r\n");
        ble_.gap().setScanParams(400, 400, 0, true);
        ble_.gap().startScan(this, &BLEManagerBase::OnAdvertisement);
    }

    void StopScan()
    {
        ble_.stopScan();
    }

    void OnInitialized(BLE::InitializationCompleteCallbackContext *event)
    {
        INFO("~OnInitialized() err=0x%x\r\n", event->error);
        StartScan();
    }

    void ScheduleBleEvents(BLE::OnEventsToProcessCallbackContext *event)
    {
        event_queue_.call(mbed::callback(&event->ble, &BLE::processEvents));
    }

    void Start()
    {
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
    {
    }

    virtual ~BLEManagerBase()
    {
    }
};

#endif
