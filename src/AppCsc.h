#ifndef APP_CSC_H_
#define APP_CSC_H_

#include "mbed.h"
#include <stdint.h>
#include "IUICsc.h"
#include "events/EventQueue.h"

#define FILTER_VALUES_CNT 5u
#define FILTER_VALUES_MAX (FILTER_VALUES_CNT-1u)

class AppCsc
{
  public:
    AppCsc();
    virtual ~AppCsc();
    bool ProcessData(uint32_t now_ms, const uint8_t *data, uint32_t len);
    void CalculateSpeedValues();
    uint16_t AddFilterVal(uint16_t array[], uint16_t val);

  //protected:
    typedef struct cscMsg
    {
        uint8_t flags;
        uint32_t wheelCounter;
        uint16_t wheelEvent;
        uint16_t crankCounter;
        uint16_t crankEvent;
    } cscMsg_t;

    cscMsg_t last_msg_;
    IUICsc::CscData_t data_;
    double wheel_size_cm_;
    uint32_t crank_counter_sum_;
    uint32_t crank_event_sum_;
    uint16_t filtered_speed_kmhX10_[FILTER_VALUES_MAX];
    uint16_t filtered_cadence_[FILTER_VALUES_MAX];
    bool is_init_;
};

#endif
