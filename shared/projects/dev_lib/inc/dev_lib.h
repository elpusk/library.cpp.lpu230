#pragma once

#include <mp_os_type.h>

/*!
*	return value definition.
*/
#define	DEV_LIB_RESULT_SUCCESS			0		//! processing success.
#define	DEV_LIB_RESULT_ERROR			0xFFFFFFFF	//! processing error.( maybe system or communication error ); (-1)
#define	DEV_LIB_RESULT_CANCEL			0xFFFFFFFE	//! processing is canceled by another reqest.(-2)


#ifndef _WIN32
extern "C" {
#endif

uint32_t _CALLTYPE_ dev_lib_on();

uint32_t _CALLTYPE_ dev_lib_off();

#ifndef _WIN32
}//extern "C"
#endif
