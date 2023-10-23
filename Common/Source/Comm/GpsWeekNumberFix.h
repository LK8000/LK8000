/******************************************************************************
 *
 * Copyright (C) u-blox AG
 * u-blox AG, Thalwil, Switzerland
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice is
 * included in all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF
 * THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 *****************************************************************************/
#ifndef _COMM_GPSWEEKNUMBERFIX_H_
#define _COMM_GPSWEEKNUMBERFIX_H_

#include <cstdint>
#include <cstddef>
#include "tchar.h"

/**
 * Convert NMEA $GPRMC date string to integer components 
 * and apply workaround for the GPS week number roll-over issue
 */
bool parse_rmc_date(const char *gprmc, size_t gprmc_size, int32_t &year, int32_t &month, int32_t &day);

#endif // _COMM_GPSWEEKNUMBERFIX_H_