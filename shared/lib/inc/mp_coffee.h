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
    }
}