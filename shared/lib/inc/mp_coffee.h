#pragma once

namespace _mp{

    /**
    * namespace of coffee manager
    */
    namespace _coffee{
    
        /**
        * The name used to identify an interprocess mechanism is not portable, even between UNIX systems. For this reason, Boost.Interprocess limits this name to a C++ variable identifier or keyword:
        * Starts with a letter, lowercase or uppercase, such as a letter from a to z or from A to Z. Examples: Sharedmemory, sharedmemory, sHaReDmEmOrY...
        * Can include letters, underscore, or digits. Examples: shm1, shm2and3, ShM3plus4...
        */
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_PIPE_NAME = L"PIPE_NAME_COFFEE_MGMT_CTL_6B092EC7_0D20_4123_8165_2C1DE27C9AAF";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME = L"PIPE_NAME_COFFEE_MGMT_TRACE_036423FC_2189_423D_8D0E_75992725F843";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_PIPE_NAME_FOR_SINGLE = L"PIPE_NAME_COFFEE_MGMT_72F5E19C_A00E_4FFF_9F6F_53AAAB4B7264";//L"Global\\PIPE_NAME_NAME_COFFEE_MGMT_72F5E19C_A00E_4FFF_9F6F_53AAAB4B7264
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE = L"FILE_LOCK_COFFEE_MGMT_E0A38B4D_DBE7_4F77_A657_45BD8A19B923";

        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_REQ = L"GET_OUT_OF_HERE_NOW";
        constexpr const int CONST_N_COFFEE_MGMT_SLEEP_INTERVAL_MMSEC = 5;

#ifdef _WIN32
        ////////////////// WIN
#ifdef _DEBUG
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
        //constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = _mp::cfile::get_path_ProgramData()+"\\elpusk"
        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\ElpuskHidDaemon";
        constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\coffee_service";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_ibutton";
#else
		// release win case - defined in mp_coffee_path.h!
        //constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.crt";
        //constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.key";
        //constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = _mp::cfile::get_path_ProgramData()+"\\elpusk"
        //constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\root";
        //constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d";
		//constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_service";

        //constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\tg_lpu237_dll";
        //constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\tg_lpu237_ibutton";
#endif //_DEBUG
#else
        ////////////////// LINUX
#ifdef _DEBUG
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
        constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon";
		constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/coffee_service";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_ibutton";

#else
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.key";
        constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"/var/log/elpusk";
        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/root";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/elpusk-hid-d";
		constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_service";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/tg_lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/tg_lpu237_ibutton";
#endif //_DEBUG
#endif //_WIN32

    }
}