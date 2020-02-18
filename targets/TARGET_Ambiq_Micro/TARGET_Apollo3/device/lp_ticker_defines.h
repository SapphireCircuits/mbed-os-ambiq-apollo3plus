// SparkFun Electronics 2019

// #ifndef MBED_US_TICKER_DEFINES_H
// #define MBED_US_TICKER_DEFINES_H

#include "am_mcu_apollo.h"

#define LP_TICKER_FREQ 32768
#define LP_TICKER_AM_HAL_CTIMER_SRC AM_HAL_CTIMER_XT_32_768KHZ

#define LP_TICKER_AM_HAL_CTIMER_SEGMENT_TIME_KEEPER AM_HAL_CTIMER_TIMERA
#define LP_TICKER_AM_HAL_CTIMER_SEGMENT_INT_COUNTER AM_HAL_CTIMER_TIMERB
#define LP_TICKER_AM_HAL_CTIMER_NUMBER 7
#define LP_TICKER_AM_HAL_CTIMER_TIME_KEEPER_FN AM_HAL_CTIMER_FN_CONTINUOUS
#define LP_TICKER_AM_HAL_CTIMER_INT_COUNTER_FN AM_HAL_CTIMER_FN_ONCE
#define LP_TICKER_AM_HAL_CTIMER_CMPR_REG 0 // CMPR0 reg used with CTIMER_FN_CONTINUOUS mode
#define LP_TICKER_AM_HAL_CTIMER_CMPR_INT AM_HAL_CTIMER_INT_TIMERB7C0
//#define LP_TICKER_AM_HAL_CTIMER_CMPR_INT1 AM_HAL_CTIMER_INT_TIMERA7C1

// Automatic configuration

#if (LP_TICKER_AM_HAL_CTIMER_SEGMENT == AM_HAL_CTIMER_BOTH)
#define LP_TICKER_BITS 32
#else
#define LP_TICKER_BITS 16
#endif // (LP_TICKER_AM_HAL_CTIMER_SEGMENT == AM_HAL_CTIMER_BOTH)

// #endif // MBED_US_TICKER_DEFINES_H