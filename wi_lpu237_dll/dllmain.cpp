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
#include <mp_coffee_path.h>

#include <cdll_ini.h>

#include <manager_of_device_of_client.h>
#include <cdef.h>
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
    std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
    std::wstring s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

    cdll_ini& cini(cdll_ini::get_instance());
	bool b_ini = cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_lpu237_dll_ini_file());

    //setup tracing system
    _mp::clog& log(_mp::clog::get_instance());
    log.enable_trace(s_pipe_name_of_trace, false); //disable trace. dll cannot be enabled trace.

    //setup logging system
    log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"),std::wstring(L"tg_lpu237_dll"), std::wstring(L"tg_lpu237_dll"));
    log.remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
    log.enable(cini.get_log_enable());

    log.log_fmt(L"[I] START tg_lpu237_dll so or dll.\n");
    log.log_fmt(L"%ls",cini.get_string().c_str());

}

void _process_detach()
{
    //manager_of_device_of_client<lpu237_of_client>::get_instance(true);
    //capi_client::get_instance().unload();
}