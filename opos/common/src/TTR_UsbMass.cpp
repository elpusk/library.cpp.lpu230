#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <dbt.h>
#include "TTR_UsbMass.h"
#include <setupapi.h>
#include <Cfgmgr32.h>
#include <Storduid.h>
#include <stdlib.h>
#include <time.h>
#include <atltrace.h>

#pragma comment( lib, "setupapi.lib" )

///////////////////////////////////////////////////



///////////////////////////////////////////////////
// internal function Prototypes



//////////////////////////////////////////////
//format usb drive for tylenol
BOOL CTTR_UsbMass::FormatDisk( TCHAR cDrv )
{
	BOOL bResult = FALSE;
	DWORD dwReturn = 0;
	DISK_GEOMETRY dg;
	HANDLE hDev;
	TCHAR sDrvPath[MAX_PATH];
	DWORD dwResult;


	/////////////////////
	// get drive handle

	_stprintf( sDrvPath, _T("\\\\?\\%c:"), cDrv );

	hDev = CreateFile(sDrvPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	
	if( hDev == INVALID_HANDLE_VALUE )
		return bResult;

	/////////////////////////////////
	// get drive geometry parameters
	bResult = DeviceIoControl( hDev,IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL,0, &dg, sizeof(DISK_GEOMETRY), &dwReturn, NULL );
	if( !bResult ){
		CloseHandle( hDev );
		return bResult;
	}

	/////////////////
	// lock volume
	bResult = DeviceIoControl( hDev, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwReturn, NULL );
	if( !bResult ){
		CloseHandle( hDev );
		return bResult;
	}

	/////////////////////////
	// set format parameters
	FATPARAMS ft;
	unsigned __int64 num_sectors = dg.Cylinders.QuadPart*dg.TracksPerCylinder*dg.SectorsPerTrack;
	unsigned __int64 startSector = 0;
	INT32 clusterSize = 0;//determin cluster size..default
	INT32 nStatus = 0;

	memset( &ft,0,sizeof(FATPARAMS) );


	// Calculate the fats, root dir etc
	ft.num_sectors = (UINT32) (num_sectors);
	ft.cluster_size = clusterSize;
	memcpy( ft.volume_name,"NO NAME    ", 11);//you must use ascii code. Don't unicode.
	GetFatParams (&ft); 


	nStatus = FormatFat (startSector, &ft, hDev, FALSE );

	if( nStatus ){
		CloseHandle( hDev );
		return FALSE;
	}

	/////////////////
	// unlock volume
	bResult = DeviceIoControl( hDev, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwReturn, NULL );
	if( !bResult ){
		CloseHandle( hDev );
		return bResult;
	}

	CloseHandle( hDev );

	return bResult;
}


// drive' vender & productor is matching.
BOOL CTTR_UsbMass::IsMatchDriveLetterAndProduct( 
	TCHAR cDrv,
	LPCTSTR psVender,
	LPCTSTR psProduct
	)
{
	BOOL bResult = FALSE;
	HANDLE hDev;
	
	TCHAR sDrvPath[MAX_PATH];
	BYTE buffer[MAX_PATH];
	DWORD dwRequiredSize=0;
	DWORD dwBytesReturned;

	STORAGE_PROPERTY_QUERY stQuery;
	PSTORAGE_DESCRIPTOR_HEADER pDH;
	PSTORAGE_DEVICE_DESCRIPTOR pDevDes;
	CHAR *psDevVender;
	CHAR *psDevProductor;
	CHAR strTmp[2*(MAX_PATH+1)];

	if( psVender==NULL || psProduct==NULL )
		return bResult;

	//
	_stprintf( sDrvPath, _T("\\\\?\\%c:"), cDrv );
	hDev = CreateFile(sDrvPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	
	if( hDev == INVALID_HANDLE_VALUE ){
		ATLTRACE( _T("OPEN DRV : INVALID_HANDLE_VALUE.\r\n") );
		return bResult;
	}
	else{
		ATLTRACE( _T("OPEN DRV : OK.\r\n") );
	}
	//////////////////////////////////////
	// Get vender string & product string
	stQuery.PropertyId = StorageDeviceProperty;
	stQuery.QueryType = PropertyStandardQuery;

	memset( buffer,0,MAX_PATH );//reset buffer.
	pDH = (PSTORAGE_DESCRIPTOR_HEADER)buffer;
	pDH->Size = MAX_PATH;

	bResult = DeviceIoControl(
			hDev,
			IOCTL_STORAGE_QUERY_PROPERTY,
			&stQuery,
			sizeof(STORAGE_PROPERTY_QUERY),
			pDH,
    		pDH->Size,
			&dwBytesReturned,
			NULL
		);

	CloseHandle(hDev);

	if( !bResult ){
		return bResult;
	}

	bResult = FALSE;

	pDevDes = (PSTORAGE_DEVICE_DESCRIPTOR)buffer;
	psDevVender = (CHAR*)&(buffer[pDevDes->VendorIdOffset]);	//always ASCII code.
	psDevProductor= (CHAR*)&(buffer[pDevDes->ProductIdOffset]);//always ASCII code.

	if( psDevVender ){

		#ifdef UNICODE
		wcstombs(strTmp, (const wchar_t *)psVender, sizeof(strTmp));
		#else
		strcpy( strTmp,(const char*)psVender );
		#endif

		//search given Vender string in device string
		if( strstr( psDevVender,strTmp ) ){
			//find OK.
			bResult = TRUE;
		}
		else{
			return bResult;
		}
	}
	else{
		return bResult;
	}
	
	if( psDevProductor ){

		#ifdef UNICODE
		wcstombs(strTmp, (const wchar_t *)psProduct, sizeof(strTmp));
		#else
		strcpy( strTmp,(const char*)psProduct );
		#endif

		//search given productor string in device string
		if( strstr( psDevProductor,strTmp ) ){
			//find OK.
			bResult = TRUE;
		}
		else{
			bResult = FALSE;
		}

	}
	else{
		bResult = FALSE;
	}
	
	return bResult;
}



HANDLE CTTR_UsbMass::OpenVolume(TCHAR cDriveLetter)
{
	HANDLE hVolume;
	UINT uDriveType;
	TCHAR szVolumeName[8];
	TCHAR szRootName[5];
	DWORD dwAccessFlags;
	LPTSTR szVolumeFormat = TEXT("\\\\.\\%c:");
	LPTSTR szRootFormat = TEXT("%c:\\");
	
	wsprintf(szRootName, szRootFormat, cDriveLetter);

	uDriveType = GetDriveType(szRootName);
	switch(uDriveType) {
		case DRIVE_REMOVABLE:
			dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
			break;
		case DRIVE_CDROM:
			dwAccessFlags = GENERIC_READ;
			break;
		default:
			return INVALID_HANDLE_VALUE;
    }

	wsprintf(szVolumeName, szVolumeFormat, cDriveLetter);

	hVolume = CreateFile(   szVolumeName,
							dwAccessFlags,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL );

	return hVolume;	//INVALID_HANDLE_VALUE
}

BOOL CTTR_UsbMass::CloseVolume(HANDLE hVolume)
{
	return CloseHandle(hVolume);
}

BOOL CTTR_UsbMass::LockVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl(hVolume,
					FSCTL_LOCK_VOLUME,
					NULL, 0,
					NULL, 0,
					&dwBytesReturned,
					NULL);
}

BOOL CTTR_UsbMass::DismountVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl( hVolume,
                            FSCTL_DISMOUNT_VOLUME,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

BOOL CTTR_UsbMass::PreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval)
{
	DWORD dwBytesReturned;
    PREVENT_MEDIA_REMOVAL PMRBuffer;

    PMRBuffer.PreventMediaRemoval = fPreventRemoval;

    return DeviceIoControl( hVolume,
                            IOCTL_STORAGE_MEDIA_REMOVAL,
                            &PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL),
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

BOOL CTTR_UsbMass::AutoEjectVolume(HANDLE hVolume)
{
    DWORD dwBytesReturned;

    return DeviceIoControl( hVolume,
                            IOCTL_STORAGE_EJECT_MEDIA,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

BOOL CTTR_UsbMass::EjectVolume(TCHAR cDriveLetter,INT32 nLockTry,UINT32 dwTimeOut)
{
    HANDLE hVolume;
	DWORD dwSleepAmount;
	INT32 nTryCount;

    BOOL fRemoveSafely = FALSE;
    BOOL fAutoEject = FALSE;
	BOOL bLock = FALSE;

    // Open the volume.
    hVolume = OpenVolume(cDriveLetter);
    if (hVolume == INVALID_HANDLE_VALUE)
        return FALSE;

	dwSleepAmount = dwTimeOut / nLockTry;

    // Lock the volume.
	for (nTryCount = 0; nTryCount < nLockTry; nTryCount++) {
		bLock = LockVolume(hVolume);
		
		if( bLock )
			break;//exit for
			
		Sleep(dwSleepAmount);
	}//end for
	
	if( bLock ){
		// dismount the volume.
		if( DismountVolume(hVolume) ){
			fRemoveSafely = TRUE;
			 
			// Set prevent removal to false and eject the volume.
			if( PreventRemovalOfVolume(hVolume, FALSE) && AutoEjectVolume(hVolume))
				fAutoEject = TRUE;			 
		}
	
	}
	
    // Close the volume so other processes can use the drive.
    if (!CloseVolume(hVolume))
        return FALSE;

    if (fAutoEject){
        //Media in Drive %c has been ejected safely.
	}
    else {
        if (fRemoveSafely){
			//Media in Drive %c can be safely removed.
		}
    }

    return TRUE;
}


/* example eject usb massstorage
void main(int argc, char * argv[])
{
       if (argc != 2) {
           Usage();
           return ;
       }

       if (!TTR_UsbMassEjectVolume(argv[1][0],LOCK_RETRIES,LOCK_TIMEOUT))
           printf("Failure ejecting drive %c.\n", argv[1][0]);

       return ;
}
*/


//****************************************************************************
//
//   FUNCTION: TTR_UsbMassGetOnlyChangedUsbDiskLetter()
//
//    PURPOSE:  get the usb disks only changed drive, and filling the 'szMoveDiskName' with them
//
//	  return : return the number of USB drive.
//	  psOutDiskLetters: the current driver letters - zero string value:
//	  if psOutDiskLetters = "GZ" - changed drive is G and Z.(inserted or removed)
//
//****************************************************************************
INT CTTR_UsbMass::GetOnlyChangedUsbDiskLetter(PTCHAR psOutDiskLetters,PTCHAR psCurDiskLetters)
{
	TCHAR sNewLetter[27];
	INT32 nCnt = 0;
	UINT32 dwCurDrv = 0;
	UINT32 dwNewDrv = 0;
	UINT32 dwChangeDrv = 0;
	
	memset( sNewLetter,0,sizeof(TCHAR)*27 );//reset buffer
	
	GetUsbDiskLetter( sNewLetter );
	
	///////////////////////////////////////////////////////////////
	//convert given the current drive letters to drive bit pattern.
	dwCurDrv = GetDriveBitsFromDriveLetters( psCurDiskLetters );
	
	///////////////////////////////////////////////////////////////
	//convert given the new drive letters to drive bit pattern.
	dwNewDrv = GetDriveBitsFromDriveLetters( (PTCHAR)sNewLetter );
	
	dwChangeDrv = dwCurDrv ^ dwNewDrv;//get change Drive.
	
	// create changed driver bit-pattern to lettters
	
	nCnt = GetDriveLettersFromDriveBits( psOutDiskLetters,dwChangeDrv );

	
	return nCnt;
}

//****************************************************************************
//
//   FUNCTION: TTR_UsbMassGetUsbDiskLetter()
//
//    PURPOSE:  get the usb disks, and filling the 'szMoveDiskName' with them
//
//	  return : return the number of USB drive.
//	  psOutDiskLetters: the current driver letters - zero string value:
//	  if psOutDiskLetters = "CDEFGZ" - exsit drive is C,D,E,F,G and Z.
//
//****************************************************************************
INT CTTR_UsbMass::GetUsbDiskLetter(PTCHAR psOutDiskLetters)
{
	INT32 k = 0;
	UINT32			MaxDriveSet, CurDriveSet;
	UINT32			drive;
	enum TTR_UsbMass_DriveType drivetype;
	TCHAR			szBuf[300];
	HANDLE			hDevice;
	TCHAR   szMoveDiskName[33];
	TCHAR	szDrvName[33];
	PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
	INT32 nDrvCnt = 0;
	
	if( psOutDiskLetters==NULL )
		return nDrvCnt;

	for(k=0; k<26; k++){
		szMoveDiskName[k] = '\0';
	}//end for
		
	k = 1;		
	// Get available drives we can monitor
	MaxDriveSet = CurDriveSet = 0;

	MaxDriveSet = GetLogicalDrives();
	CurDriveSet = MaxDriveSet;
	for ( drive = 0; drive < 32; ++drive )  
	{
		if ( MaxDriveSet & (1 << drive) )  
		{
			UINT32 temp = 1<<drive;
			_stprintf( szDrvName, _T("%c:\\"), 'A'+drive );
			switch ( GetDriveType( szDrvName ) )  
			{
				case 0:					// The drive type cannot be determined.
				case 1:					// The root directory does not exist.
					drivetype = TTR_UMDType_Unknown;
					break;
				case DRIVE_REMOVABLE:	// The drive can be removed from the drive.
					drivetype = TTR_UMDType_Remove;
					psOutDiskLetters[nDrvCnt] = szMoveDiskName[k] = chFirstDriveFromMask(temp);
					szMoveDiskName[0]=k;
					k++;	nDrvCnt++;
					break;
				case DRIVE_CDROM:		// The drive is a CD-ROM drive.
					break;
				case DRIVE_FIXED:		// The disk cannot be removed from the drive.
					drivetype = TTR_UMDType_Fixed;
					_stprintf(szBuf, _T("\\\\?\\%c:"), 'A'+drive);
					hDevice = CreateFile(szBuf, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

					if (hDevice != INVALID_HANDLE_VALUE)
					{

						pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1];

						pDevDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;

						if(GetDisksProperty(hDevice, pDevDesc))
						{
							if(pDevDesc->BusType == BusTypeUsb)
							{
								psOutDiskLetters[nDrvCnt] = szMoveDiskName[k] = chFirstDriveFromMask(temp);
								szMoveDiskName[0]=k;
								k++;	nDrvCnt++;
							}
						}

						delete pDevDesc;
						CloseHandle(hDevice);
					}
					
					break;
				case DRIVE_REMOTE:		// The drive is a remote (network) drive.
					drivetype = TTR_UMDType_Remote;
					break;
				case DRIVE_RAMDISK:		// The drive is a RAM disk.
					drivetype = TTR_UMDType_RAM;
					break;
			}//end switch
			
		}//end if
		
	}//for drive

	psOutDiskLetters[nDrvCnt];//make zero string
	
	return nDrvCnt;
}


//****************************************************************************
//
//    FUNCTION: chFirstDriverFrameMask(ULONG unitmask)
//
//    PURPOSE:  get the logic name of driver
//
//****************************************************************************
char CTTR_UsbMass::chFirstDriveFromMask (ULONG unitmask)
{

      char i;
      for (i = 0; i < 26; ++i)  
      {
           if (unitmask & 0x1) 
				break;
            unitmask = unitmask >> 1;
      }
    return (i + 'A');
}


//****************************************************************************
//
//    FUNCTION: GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
//
//    PURPOSE:  get the info of specified device
//
//****************************************************************************
BOOL CTTR_UsbMass::GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
{
	STORAGE_PROPERTY_QUERY	Query;	// input param for query
	DWORD dwOutBytes;				// IOCTL output length
	BOOL bResult;					// IOCTL return val

	// specify the query type
	Query.PropertyId = StorageDeviceProperty;
	Query.QueryType = PropertyStandardQuery;

	// Query using IOCTL_STORAGE_QUERY_PROPERTY 
	bResult = ::DeviceIoControl(hDevice,			// device handle
			IOCTL_STORAGE_QUERY_PROPERTY,			// info of device property
			&Query, sizeof(STORAGE_PROPERTY_QUERY),	// input data buffer
			pDevDesc, pDevDesc->Size,				// output data buffer
			&dwOutBytes,							// out's length
			(LPOVERLAPPED)NULL);					

	return bResult;
}

UINT32 CTTR_UsbMass::GetDriveBitsFromDriveLetters( PTCHAR psDiskLetters )
{
	TCHAR cDrv;
	UINT32 dwMask = 0;
	UINT32 dwDrv = 0;
	INT32 nShiftCnt = 0;
	
	///////////////////////////////////////////////////////////////
	//convert given the current drive letters to drive bit pattern.
	while( *psDiskLetters!=NULL ){
	
		cDrv = *psDiskLetters;//get drive letter
		nShiftCnt = cDrv-'A';//get shift counter.
		dwMask = 0x00000001 << nShiftCnt;//make mask pattern.
		
		dwDrv = dwDrv | dwMask;//make current drive bit pattern.
	
		psDiskLetters++;
	}//end while
	
	return dwDrv;
}

INT CTTR_UsbMass::GetDriveLettersFromDriveBits( PTCHAR psOutDrv,UINT32 dwDrv )
{
	INT32 nCnt = 0;
	INT32 i;
	UINT32 dwMask = 0x00000001;
	TCHAR sLetters[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	if( dwDrv == 0 )
		return nCnt;
	
	// create changed driver bit-pattern to lettters
	for( i=0; i< 26; i++ ){
	
		if( dwMask & dwDrv ){
			//save drive letter & increase drive-counter.
			*psOutDrv = sLetters[i];
			psOutDrv++;
			nCnt++;
		}
		
		dwMask = dwMask << 1;//adjust mask pattern.
	
	}//end for
	
	
	return nCnt;//return drive count.
}

////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void CTTR_UsbMass::PutBoot (FATPARAMS * ft, unsigned char *boot)
{
	INT32 cnt = 0;

	boot[cnt++] = 0xeb;	/* boot jump */
	boot[cnt++] = 0x3c;
	boot[cnt++] = 0x90;
	memcpy (boot + cnt, "MSDOS5.0", 8); /* system id */
	cnt += 8;
	*(__int16 *)(boot + cnt) = ft->sector_size;	/* bytes per sector */
	cnt += 2;
	boot[cnt++] = (__int8) ft->cluster_size;			/* sectors per cluster */
	*(__int16 *)(boot + cnt) = ft->reserved;		// reserved sectors 04 - 02
	cnt += 2;
	boot[cnt++] = (__int8) ft->fats;					/* 2 fats */

	if(ft->size_fat == 32)
	{
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
	}
	else
	{
		*(__int16 *)(boot + cnt) = ft->dir_entries;	/* 512 root entries */
		cnt += 2;
	}

	*(__int16 *)(boot + cnt) = ft->sectors;		/* # sectors */
	cnt += 2;
	boot[cnt++] = (__int8) ft->media;					/* media byte */

	if(ft->size_fat == 32)	
	{
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
	}
	else 
	{ 
		*(__int16 *)(boot + cnt) = (unsigned __int16) ft->fat_length;	/* fat size */
		cnt += 2;
	}

	*(__int16 *)(boot + cnt) = ft->secs_track;	/* # sectors per track */
	cnt += 2;
	*(__int16 *)(boot + cnt) = ft->heads;		/* # heads */
	cnt += 2;
	*(__int32 *)(boot + cnt) = ft->hidden;		/* # hidden sectors */
	cnt += 4;
	*(__int32 *)(boot + cnt) = ft->total_sect;	/* # huge sectors */
	cnt += 4;

	if(ft->size_fat == 32)
	{
		*(__int32 *)(boot + cnt) = ft->fat_length; cnt += 4;	/* fat size 32 */
		boot[cnt++] = 0x00;	/* ExtFlags */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;	/* FSVer */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x02;	/* RootClus */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x01;	/* FSInfo */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x06;	/* BkBootSec */
		boot[cnt++] = 0x00;
		memset(boot+cnt, 0, 12); cnt+=12;	/* Reserved */
	}

	boot[cnt++] = 0x00;	/* drive number */   // FIXED 80 > 00
	boot[cnt++] = 0x00;	/* reserved */
	boot[cnt++] = 0x29;	/* boot sig */

	// volume ID
	unsigned char cRad;

	for( INT32 i=0; i<4; i++ ){

		srand(time(NULL)+rand());
		cRad = (unsigned char)(rand() % 256);

		//
		*(boot + cnt +i) = cRad;
	}//end for

	cnt += 4;

	memcpy (boot + cnt, ft->volume_name, 11);	/* vol title */
	cnt += 11;

	switch(ft->size_fat) /* filesystem type */
	{
		case 12: memcpy (boot + cnt, "FAT12   ", 8); break;
		case 16: memcpy (boot + cnt, "FAT16   ", 8); break;
		case 32: memcpy (boot + cnt, "FAT32   ", 8); break;
	}
	cnt += 8;

	memset (boot + cnt, 0, ft->size_fat==32 ? 420:448);	/* boot code */
	cnt += ft->size_fat==32 ? 420:448;
	boot[cnt++] = 0x55;
	boot[cnt++] = 0xaa;	/* boot sig */
}


/* FAT32 FSInfo */
void CTTR_UsbMass::PutFSInfo (unsigned char *sector, FATPARAMS *ft)
{
	memset (sector, 0, 512);
	sector[3]=0x41; /* LeadSig */
	sector[2]=0x61; 
	sector[1]=0x52; 
	sector[0]=0x52; 
	sector[484+3]=0x61; /* StrucSig */
	sector[484+2]=0x41; 
	sector[484+1]=0x72; 
	sector[484+0]=0x72; 

	// Free cluster count
	*(unsigned __int32 *)(sector + 488) = ft->cluster_count - ft->size_root_dir / TTR_USBMASS_SECTOR_SIZE / ft->cluster_size;

	// Next free cluster
	*(unsigned __int32 *)(sector + 492) = 2;

	sector[508+3]=0xaa; /* TrailSig */
	sector[508+2]=0x55;
	sector[508+1]=0x00;
	sector[508+0]=0x00;
}




BOOL CTTR_UsbMass::FlushFormatWriteBuffer (HANDLE dev, char *write_buf, INT32 *write_buf_cnt)
{
	DWORD bytesWritten;

	if (*write_buf_cnt == 0)
		return TRUE;

	if (!WriteFile ((HANDLE) dev, write_buf, *write_buf_cnt, &bytesWritten, NULL)){
			return FALSE;
	}

	*write_buf_cnt = 0;
	return TRUE;
}

BOOL CTTR_UsbMass::WriteSector (void *dev, char *sector, char *write_buf, INT32 *write_buf_cnt,unsigned __int64 *nSecNo)
{

	(*nSecNo)++;

	memcpy (write_buf + *write_buf_cnt, sector, TTR_USBMASS_SECTOR_SIZE);
	(*write_buf_cnt) += TTR_USBMASS_SECTOR_SIZE;

	if (*write_buf_cnt == m_nFormatWriteBufferSize && !FlushFormatWriteBuffer (dev, write_buf, write_buf_cnt))
		return FALSE;
	
	return TRUE;

}


void CTTR_UsbMass::GetFatParams (FATPARAMS * ft)
{
	UINT32 fatsecs;

	if(ft->cluster_size == 0)	// 'Default' cluster size
	{
		if (ft->num_sectors * 512LL >= 256*TTR_USBMASS_BYTES_PER_GB)
			ft->cluster_size = 128;
		else if (ft->num_sectors * 512LL >= 64*TTR_USBMASS_BYTES_PER_GB)
			ft->cluster_size = 64;
		else if (ft->num_sectors * 512LL >= 16*TTR_USBMASS_BYTES_PER_GB)
			ft->cluster_size = 32;
		else if (ft->num_sectors * 512LL >= 8*TTR_USBMASS_BYTES_PER_GB)
			ft->cluster_size = 16;
		else if (ft->num_sectors * 512LL >= 128*TTR_USBMASS_BYTES_PER_MB)
			ft->cluster_size = 8;
		else if (ft->num_sectors * 512LL >= 64*TTR_USBMASS_BYTES_PER_MB)
			ft->cluster_size = 4;
		else if (ft->num_sectors * 512LL >= 32*TTR_USBMASS_BYTES_PER_MB)
			ft->cluster_size = 2;
		else
			ft->cluster_size = 1;
	}

	// Geometry always set to SECTORS/1/1
	ft->secs_track = 1; 
	ft->heads = 1; 

	ft->dir_entries = 512;
	ft->fats = 2;
	ft->media = 0xf8;
	ft->sector_size = TTR_USBMASS_SECTOR_SIZE;
	ft->hidden = 0;

	ft->size_root_dir = ft->dir_entries * 32;

	// FAT12
	ft->size_fat = 12;
	ft->reserved = 4;//2;
	fatsecs = ft->num_sectors - (ft->size_root_dir + TTR_USBMASS_SECTOR_SIZE - 1) / TTR_USBMASS_SECTOR_SIZE - ft->reserved;
	ft->cluster_count = (INT32) (((__int64) fatsecs * TTR_USBMASS_SECTOR_SIZE) / (ft->cluster_size * TTR_USBMASS_SECTOR_SIZE + 3));
	ft->fat_length = (((ft->cluster_count * 3 + 1) >> 1) + TTR_USBMASS_SECTOR_SIZE - 1) / TTR_USBMASS_SECTOR_SIZE;

	if (ft->cluster_count >= 4085) // FAT16
	{
		ft->size_fat = 16;
		ft->reserved = 4;//2;
		fatsecs = ft->num_sectors - (ft->size_root_dir + TTR_USBMASS_SECTOR_SIZE - 1) / TTR_USBMASS_SECTOR_SIZE - ft->reserved;
		ft->cluster_count = (INT32) (((__int64) fatsecs * TTR_USBMASS_SECTOR_SIZE) / (ft->cluster_size * TTR_USBMASS_SECTOR_SIZE + 4));
		ft->fat_length = (ft->cluster_count * 2 + TTR_USBMASS_SECTOR_SIZE - 1) / TTR_USBMASS_SECTOR_SIZE;
	}
	
	if(ft->cluster_count >= 65525) // FAT32
	{
		ft->size_fat = 32;
		ft->reserved = 32;
		fatsecs = ft->num_sectors - ft->reserved;
		ft->size_root_dir = ft->cluster_size * TTR_USBMASS_SECTOR_SIZE;
		ft->cluster_count = (INT32) (((__int64) fatsecs * TTR_USBMASS_SECTOR_SIZE) / (ft->cluster_size * TTR_USBMASS_SECTOR_SIZE + 8));
		ft->fat_length = (ft->cluster_count * 4 + TTR_USBMASS_SECTOR_SIZE - 1) / TTR_USBMASS_SECTOR_SIZE;
	}

	if (ft->num_sectors >= 65536 || ft->size_fat == 32)
	{
		ft->sectors = 0;
		ft->total_sect = ft->num_sectors;
	}
	else
	{
		ft->sectors = (unsigned __int16) ft->num_sectors;
		ft->total_sect = 0;
	}
}


INT32 CTTR_UsbMass::FormatFat (unsigned __int64 startSector, FATPARAMS * ft, HANDLE dev, BOOL quickFormat)
{
	INT32 write_buf_cnt = 0;
	char sector[TTR_USBMASS_SECTOR_SIZE], *write_buf;
	unsigned __int64 nSecNo = startSector;
	INT32 x, n;

	LARGE_INTEGER startOffset;
	LARGE_INTEGER newOffset;

	// Seek to start sector
	startOffset.QuadPart = startSector * TTR_USBMASS_SECTOR_SIZE;
	if (!SetFilePointerEx ((HANDLE) dev, startOffset, &newOffset, FILE_BEGIN)
		|| newOffset.QuadPart != startOffset.QuadPart)
	{
		return -1;//ERR_VOL_SEEKING;
	}

	// Write the data area

	write_buf = (char *)malloc (m_nFormatWriteBufferSize);
	if (!write_buf)
		return -1;//ERR_OUTOFMEMORY;

	memset (sector, 0, sizeof (sector));

	PutBoot (ft, (unsigned char *) sector);//<<this function>>
	if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
		goto fail;

	// fat32 boot area
	if (ft->size_fat == 32)				
	{
		// fsinfo
		PutFSInfo((unsigned char *) sector, ft);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
			goto fail;

		// reserved 
		while (nSecNo - startSector < 6)
		{
			memset (sector, 0, sizeof (sector));
			sector[508+3]=0xaa; /* TrailSig */
			sector[508+2]=0x55;
			if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
				goto fail;
		}
		
		// bootsector backup
		memset (sector, 0, sizeof (sector));
		PutBoot (ft, (unsigned char *) sector);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
			goto fail;

		PutFSInfo((unsigned char *) sector, ft);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
			goto fail;
	}

	// reserved
	while (nSecNo - startSector < (UINT32)ft->reserved)
	{
		memset (sector, 0, sizeof (sector));
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
			goto fail;
	}

	// write fat
	for (x = 1; x <= ft->fats; x++)
	{
		for (n = 0; n < ft->fat_length; n++)
		{
			memset (sector, 0, TTR_USBMASS_SECTOR_SIZE);

			if (n == 0)
			{
				unsigned char fat_sig[12];
				if (ft->size_fat == 32)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = fat_sig[2] = 0xff;
					fat_sig[3] = 0x0f;
					fat_sig[4] = fat_sig[5] = fat_sig[6] = 0xff;
					fat_sig[7] = 0x0f;
					fat_sig[8] = fat_sig[9] = fat_sig[10] = 0xff;
					fat_sig[11] = 0x0f;
					memcpy (sector, fat_sig, 12);
				}				
				else if (ft->size_fat == 16)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = 0xff;
					fat_sig[2] = 0xff;
					fat_sig[3] = 0xff;
					memcpy (sector, fat_sig, 4);
				}
				else if (ft->size_fat == 12)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = 0xff;
					fat_sig[2] = 0xff;
					fat_sig[3] = 0x00;
					memcpy (sector, fat_sig, 4);
				}
			}

			if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
				goto fail;
		}
	}


	// write rootdir
	for (x = 0; x < ft->size_root_dir / TTR_USBMASS_SECTOR_SIZE; x++)
	{
		memset (sector, 0, TTR_USBMASS_SECTOR_SIZE);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo) == FALSE)
			goto fail;

	}

	// Fill the rest of the data area with random data 
	// Not anything for high performance.


	//flush  
	if (!FlushFormatWriteBuffer (dev, write_buf, &write_buf_cnt))
		goto fail;

	free (write_buf);
	return 0;//success

fail:

	free (write_buf);
	return -1;//ERR_OS_ERROR;
}





