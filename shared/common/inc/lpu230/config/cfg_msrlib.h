/*
        cfg_lib.h
        
        Copyright 2009 yss-totoro <totoro team>
        
        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.
        
        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.
        
        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
        MA 02110-1301, USA.

		WARNING THIS FILE MUST BE COPYED TO USER PROJECT FOLDER & MODIFIY
		- DON'T CHANGE ORIGINAL FILE. AFTER COPYING, MODIFIY THIS
*/

#if !defined( _CFG_MSR_LIB_HEADER_201002220001_ )
#define _CFG_MSR_LIB_HEADER_201002220001_

// this header will configurate librarys of totoro teams.


/* 
 * 
 * for base driver configuration
 * 
 */
 
/* 
 * 
 * for MSR basic driver configuration
 * 
 */
#define	CONF_USE_FUN_GETPADDATA		//use MSROBJ_GetPaddingData()

//#define	CONF_USE_FUN_DEF_MSRREAD		//use default MSROBJ_Read()
//#define	CONF_USE_FUN_DEF_MSRWRITE		//use default MSROBJ_Write()
//#define	CONF_USE_FUN_DEF_MSRRDD			//use default MSROBJ_RDDCollect()
//#define	CONF_USE_OPTIMAZATION_8051		//optimize default MSROBJ_RDDCollect() for 8051 core.......

#define		CONF_USE_TABLE_ASCTO_ENG_HIDKEY		//use Ascii to english USB HID key mapping table
#define		CONF_USE_TABLE_ASCTO_ENG_PS2KEY		//use Ascii to english PS2 key mapping table

#define		CONF_USE_TABLE_ASCTO_SPANISH_HIDKEY		//use Ascii to spanish USB HID key mapping table
#define		CONF_USE_TABLE_ASCTO_SPANISH_PS2KEY		//use Ascii to spanish PS2 key mapping table

/* 
 * 
 * for MSR object information configuration
 * 
 */
#define		CONF_MSROBJ_NUMBER			3		//THE NUMBER OF MSR OBJECT
#define		CONF_MSROBJ_NUMBER_COMB		3		//THE NUMBER OF COMBINATION OF ONE MSR-OBJECT
#define		CONF_MSROBJ_RAW_BUFFER_SIZE	0		//the sum of all raw buffer'size. 0 - automatically determined by system.


#endif	//_CFG_MSR_LIB_HEADER_201002220001_
