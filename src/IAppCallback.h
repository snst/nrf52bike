#ifndef IAPP_CALLBACK_H_
#define IAPP_CALLBACK_H_

#include "mbed.h"

class BLEAppBase;

class IAppCallback
{
public:
  virtual void OnAppReady(BLEAppBase *app) = 0;
};

#endif