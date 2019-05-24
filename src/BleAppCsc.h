#ifndef BLE_APP_CSC_H_
#define BLE_APP_CSC_H_

#include "BleAppBase.h"
#include "AppCsc.h"

class IUICsc;

class BleAppCsc : public BleAppBase
{
  public:
    BleAppCsc(events::EventQueue &event_queue, BLE &ble_interface, IUICsc* sink);
    virtual ~BleAppCsc();
    void OnHVX(const GattHVXCallbackParams *params);
    void OnDataRead(const GattReadCallbackParams *params);
    virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
    virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
    virtual bool Connect();

  protected:
    AppCsc csc_;
    IUICsc* ui_;
};

#endif
