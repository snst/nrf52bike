#ifndef TRACER_H
#define TRAVER_H

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

#endif