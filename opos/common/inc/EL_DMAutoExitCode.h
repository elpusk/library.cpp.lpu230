#if !defined(__EL_DM_AUTO_EXIT_CODE_HEADER__)
#define __EL_DM_AUTO_EXIT_CODE_HEADER__

//this constants return value of autodownload EXE program
#define	DMAUTO_EXIT_W_ALREADY_EXECUTE		10	//already executed
#define	DMAUTO_EXIT_S_DOWNLOAD				1	//update firmware ok
#define	DMAUTO_EXIT_W_NO_NEED				0	//given firmware already download
#define	DMAUTO_EXIT_E_NOT_FOUND_FILE		-1	//not found firmware file
#define	DMAUTO_EXIT_E_START_WOKER			-2	//failure starting download worker-thread
#define	DMAUTO_EXIT_E_NOT_FOUND_PINPAD		-3	//not found available pinpad.
#define	DMAUTO_EXIT_E_RESPONSE				-4	//pinpad response error at downloading firmware.


#endif//__EL_DM_AUTO_EXIT_CODE_HEADER__