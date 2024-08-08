// dllmain.cpp : Defines the entry point for the DLL application.

// out is tg_lpu237_dll.dll 
// this dll version must be greater then equal v6.0.
// Why ?
// v1.x is diret io dll
// v3.x is NDM io dll.
// v4.x is dual mode dll( direct & NDM )
// v5.x is - support secure msr
// v6.x is - support win(dll) & linux(so) by wss clients. 

#include <websocket/mp_win_nt.h>
#include <mp_clog.h>

#include <manager_of_device_of_client.h>
#include "lpu237_of_client.h"

static void _process_attach(HINSTANCE hInstance);
static void _process_detach();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        _process_attach(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        _process_detach();
        break;
    }
    return TRUE;
}


void _process_attach(HINSTANCE hInstance)
{
    _mp::clog::get_instance().config(L"elpusk", 6);
    _mp::clog::get_instance().remove_log_files_older_then_now_day(1);
    _mp::clog::get_instance().enable(true);
    _mp::clog::get_instance().log_fmt(L" ***** Attach tg_lpu237_dll.dll *****\n");
}

void _process_detach()
{
    //manager_of_device_of_client<lpu237_of_client>::get_instance(true);
    //capi_client::get_instance().unload();
}