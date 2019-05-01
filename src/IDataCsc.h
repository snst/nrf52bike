#ifndef ICSC_GUI_H_
#define ICSC_GUI_H_

#include "mbed.h"

class IDataCsc
{
public:
  typedef struct CscData {
    uint32_t timestamp_ms;
    uint32_t distance_cm;
    uint32_t time_ms;
    uint32_t total_wheel_rounds;
    uint16_t speed_kmhX10;
    uint16_t filtered_speed_kmhX10;
    uint16_t average_speed_kmhX10;
    uint16_t cadence;
    bool is_riding;
  } CscData_t;

  virtual void Update(const CscData_t& data) = 0;
};

#endif