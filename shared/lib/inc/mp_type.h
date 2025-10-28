#pragma once

/**
* Dont' include user defined header.
* this header must be rooer of all user header files.
*/

#include <stdint.h>
#include <vector>
#include <string>
#include <deque>
#include <set>
#include <memory>
#include <list>
#include <map>
#include <locale>
#include <codecvt>

namespace _mp{

#ifdef _WIN32
    // windows

#define _WIDEN(x) L ## x
#define _WIDEN2(x) _WIDEN(x)
#define __WFILE__		_WIDEN2(__FILE__)
#define __WFUNCTION__	_WIDEN2(__FUNCTION__)

#else
	// linux

    inline std::wstring _mcsc_to_unicode(const std::string& s_mcsc)
    {
        std::wstring s_unicode;
        std::vector<wchar_t> v_unicode;
        do {
            if (s_mcsc.empty()) {
                continue;
            }
            size_t size_needed = std::mbstowcs(nullptr, s_mcsc.c_str(), 0);
            if (size_needed != (size_t)-1) {
                v_unicode.resize(size_needed+1,0);
                std::mbstowcs(&v_unicode[0], s_mcsc.c_str(), size_needed);
                s_unicode = &v_unicode[0];
            }
        } while (false);
        return s_unicode;
    }

#define __WFILE__   _mp::_mcsc_to_unicode(__FILE__).c_str()
#define __WFUNCTION__   _mp::_mcsc_to_unicode(__FUNCTION__).c_str()
#endif


#define	_MP_TOOLS_CT_TYPE_IC_REQUEST_PREFIX		L"{{{{{"
#define	_MP_TOOLS_CT_TYPE_IC_REQUEST_POSTFIX	L"}}}}}"
#define	_MP_TOOLS_CT_TYPE_IC_RESPONSE_PREFIX	L"[[[[["
#define	_MP_TOOLS_CT_TYPE_IC_RESPONSE_POSTFIX	L"]]]]]"
#define	_MP_TOOLS_CT_TYPE_MSR_DATA_PREFIX		L"((((("
#define	_MP_TOOLS_CT_TYPE_MSR_DATA_POSTFIX		L")))))"

#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND		((uint64_t)10000000)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE		((uint64_t)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND*60)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR			((uint64_t)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE*60)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_DAY			((uint64_t)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR*24)

#define	_MP_TOOLS_INVALID_SESSION_NUMBER		0xFFFFFFFF
#define	_MP_TOOLS_INVALID_DEVICE_INDEX		0x0000

#define _MP_TOOLS_OFFSET_OF_STRUCT(s, m)   (size_t)&(((s *)0)->m)

    //the size of structure member
    //s - structure name, m - member name
#define	_MP_TOOLS_SIZE_OF_STRUCT(s,m)		sizeof(((s *)0 )->m)

#define _MP_TOOLS_MAKE_QWORD(hi, lo)    (  (uint64_t(uint32_t(hi) & 0xffffffff) << 32 ) | uint64_t(uint32_t(lo) & 0xffffffff)  )

#ifdef _WIN32
    #define _MP_TIMEOUT     INFINITE
    #define _PACK_BYTE
#else
    #define    _MP_TIMEOUT     -1
    #define _PACK_BYTE  __attribute__((packed))
    #define __stdcall   __attribute__((__stdcall__))

    #define PtrToUlong(_a)      ((uint32_t)(_a))
    #define INVALID_HANDLE_VALUE    ((uint32_t)-1)

    #define FALSE   0
    #define TRUE    1

    typedef uint32_t HWND;
    typedef uint32_t UINT;
    typedef uint32_t HANDLE;
    typedef uint32_t BOOL;

    typedef	void* HMODULE;

    #define WPARAM unsigned long long
    #define LPARAM long long

#endif // !_WIN32

	//server timeout constants
#define CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_WEBSOCKET_UPGRADE_REQ_OF_CLIENT_MSEC	7000
#define CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_IDLE_MSEC  -1 // -1 means no timeout for idle
#define CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC 5000

	//client timeout constants
#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC	5000
#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WSS_MSEC 7000
#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WSS_MSEC -1 // -1 means no timeout for idle

#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WS_MSEC 30000
#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WS_MSEC -1 // -1 means no timeout for idle

#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WSS_MSEC 500
#define CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WS_MSEC 500

#define CONST_DEFAULT_WSS_CONNECT_TIMEOUT_IN_API_MSEC 1000

//
#define CONST_MAX_OPEN_COUNT_ON_SHARED_MODE     20  //// max open counter value in shared mode.

    typedef std::vector<unsigned char> type_v_buffer;
    typedef std::shared_ptr< std::vector<unsigned char> > type_ptr_v_buffer;
    typedef	std::vector<unsigned long>			type_v_ul_buffer;

