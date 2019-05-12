#ifndef ICSC_GUI_H_
#define ICSC_GUI_H_

#include "mbed.h"

class ISinkCsc
{
public:
  typedef struct CscData {
    uint32_t timestamp_ms;
    uint32_t trip_distance_cm;
    uint32_t trip_time_ms;
    uint32_t total_wheel_rounds;
    uint16_t speed_kmhX10;
    uint16_t filtered_speed_kmhX10;
    uint16_t average_speed_kmhX10;
    uint16_t cadence;
    uint16_t average_cadence;
    bool is_riding;
  } CscData_t;

  virtual void Update(const CscData_t& data, bool force) = 0;
  virtual void UpdateBat(uint8_t val) = 0;
};

#endif