
/*
 * system command set header
 *
 */

#ifndef _COMPILER_201007260001H_
#define _COMPILER_201007260001H_

#ifdef	_WIN32
//visual studio plathome

//byte alignment
#define	COMPILER_ATTRIBUTE_BYTE_ALIGNMENT

#else
//gcc
#define	COMPILER_ATTRIBUTE_BYTE_ALIGNMENT	__attribute__ ((packed))

#endif	//_WIN32

#endif	//_COMPILER_201007260001H_
