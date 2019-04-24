#ifndef BLEAPPBASE_H_
#define BLEAPPBASE_H_

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
#include "IAppCallback.h"

#include "tracer.h"

class BikeGUI;

#define MAX_NAME (7u)

class BLEAppBase : private mbed::NonCopyable<BLEAppBase>
{
  typedef BLEAppBase Self;

public:
  BLEAppBase(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, char* name);
  virtual ~BLEAppBase();
  void SetGUI(BikeGUI *gui);
  bool HaveFoundDevice();
  const Gap::Handle_t &GetConnectionHandle();
  bool Connect();
  virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
  bool IsConnected();
  void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
  virtual void OnServiceFound(const DiscoveredService *service);
  virtual void OnServiceCharacteristicFound(const DiscoveredCharacteristic *param);
  virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle);
  void StartServiceDiscovery();
  virtual void OnCharacteristicDescriptorsFound(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params);
  virtual void OnAppReady();
  virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
  virtual void StartCharacteristicDescriptorsDiscovery(DiscoveredCharacteristic &characteristic);
  virtual void RequestNotify(bool enable);
  void ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc);
  virtual void OnDataRead(const GattReadCallbackParams *params);
  virtual void OnHVX(const GattHVXCallbackParams *params);
  void on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb);
  void SetDeviceAddress(const Gap::AdvertisementCallbackParams_t *params);
  bool HasAddress(const BLEProtocol::AddressBytes_t &peerAddr);
  void FindCharacteristic(uint16_t id);
  bool FoundCharacteristic();
  bool FoundDescNotify();
  DiscoveredCharacteristic &GetCharacteristic();
  void SetAppCallback(IAppCallback* cb);
    
  template <typename ContextType>
  FunctionPointerWithContext<ContextType> as_cb(void (Self::*member)(ContextType context))
  {
    return makeFunctionPointer(this, member);
  }

protected:
  events::EventQueue &event_queue_;
  BLE &ble_;
  mbed::Callback<void(BLE &, events::EventQueue &)> _post_init_cb;

  Gap::Handle_t connection_handle_;
  GattAttribute::Handle_t descriptor_handle_;
  DiscoveredCharacteristic characteristic_;
  DiscoveredCharacteristicDescriptor desc2902_;
  BLEProtocol::AddressBytes_t device_addr_;
  BLEProtocol::AddressType_t device_addr_type_;
  uint16_t characteristic_id;
  bool found_characteristic;
  bool found_desc2902_;
  bool found_device_;
  Timer &timer_;
  BikeGUI *gui_;
  IAppCallback* app_callback_;
  char name_[MAX_NAME+1];

};

#endif
