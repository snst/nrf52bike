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

public:
  BLEAppBase(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, char* name) : event_queue_(event_queue),
                                                                                  timer_(timer),
                                                                                  ble_(ble_interface),
                                                                                  _post_init_cb(),
                                                                                  desc2902_(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0)),
                                                                                  found_characteristic(false),
                                                                                  found_desc2902_(false),
                                                                                  found_device_(false), 
                                                                                  app_callback_(NULL)
  {
    strcpy(name_, name);
  }

  virtual ~BLEAppBase()
  {
  }

  void SetGUI(BikeGUI *gui)
  {
    gui_ = gui;
  }

  template <typename ContextType>
  FunctionPointerWithContext<ContextType> as_cb(
      void (Self::*member)(ContextType context))
  {
    return makeFunctionPointer(this, member);
  }

  bool HaveFoundDevice()
  {
    return found_device_;
  }

  const Gap::Handle_t &GetConnectionHandle()
  {
    return connection_handle_;
  }

  bool Connect()
  {
    if(found_device_) {
      ble_error_t err = ble_.gap().connect(device_addr_, device_addr_type_, NULL, NULL);
      INFO("~BLEAppBase::Connect() => %s, err=0x%x\r\n", name_, err);
      return true;
    }
    return false;
  }

  virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params)
  {
    connection_handle_ = params->handle;
    FLOW("~BLEAppBase::OnConnected() => %s, handle=0x%x\r\n", name_, connection_handle_);
    if (!FoundCharacteristic())
    {
      StartServiceDiscovery();
    }
    else
    {
      OnServiceDiscoveryFinished(connection_handle_);
    }
  }

  void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
  {
    INFO("~BLEAppBase::OnDisconnected() => %s, handle=0x%x, reason=0x%x\r\n", name_, param->handle, param->reason);
    connection_handle_ = 0xFFFF;
    event_queue_.call_in(500, mbed::callback(this, &BLEAppBase::Connect));
  }

  virtual void OnServiceFound(const DiscoveredService *service)
  {
    DBG("~BLEAppBase::OnServiceFound() => %s UUID-%x\r\n", name_, service->getUUID().getShortUUID());
    //DBG("~BLEAppBase::OnServiceFound() LONG-%x %x %x %x %x\r\n", service->getUUID().getBaseUUID()[0], service->getUUID().getBaseUUID()[1], service->getUUID().getBaseUUID()[2], service->getUUID().getBaseUUID()[3], service->getUUID().getBaseUUID()[4]);
  }

  virtual void OnServiceCharacteristicFound(const DiscoveredCharacteristic *param)
  {
    DBG("~BLEAppBase::OnServiceCharacteristicFound() => %s, UUID-%x valueHandle=%u, declHandle=%u, props[%x]\r\n", name_, param->getUUID().getShortUUID(), param->getValueHandle(), param->getDeclHandle(), (uint8_t)param->getProperties().broadcast());
    //DBG("~BLEAppBase::OnServiceCharacteristicFound() LONG-%x %x %x %x %x\r\n", param->getUUID().getBaseUUID()[0], param->getUUID().getBaseUUID()[1], param->getUUID().getBaseUUID()[2], param->getUUID().getBaseUUID()[3], param->getUUID().getBaseUUID()[4]);
    if (param->getUUID().getShortUUID() == characteristic_id)
    {
      INFO("BLEAppBase:: => %s -> characteristic 0x%x\r\n", name_, characteristic_id);
      characteristic_ = *param;
      found_characteristic = true;
    }
  }

  virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle)
  {
    FLOW("~OnServiceDiscoveryFinished() => %s\r\n", name_);

    if (!FoundDescNotify())
    {
      if (FoundCharacteristic())
      {
        StartCharacteristicDescriptorsDiscovery(GetCharacteristic());
      }
    }
    else
    {
      OnCharacteristicDescriptorsFinished(NULL);
    }
  }

  void StartServiceDiscovery()
  {
    ble_.gattClient().onServiceDiscoveryTermination(as_cb(&Self::OnServiceDiscoveryFinished));
    ble_error_t error = ble_.gattClient().launchServiceDiscovery(
        connection_handle_,
        as_cb(&Self::OnServiceFound),
        as_cb(&Self::OnServiceCharacteristicFound));

    INFO("~BLEAppBase::StartServiceDiscovery() => %s, err=0x%x\r\n", name_, error);
  }

  virtual void OnCharacteristicDescriptorsFound(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
  {
    DBG("~BLEAppBase::OnCharacteristicDescriptorsFound() => %s, UUID %x\r\n", name_, params->descriptor.getUUID().getShortUUID());
    if (params->descriptor.getUUID().getShortUUID() == 0x2902)
    {
      DBG("BLEAppBase::  ->  found 0x2902\r\n");
      desc2902_ = params->descriptor;
      found_desc2902_ = true;
      ble_.gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
    }
  }

  virtual void OnAppReady() 
  {
    INFO("~BLEAppBase::OnAppReady() => %s\r\n", name_);
    if (NULL != app_callback_)
    {
      app_callback_->OnAppReady(this);
    }
  }

  virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
  {
    DBG("~BLEAppBase::OnCharacteristicDescriptorsFinished() => %s\r\n", name_);
    OnAppReady();
  }

  virtual void StartCharacteristicDescriptorsDiscovery(DiscoveredCharacteristic &characteristic)
  {
    ble_error_t err = ble_.gattClient().discoverCharacteristicDescriptors(characteristic, as_cb(&Self::OnCharacteristicDescriptorsFound), as_cb(&Self::OnCharacteristicDescriptorsFinished));
    FLOW("~BLEAppBase::StartCharacteristicDescriptorsDiscovery() => %s, err=0x%x\r\n", name_, err);
  }

  virtual void RequestNotify(bool enable)
  {
    uint16_t value = enable ? BLE_HVX_NOTIFICATION : 0;
    ble_error_t err = ble_.gattClient().write(
        GattClient::GATT_OP_WRITE_CMD,
        desc2902_.getConnectionHandle(),
        desc2902_.getAttributeHandle(),
        sizeof(uint16_t),
        (uint8_t *)&value);
    DBG("~BLEAppBase::RequestNotify(%d) => %s, attrHandle=%u, ret=0x%x\r\n", value, name_, desc2902_.getAttributeHandle(), err);
  }

  void ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc) {}

  virtual void OnDataRead(const GattReadCallbackParams *params) {}

  void OnHVX(const GattHVXCallbackParams *params)
  {
  }

  void on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb)
  {
    _post_init_cb = cb;
  }

  void SetDeviceAddress(const Gap::AdvertisementCallbackParams_t *params)
  {
    memcpy(&device_addr_, &params->peerAddr, sizeof(device_addr_));
    device_addr_type_ = params->addressType;
    found_device_ = true;
  }

  bool HasAddress(const BLEProtocol::AddressBytes_t &peerAddr)
  {
    return 0 == memcmp(&device_addr_, &peerAddr, sizeof(device_addr_));
  }

  void FindCharacteristic(uint16_t id)
  {
    characteristic_id = id;
  }

  bool FoundCharacteristic()
  {
    return found_characteristic;
  }

  bool FoundDescNotify()
  {
    return found_desc2902_;
  }

  DiscoveredCharacteristic &GetCharacteristic()
  {
    return characteristic_;
  }

  void SetAppCallback(IAppCallback* cb)
  {
    app_callback_ = cb;
  }
};

#endif
