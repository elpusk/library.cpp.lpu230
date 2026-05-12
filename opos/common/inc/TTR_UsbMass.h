
#if !defined(__TTRL_USB_MASSSTORAGE__HEADER_20100402__)
#define __TTRL_USB_MASSSTORAGE__HEADER_20100402__


#include <winioctl.h>

//////////////////////////////////////////////////////////////////////////////////
//this constants is used to lock volume.
#define TTR_USBMASS_LOCK_TIMEOUT		10000       // 10 Seconds
#define TTR_USBMASS_LOCK_RETRIES		20			// retry for lock volume

// internal constatns definition
#define	TTR_USBMASS_SECTOR_SIZE		512


#define TTR_USBMASS_BYTES_PER_KB                    1024LL
#define TTR_USBMASS_BYTES_PER_MB                    1048576LL
#define TTR_USBMASS_BYTES_PER_GB                    1073741824LL
#define TTR_USBMASS_BYTES_PER_TB                    1099511627776LL
#define TTR_USBMASS_BYTES_PER_PB                    1125899906842624LL



///////////////////////////
// Usb masstorage class
//////////////////////////

class CTTR_UsbMass{

public:
 
	enum TTR_UsbMass_DriveType	// Declare enum driver type
	{
		TTR_UMDType_Unknown = 0,	//unknown drive type.
		TTR_UMDType_Fixed,			//fixed drive type.
		TTR_UMDType_Remote,			//remote drive type.
		TTR_UMDType_RAM,			//RAM drive type.
		TTR_UMDType_CD,				//CD drive type.
		TTR_UMDType_Remove			//Removable type.
	};


	typedef struct tag_FATPARAMS
	{
		char volume_name[11];
		UINT32 num_sectors;	/* total number of sectors */
		INT32 cluster_count;	/* number of clusters */
		INT32 size_root_dir;	/* size of the root directory in bytes */
		INT32 size_fat;		/* size of FAT */
		INT32 fats;
		INT32 media;
		INT32 cluster_size;
		INT32 fat_length;
		unsigned __int16 dir_entries;
		unsigned __int16 sector_size;
		INT32 hidden;
		__int16 reserved;
		unsigned __int16 sectors;
		UINT32 total_sect;

		unsigned __int16 heads;
		unsigned __int16 secs_track;

	}FATPARAMS;

	//method
public:

	//format usb drive for tylenol
	static BOOL FormatDisk( TCHAR cDrv );

	// drive' vender & productor is matching.
	static BOOL IsMatchDriveLetterAndProduct( 
		TCHAR cDrv,
		LPCTSTR psVender,
		LPCTSTR psProduct
		);

		
	//eject given letter's usb drive
	//nLockTry : retry number.
	static BOOL	EjectVolume(TCHAR cDriveLetter,INT32 nLockTry,UINT32 dwTimeOut);



	//Get current usb disk letters
	static INT GetUsbDiskLetter(PTCHAR psOutDiskLetters);

	static UINT32 GetDriveBitsFromDriveLetters( PTCHAR psDiskLetters );
	static INT GetDriveLettersFromDriveBits( PTCHAR psOutDrv,UINT32 dwDrv );

	static INT GetOnlyChangedUsbDiskLetter(PTCHAR psOutDiskLetters,PTCHAR psCurDiskLetters);

protected:
private:
	static HANDLE OpenVolume(TCHAR cDriveLetter);
	static BOOL	LockVolume(HANDLE hVolume);
	static BOOL	DismountVolume(HANDLE hVolume);
	static BOOL	PreventRemovalOfVolume(HANDLE hVolume, BOOL fPrevent);
	static BOOL	AutoEjectVolume(HANDLE hVolume);
	static BOOL	CloseVolume(HANDLE hVolume);

	static char chFirstDriveFromMask (ULONG unitmask);

	static BOOL GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc);

	static void PutBoot (FATPARAMS * ft, unsigned char *boot);
	static void PutFSInfo (unsigned char *sector, FATPARAMS *ft);
	static BOOL FlushFormatWriteBuffer (HANDLE dev, char *write_buf, INT32 *write_buf_cnt);
	static BOOL WriteSector (void *dev, char *sector, char *write_buf, INT32 *write_buf_cnt,unsigned __int64 *nSecNo);
	static void GetFatParams (FATPARAMS * ft);
	static INT32 FormatFat (unsigned __int64 startSector, FATPARAMS * ft, HANDLE dev, BOOL quickFormat);


	//////////////////////////////////////////////////
	//variables
private:
	static CONST INT32 m_nFormatWriteBufferSize = 1024 * 1024;

};


#endif	//__TTRL_USB_MASSSTORAGE__HEADER_20100402__