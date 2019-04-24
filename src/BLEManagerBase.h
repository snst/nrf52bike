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

  public:

    template <typename ContextType>
    FunctionPointerWithContext<ContextType> as_cb(
        void (Self::*member)(ContextType context))
    {
        return makeFunctionPointer(this, member);
    }

    BLEManagerBase(BLE &ble_interface, BikeGUI *gui);
    virtual ~BLEManagerBase();
    virtual void OnAppReady(BLEAppBase *app);
    virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
    virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
    virtual void OnDataRead(const GattReadCallbackParams *params);
    virtual void OnHVX(const GattHVXCallbackParams *params);
    virtual void OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params);
    virtual void OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params);
    void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params);
    bool IsScanActive();
    void StartScan(uint32_t timeout = 0);
    uint32_t GetScanTimeMs();
    void StopScan();
    virtual void OnScanStopped();
    virtual void OnInitDone();
    void OnInitialized(BLE::InitializationCompleteCallbackContext *event);
    void ScheduleBleEvents(BLE::OnEventsToProcessCallbackContext *event);
    void Start();
    bool IsSameId128(const uint8_t *a, const uint8_t *b);

protected:
    events::EventQueue event_queue_;
    BLE &ble_;
    Timer timer_;
    BikeGUI *gui_;
    bool scanning_active_;
    uint32_t start_scan_ms_;
};

#endif
