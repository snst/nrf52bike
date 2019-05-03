#ifndef TRACER_H
#define TRACER_H

#include "IUILog.h"

//#define ENABLE_DEBUG_PRINT
//#define ENABLE_FLOW_PRINT
#define ENABLE_INFO_PRINT

#ifdef ENABLE_DEBUG_PRINT
#define DBG(...)             \
    {                        \
        ::printf(__VA_ARGS__); \
    }
#else
#define DBG(...)
#endif

#ifdef ENABLE_FLOW_PRINT
#define FLOW(...)            \
    {                        \
        ::printf(__VA_ARGS__); \
    }
#else
#define FLOW(...)
#endif

#ifdef ENABLE_INFO_PRINT
#define INFO(...)            \
    {                        \
        ::printf(__VA_ARGS__); \
    }
#else
#define INFO(...)
#endif

extern IUILog* uilog;

static void UILog(const char* str)
{
    if (NULL != uilog) {
        uilog->Log(str);
    }
}

#endif