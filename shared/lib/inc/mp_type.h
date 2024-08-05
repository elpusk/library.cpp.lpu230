#pragma once

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
/*
#ifdef _WIN32
#define _WIDEN(x) L ## x
#define _WIDEN2(x) _WIDEN(x)
#define __WFILE__		_WIDEN2(__FILE__)
#define __WFUNCTION__	_WIDEN2(__FUNCTION__)
#else
#define __WFILE__		std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(__FILE__)
#define __WFUNCTION__	std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(__FUNCTION__)
#endif
*/

#define __WFILE__		(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(__FILE__).c_str())
#define __WFUNCTION__	(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(__FUNCTION__).c_str())


#define	_MP_TOOLS_CT_TYPE_IC_REQUEST_PREFIX		L"{{{{{"
#define	_MP_TOOLS_CT_TYPE_IC_REQUEST_POSTFIX	L"}}}}}"
#define	_MP_TOOLS_CT_TYPE_IC_RESPONSE_PREFIX	L"[[[[["
#define	_MP_TOOLS_CT_TYPE_IC_RESPONSE_POSTFIX	L"]]]]]"
#define	_MP_TOOLS_CT_TYPE_MSR_DATA_PREFIX		L"((((("
#define	_MP_TOOLS_CT_TYPE_MSR_DATA_POSTFIX		L")))))"

#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND		((unsigned long long)10000000)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE		((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND*60)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR			((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE*60)
#define	_MP_TOOLS_CT_TYPE_FILE_TIMEIN_DAY			((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR*24)

#define	_MP_TOOLS_INVALID_SESSION_NUMBER		0xFFFFFFFF
#define	_MP_TOOLS_INVALID_DEVICE_INDEX		0x0000

#define _MP_TOOLS_OFFSET_OF_STRUCT(s, m)   (size_t)&(((s *)0)->m)

    //the size of structure member
    //s - structure name, m - member name
#define	_MP_TOOLS_SIZE_OF_STRUCT(s,m)		sizeof(((s *)0 )->m)

#define _MP_TOOLS_MAKE_QWORD(hi, lo)    (  (unsigned long long(unsigned long(hi) & 0xffffffff) << 32 ) | unsigned long long(unsigned long(lo) & 0xffffffff)  )

    typedef std::vector<unsigned char> type_v_buffer;
    typedef std::shared_ptr< std::vector<unsigned char> > type_ptr_v_buffer;
    typedef	std::vector<unsigned long>			type_v_ul_buffer;

    typedef std::vector<char> type_v_s_buffer;
    typedef std::vector<wchar_t> type_v_ws_buffer;

    typedef std::set<std::wstring> type_set_wstring;
    typedef std::set<std::string> type_set_string;

    typedef	std::tuple<int, int, int> type_tuple_usb_filter;//get<0> vid, get<1> pid, get<2> inf
    typedef	std::set<type_tuple_usb_filter> type_set_usb_filter;

    typedef std::list<std::wstring> type_list_wstring;

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

    typedef enum : unsigned long {
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

}