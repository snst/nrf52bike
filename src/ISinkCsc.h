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

    bool trip_distance_cm_updated;
    bool trip_time_ms_updated;
    bool speed_kmhX10_updated;
    bool filtered_speed_kmhX10_updated;
    bool average_speed_kmhX10_updated;
    bool cadence_updated;
    bool average_cadence_updated;
    bool is_riding_updated;

  } CscData_t;

  virtual void Update(const CscData_t& data) = 0;
};

#endif