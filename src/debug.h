/* 
 * File:   m2debug.h
 * Author: xavierlange
 *
 * Created on September 17, 2011, 6:51 PM
 */

#include "stdint.h"

#ifndef M2DEBUG_H
#define	M2DEBUG_H

void mongrel2_debug_frame(size_t len, uint8_t* header);

#endif	/* M2DEBUG_H */