#ifndef IAPP_CALLBACK_H_
#define IAPP_CALLBACK_H_

#include "mbed.h"

class BleAppBase;

class IAppCallback
{
public:
  virtual void OnAppReady(BleAppBase *app) = 0;
};

#endif