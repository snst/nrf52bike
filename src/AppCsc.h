#ifndef APP_CSC_H_
#define APP_CSC_H_

#include "mbed.h"
#include <stdint.h>
#include "IDataCsc.h"

#define SPEED_FILTER_VALUES_CNT 3u
#define SPEED_FILTER_VALUES_MAX (SPEED_FILTER_VALUES_CNT-1u)

class AppCsc
{
  public:
    AppCsc(IDataCsc* csc);
    virtual ~AppCsc();
    void UpdateGUI();
    void ProcessData(uint32_t now_ms, const uint8_t *data, uint32_t len);
    void CalculateAverageSpeed();

  //protected:
    typedef struct cscResponse
    {
        uint8_t flags;
        uint32_t wheelCounter;
        uint16_t wheelEvent;
        uint16_t crankCounter;
        uint16_t crankEvent;
    } cscResponse_t;

    cscResponse_t last_csc_;
    IDataCsc::CscData_t csc_data_;
    double wheel_size_cm_;
    bool is_init_;
    IDataCsc* csc_sink_;
    uint16_t filtered_speed_kmhX10_[SPEED_FILTER_VALUES_MAX];
};

#endif
