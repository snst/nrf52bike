#ifndef IBIKECOMPUTER_H_
#define IBIKECOMPUTER_H_

#include <mbed.h>

class IBikeComputer
{
public:
  virtual void ConnectCsc() = 0;
  virtual void ConnectKomoot() = 0;
};

#endif
