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
        * Named pipes cannot contain backslashes (\) or forward slashes (/) in their names.
        * The boost::interprocess::message_queue, which internally uses the CreateFile function with these names, 
        * cannot create such filenames in the Windows system.
        */
        constexpr const char* CONST_S_COFFEE_MGMT_CTL_PIPE_NAME = "PIPE_NAME_COFFEE_MGMT_CTL_6B092EC7_0D20_4123_8165_2C1DE27C9AAF";
        constexpr const char* CONST_S_COFFEE_MGMT_CTL_PIPE_NAME_OF_SERVER_RESPONSE = "PIPE_NAME_COFFEE_MGMT_CTL_SV_RSP_867621B4_B234_4823_82EC_D5562655EFA3";
        constexpr const char* CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME = "PIPE_NAME_COFFEE_MGMT_TRACE_036423FC_2189_423D_8D0E_75992725F843";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE = L"FILE_LOCK_COFFEE_MGMT_E0A38B4D_DBE7_4F77_A657_45BD8A19B923";

        /**
        * the definition of CONST_S_COFFEE_MGMT_CTL_PIPE_NAME requests
        */

        // terminate server
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_REQ = L"GET_OUT_OF_HERE_NOW"; 

        // control inner time-interval for optimation
        constexpr const int CONST_N_COFFEE_MGMT_SLEEP_INTERVAL_MMSEC = 500;
        constexpr const int CONST_N_COFFEE_MGMT_CTL_PIPE_READ_INTERVAL_MMSEC = 10;

        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_WOKER_SLEEP_TIME = L"server-woker-sleep-time:";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_PLUG_IN_OUT_SLEEP_TIME = L"dev-plug-in-out-sleep-time:";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_API_TX_LOOP_INTERVAL = L"dev-tx-api-loop-interval-time:";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_API_RX_LOOP_INTERVAL = L"dev-rx-api-loop-interval-time:";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_RX_Q_LOOP_INTERVAL = L"dev-rx-q-loop-interval-time:";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_CTL_PIPE_CHECK_INTERVAL = L"ctl-pipe-check-interval-time:";

        // stop device service prefix string
        // ex) if ypu want to stop a usb device(vid 0x1234, pid 0x4567),
        // send "DEV_STOP:USB:VID_1234:PID_4567" to CONST_S_COFFEE_MGMT_CTL_PIPE_NAME control pipe.
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_REQ_STOP_DEV_PREFIX = L"DEV_STOP:USB:";
        // the sever response of CONST_S_COFFEE_MGMT_CTL_REQ_STOP_DEV_PREFIX req.
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_RSP_STOP_DEV = L"DEV_STOP:USB:OK";

        // start device service prefix string
        // ex) if ypu want to start a usb device(vid 0x1234, pid 0x4567),
        // send "DEV_START:USB:VID_1234:PID_4567" to CONST_S_COFFEE_MGMT_CTL_PIPE_NAME control pipe.
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_REQ_START_DEV_PREFIX = L"DEV_START:USB:";
        // the sever response of CONST_S_COFFEE_MGMT_CTL_REQ_START_DEV_PREFIX req.
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_RSP_START_DEV = L"DEV_START:USB:OK";

        // notify service prefix string
        // ex) if ypu want to start a usb device(vid 0x1234, pid 0x4567), max step 200(0x00C8), current step 23(0x0017), result-error, reason io failure.
        // send "DEV_NOTIFY:USB:VID_1234:PID_4567:CUR_0017:MAX_00c8:RSP_error:WHY_io failure" to CONST_S_COFFEE_MGMT_CTL_PIPE_NAME control pipe.
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_CTL_REQ_NOTIFY_DEV_PREFIX = L"DEV_NOTIFY:USB:";


		constexpr const int CONST_N_MGMT_ALIVE_CHECK_INTERVAL_SEC = 10; //10 sec

		// for firmware update command line argument
		constexpr const wchar_t* CONST_S_CMD_LINE_FW_UPDATE_INDICATOR = L"run_by_coffee_manager_2nd"; // indicator key for coffee manager 2'nd process
		constexpr const wchar_t* CONST_S_CMD_LINE_FW_UPDATE_SET_FW_FILE = L"file"; // rom or bin file full abs path key
		constexpr const wchar_t* CONST_S_CMD_LINE_FW_UPDATE_SET_DEV_PATH = L"device"; // fw update device path key
        constexpr const wchar_t* CONST_S_CMD_LINE_FW_UPDATE_SET_HIDE_UI = L"quiet"; // fw update hide UI
		constexpr const wchar_t* CONST_S_CMD_LINE_FW_UPDATE_SET_NOTIFY_PROGRESS = L"notify"; // fw update progress notify via ipc pipe


#ifdef _WIN32
        ////////////////// WIN
#ifdef _DEBUG
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
        //constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = _mp::cfile::get_path_ProgramData()+"\\elpusk"
        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\ElpuskHidDaemon";
        constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\coffee_service\\deb_json";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\shared\\projects\\lpu237_ibutton";

