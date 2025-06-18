#ifndef AVIONICS_INCLUDE_SOAR_DATA_H
#define AVIONICS_INCLUDE_SOAR_DATA_H

#include "SystemDefines.hpp"

/* Structs containing data primitives */

/*
 * IMPORTANT NOTE:
 *  Despite being typed as int32 or uint32 these are actually not integers.
 *  They represent fixed point decimal numbers.
 *
 * The specified precision is not consistent across all instruments,
 * please see the design manual for more information.
 */

typedef struct
{
    int32_t     voltage_; // Volts * 1000, eg. 3300 == 3.3V
} BatteryData;

/* Data Containers */

/*
 * This is meant to act as a pointer to the other data structs.
 */

typedef struct
{
    BatteryData*       				batteryData_;
} AllData;

#endif //AVIONICS_INCLUDE_SOAR_DATA_H
