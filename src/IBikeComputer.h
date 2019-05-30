#ifndef IBIKECOMPUTER_H_
#define IBIKECOMPUTER_H_

#include <mbed.h>
#include "AppId.h"
#include "IUIMode.h"
#include "IUICsc.h"

class IBikeComputer
{
public:
  virtual void Connect(BC::eApp_t app) = 0;
  virtual void SetUiMode(IUIMode::eUiMode_t mode) = 0;
  virtual void SetBacklightBrightness(uint8_t val) = 0;
  virtual uint32_t GetCscDisconnects() = 0;
  virtual bool IsAppAvailable(BC::eApp_t app_id) = 0;
  virtual uint8_t GetCscBat() = 0;
  virtual IUICsc::CscData_t* GetCscData() = 0;
};

#endif