#ifdef _M_IX86
        //WIN x86
        constexpr const wchar_t* CONST_S_MGMT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\Debug\\elpusk-hid-d.exe";
        constexpr const wchar_t* CONST_S_DIR_MGMT_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\Debug";
        constexpr const wchar_t* CONST_S_DIR_DLL_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\Debug";
#else
        //WIN x64
        constexpr const wchar_t* CONST_S_MGMT_ABS_FULL_PATH = L"C:\\job\\library.cpp.lpu230\\x64\\Debug\\elpusk-hid-d.exe";
        constexpr const wchar_t* CONST_S_DIR_MGMT_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\x64\\Debug";
        constexpr const wchar_t* CONST_S_DIR_DLL_EXCEPT_BACKSLASH = L"C:\\job\\library.cpp.lpu230\\x64\\Debug";
#endif
#else
		// release win case - defined in mp_coffee_path.h!
        //constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.crt";
        //constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.key";
        //constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = _mp::cfile::get_path_ProgramData()+"\\elpusk"
        //constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\root";
        //constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d";
		//constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\coffee_service";

        //constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\tg_lpu237_dll";
        //constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"%ProgramData%\\elpusk\\00000006\\coffee_manager\\tg_lpu237_ibutton";
        // constexpr const wchar_t* CONST_S_MGMT_ABS_FULL_PATH = L"%ProgramFiles%\\elpusk\\00000006\\coffee_manager\\bin\\elpusk-hid-d.exe";
        //constexpr const wchar_t* CONST_S_DIR_MGMT_EXCEPT_BACKSLASH = L"%ProgramFiles%\\elpusk\\00000006\\coffee_manager\\bin";
        //constexpr const wchar_t* CONST_S_DIR_DLL_EXCEPT_BACKSLASH = L"%ProgramFiles%\\elpusk\\00000006\\coffee_manager\\dll";
#endif //_DEBUG
#else
        ////////////////// LINUX
#ifdef _DEBUG
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
        constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_NOT_ROOT_USER_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";

        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH_LPU230_UPDATE = L"/var/run/lpu230_update.pid";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/ElpuskHidDaemon";
		constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/coffee_service";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/job/library.cpp.lpu230/shared/projects/tg_lpu237_ibutton";

        constexpr const wchar_t* CONST_S_MGMT_ABS_FULL_PATH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/elpusk-hid-d";
        constexpr const wchar_t* CONST_S_DIR_MGMT_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_DIR_DLL_EXCEPT_BACKSLASH = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug";
        constexpr const wchar_t* CONST_S_DIR_DLL_ROM_SO = L"/home/tester/projects/li_rom/bin/x64/Debug/libtg_rom.so";
        constexpr const wchar_t* CONST_S_DIR_DLL_DEV_LIB_SO = L"/home/tester/projects/li_rom/bin/x64/Debug/libdev_lib.so";
        constexpr const wchar_t* CONST_S_DIR_LPU230_UPDATE = L"/home/tester/projects/li_lpu230_update/bin/x64/Debug/lpu230_update.out";
#else
        constexpr const wchar_t* CONST_S_CERT_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.crt";
        constexpr const wchar_t* CONST_S_PRIVATE_KEY_ABS_FULL_PATH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/data/server/coffee_server.key";
        constexpr const wchar_t* CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"/var/log/elpusk";
        constexpr const wchar_t* CONST_S_NOT_ROOT_USER_LOGS_ROOT_DIR_EXCEPT_BACKSLASH = L"~/.elpusk/log";

        constexpr const wchar_t* CONST_S_ROOT_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/root";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH = L"/var/run/elpusk-hid-d.pid";
        constexpr const wchar_t* CONST_S_PID_FILE_FULL_PATH_LPU230_UPDATE = L"/var/run/lpu230_update.pid";
        constexpr const wchar_t* CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/elpusk-hid-d";
		constexpr const wchar_t* CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/coffee_service";

        constexpr const wchar_t* CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_dll";
        constexpr const wchar_t* CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH = L"/usr/share/elpusk/programdata/00000006/coffee_manager/tg_lpu237_ibutton";

        constexpr const wchar_t* CONST_S_MGMT_ABS_FULL_PATH = L"/usr/share/elpusk/program/00000006/coffee_manager/bin/elpusk-hid-d";
        constexpr const wchar_t* CONST_S_DIR_MGMT_EXCEPT_BACKSLASH = L"/usr/share/elpusk/program/00000006/coffee_manager/bin";
        constexpr const wchar_t* CONST_S_DIR_DLL_EXCEPT_BACKSLASH = L"/usr/share/elpusk/program/00000006/coffee_manager/so";
        constexpr const wchar_t* CONST_S_DIR_DLL_ROM_SO = L"/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_rom.so";
        constexpr const wchar_t* CONST_S_DIR_DLL_DEV_LIB_SO = L"/usr/share/elpusk/program/00000006/coffee_manager/so/libdev_lib.so";
#endif //_DEBUG
#endif //_WIN32

    }
}