    typedef std::vector<char> type_v_s_buffer;
    typedef std::vector<wchar_t> type_v_ws_buffer;

    typedef std::set<std::wstring> type_set_wstring;
    typedef std::set<std::string> type_set_string;

    typedef	std::tuple<int, int, int> type_tuple_usb_filter;//get<0> vid, get<1> pid, get<2> inf
    typedef	std::set<type_tuple_usb_filter> type_set_usb_filter;

    typedef	std::pair<int, int> type_pair_usb_id;//first vid, second pid
    typedef	std::set<type_pair_usb_id> type_set_usb_id;

    typedef std::list<std::wstring> type_list_wstring;
    typedef std::list<std::string> type_list_string;

    typedef	std::shared_ptr<std::wfstream>		type_ptr_wfstream;
    typedef	std::shared_ptr<std::fstream>		type_ptr_fstream;
    typedef	std::pair<std::wstring, type_ptr_wfstream>	type_pair_wfilename_ptr_wfstream;
    typedef	std::pair<std::string, type_ptr_fstream>	type_pair_filename_ptr_fstream;
    typedef	std::pair<std::wstring, type_ptr_fstream>	type_pair_wfilename_ptr_fstream;

    typedef	std::pair<bool, bool>				type_pair_bool_result_bool_complete;
    typedef	std::pair<std::wstring, bool>		type_pair_string_bool;

    typedef	std::set< type_pair_wfilename_ptr_wfstream >	type_set_pair_wfilename_ptr_wfstream;
    typedef	std::set< type_pair_filename_ptr_fstream >		type_set_pair_filename_ptr_fstream;

    typedef	std::map<std::wstring, std::wstring>		type_map_wstring_wstring;
    typedef	std::map<std::wstring, type_ptr_wfstream>	type_map_wfilename_ptr_wfstream;
    typedef	std::map<std::string, type_ptr_fstream>		type_map_filename_ptr_fstream;
    typedef	std::map<std::wstring, type_ptr_fstream>	type_map_wfilename_ptr_fstream;

    typedef	std::deque<std::wstring>			type_deque_wstring;
    typedef	std::deque<std::string>				type_deque_string;

    typedef	std::deque<type_v_buffer>			type_dequeu_v_buffer;
    typedef	std::deque<type_ptr_v_buffer>		type_dequeu_ptr_v_buffer;

    /**
	* @description: the type of device
	* size of type_bm_dev is 4bytes.
    * primitive type mask pattern is 0x00ff0000.
	* leaf device(sub-device or compositive device) mask pattern is 0x000000ff.
	* 0x0000ff00 mask pattern will be used for identifying the connected pysical device.
    */
    typedef enum : uint32_t {
        type_bm_dev_unknown = 0x00000000//primitive type
        , type_bm_dev_winusb = 0x00010000//primitive type
        , type_bm_dev_hid = 0x00020000//primitive type
        , type_bm_dev_virtual = 0x00800000//primitive type

        , type_bm_dev_lpu200_msr = 0x00020001//low 2bytes is leaf device type. the sub device of hid.
        , type_bm_dev_lpu200_scr0 = 0x00020010//low 2bytes is leaf device type. the sub device of hid.
        , type_bm_dev_lpu200_ibutton = 0x00020020//low 2bytes is leaf device type. the sub device of hid.
        , type_bm_dev_lpu200_switch0 = 0x00020040//low 2bytes is leaf device type. the sub device of hid.
    }type_bm_dev;

    typedef	std::vector< type_bm_dev > type_v_bm_dev;

    // For construting one transaction, the next request type.
    typedef	enum : int {
        // the current transaction is terminated in the current phase(request).
        next_io_none = -1,

        // the current transaction isn't terminated in the current phase(request).
        // the next phase is writing request.
        next_io_write = -2,

        // the current transaction isn't terminated in the current phase(request).
        // the next phase is reaing request.
        next_io_read = -3
    }type_next_io;

	// exit code type for main() function.
    typedef enum : int {
        exit_error_not_supported = 200,
        exit_error_daemonize = 250,
        exit_error_already_running = 251,
        exit_error_create_ctl_pipe = 252,
        exit_error_start_server = 253,
        exit_error_get_ctl_pipe = 254,
        exit_error_create_install_cert = 255,
        exit_error_remove_cert = 256,
        exit_error_load_dev_lib = 257,
        exit_error_load_rom_lib = 258,
        exit_error_create_dev_mgmt = 259,

        exit_error_run_by_cf_rom_file = 260,
        exit_error_run_by_cf_device = 261,

        exit_info_ctl_pipe_requst_terminate = 300
    }type_exit_code;

}