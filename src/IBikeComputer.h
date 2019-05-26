#ifndef IBIKECOMPUTER_H_
#define IBIKECOMPUTER_H_

#include <mbed.h>
#include "AppId.h"
#include "IUIMode.h"

class IBikeComputer
{
public:
  virtual void Connect(BC::eApp_t app) = 0;
  virtual void SetUiMode(IUIMode::eUiMode_t mode) = 0;
  virtual void SetBacklightBrightness(uint8_t val) = 0;
  virtual uint32_t GetCscDisconnects() = 0;
};

#endif
