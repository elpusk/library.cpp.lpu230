#include "stdafx.h"
#include "SyncObject.h"

///////////////////////////////////////////////////////////
//this routine will be used for single instance
//return value
//1. NULL	: already exist the other instance
//2. !NULL	: the first instance of a process
//
//paramter value
//1. lpName : the name of mutex object
//2. bOwnerShip : inital ownership of the mutex object.
HANDLE CreateOneAppMutex(LPCTSTR lpName,BOOL bOwnerShip)
{
    HANDLE hMutex;
    hMutex = CreateMutex(NULL, bOwnerShip, lpName);   // Create mutex
    switch(GetLastError()){
    case ERROR_SUCCESS:
        // Mutex created successfully. There is no instances running
        break;
    case ERROR_ALREADY_EXISTS:
        // Mutex already exists so there is a running instance of our app.
        hMutex = NULL;
        break;
    default:
        // Failed to create mutex by unknown reason
        break;
    }
    return hMutex;
}


LPVOID CreateProcessShareMem( unsigned long nMemSize,LPCTSTR lpcsMemName )
{
	BOOL bFirst;
	HANDLE hMapObj=NULL;
	LPVOID lpShareMem=NULL;

	// Create a named file mapping object.
	hMapObj=CreateFileMapping(
				(HANDLE) 0xFFFFFFFF,// Use paging file
				NULL,				// No security attributes
				PAGE_READWRITE,		// Read/Write access
				0,					// Mem Size: high 32 bits
				nMemSize			// Mem Size: low 32 bits
				lpcsMemName);		// Name of map object
	if(hMapObj!=NULL){
		// Determine if this is the first create of the file mapping.
		bFirst = (ERROR_ALREADY_EXISTS != GetLastError());
	
		// Now get a pointer to the file-mapped shared memory.
		lpShareMem = MapViewOfFile(
					hMapObj,		// File Map obj to view
					FILE_MAP_WRITE,	// Read/Write access
					0,				// high: map from beginning
					0,				// low:
					nMemSize);		// default 0: map entire file
		if(lpShareMem!=NULL)
			if(bFirst){
				// If this is the first attaching process, init the shared memory.
				memset(lpShareMem, 0, nMemSize);
			}
		else
			CloseHandle(hMapObj);
	}
	return lpShareMem;
}

BOOL DeleteProcessShareMem( LPVOID lpShareMem )
{
	// Unmap any shared memory from the process's address space.
	UnmapViewOfFile(lpShareMem);
	// Close the process's handle to the file-mapping object.
	CloseHandle(ghMapObjH);
	return TRUE;
}

