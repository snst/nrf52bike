#ifndef APP_CSC_H_
#define APP_CSC_H_

#include "mbed.h"
#include <stdint.h>
#include "IUICsc.h"
#include "events/EventQueue.h"

#define SPEED_FILTER_VALUES_CNT 5u
#define SPEED_FILTER_VALUES_MAX (SPEED_FILTER_VALUES_CNT-1u)

class AppCsc
{
  public:
    AppCsc();
    virtual ~AppCsc();
    bool ProcessData(uint32_t now_ms, const uint8_t *data, uint32_t len);
    void CalculateAverageSpeed();

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
    uint16_t filtered_speed_kmhX10_[SPEED_FILTER_VALUES_MAX];
    bool is_init_;
};

#endif
