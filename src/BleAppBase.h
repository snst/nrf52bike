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
#include "AppId.h"

#include "tracer.h"

#define MAX_NAME (7u)

class BleAppBase : private mbed::NonCopyable<BleAppBase>
{
  typedef BleAppBase Self;

public:
  BleAppBase(events::EventQueue &event_queue, BLE &ble_interface, char* name, BC::eApp_t id);
  virtual ~BleAppBase();
  bool FoundDevice();
  const Gap::Handle_t &GetConnectionHandle();
  virtual bool Connect();
  virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
  bool IsConnected();
  virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
  virtual void OnServiceFound(const DiscoveredService *service);
  virtual void OnServiceCharacteristicFound(const DiscoveredCharacteristic *param);
  virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle);
  void StartServiceDiscovery();
  virtual void OnCharacteristicDescriptorsFound(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params);
  virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
  virtual void StartCharacteristicDescriptorsDiscovery(DiscoveredCharacteristic &characteristic);
  virtual void OnAllServiceAndCharFound();
  virtual void RequestNotify();
  void ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc);
  virtual void OnDataRead(const GattReadCallbackParams *params);
  virtual void OnHVX(const GattHVXCallbackParams *params);
  void SetDeviceAddress(const Gap::AdvertisementCallbackParams_t *params);
  bool HasAddress(const BLEProtocol::AddressBytes_t &peerAddr);
  void FindCharacteristic(uint16_t id);
  bool FoundCharacteristic();
  bool FoundDescNotify();
  DiscoveredCharacteristic &GetCharacteristic();
  bool RequestBatteryLevel();
  const char* GetName();
  BC::eApp_t getId();
    
  template <typename ContextType>
  FunctionPointerWithContext<ContextType> as_cb(void (Self::*member)(ContextType context))
  {
    return makeFunctionPointer(this, member);
  }

protected:
  events::EventQueue &event_queue_;
  BLE &ble_;
  Gap::Handle_t connection_handle_;
  GattAttribute::Handle_t descriptor_handle_;
  DiscoveredCharacteristic characteristic_bat_;
  DiscoveredCharacteristic characteristic_;
  DiscoveredCharacteristicDescriptor desc2902_;
  BLEProtocol::AddressBytes_t device_addr_;
  BLEProtocol::AddressType_t device_addr_type_;
  uint16_t characteristic_id;
  bool found_characteristic_bat_;
  bool found_characteristic_;
  bool found_desc2902_;
  bool found_device_;
  char name_[MAX_NAME+1];
  bool bat_requested_;
  BC::eApp_t app_id_;
};

#endif
