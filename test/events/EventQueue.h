#ifndef TEST_EVENT_QUEUE_H
#define TEST_EVENT_QUEUE_H

#include <stdint.h>

namespace events {


class EventQueue
{
    public:
    void call_every(uint32_t ms, void* ptr) {}
    uint32_t call_in(uint32_t ms, void* ptr) {}
    void cancel(uint32_t id) {}
};

}

#endif