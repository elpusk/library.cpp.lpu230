
#if !defined(__TTRL_TYLENOL_COMMNAD_HEADER_2010040900__)
#define __TTRL_TYLENOL_COMMNAD_HEADER_2010040900__


///////////////////////////////////////////////
//////////////////////////////////////////////	
#include "TTR_def.h"
#include "TTR_tchar.h"
#include "windows.h"


#pragma pack(push,8)

class CTTR_TylenolCmd
{
	/////////////////////
	//internal type part 
public:

	/////////////////////////////
	//tylenol device command

	static CONST TCHAR STR_CMD_FORMAT[TTR_DEF_STR_BUF_SIZE];		//format command string
	static CONST TCHAR STR_CMD_DELETE[TTR_DEF_STR_BUF_SIZE];		//delete file command string
	static CONST TCHAR STR_CMD_COPY[TTR_DEF_STR_BUF_SIZE];			//copy file command string

	static CONST TCHAR STR_ROM_FILENAME[TTR_DEF_STR_BUF_SIZE];		//tylenol file name

};

#pragma pack(pop)

/////////////////////////////////////////


#endif	//__TTRL_TYLENOL_COMMNAD_HEADER_2010040900__