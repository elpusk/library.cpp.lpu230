
#if !defined(__EL_PIN_A_IOCTL_H_)
#define __EL_PIN_A_IOCTL_H_

#include <winioctl.h>

#define IOCTL_RESET_PORT 	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_NEITHER, FILE_ANY_ACCESS)

#endif