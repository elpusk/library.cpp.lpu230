#pragma once

// c++ header & all inline
// support only unicode.

#include <vector>
#include <memory>
#include <list>
#include <set>
#include <algorithm>
#include <utility>
#include <string>
#include <deque>
#include <fstream>
#include <map>
#include <thread>
#include <array>

#if defined(_MSC_VER)
#include <atltrace.h>
#define	_ATLTRACE_	ATLTRACE

#if( _MSC_VER == 1600 )
	//vs2010
#define	ct_ns_ex_lib			boost
#define	ct_ns_ex_lib_spirit_char_encoding_standard			boost::spirit::char_encoding::standard
#else
	//vs2019
#define	ct_ns_ex_lib			std
#define	ct_ns_ex_lib_spirit_char_encoding_standard		std
#endif
#endif	//_MSC_VER

namespace _ns_tools
{
	#define _NS_TOOLS_OFFSET_OF_STRUCT(s, m)   (size_t)&(((s *)0)->m)

	//the size of structure member
	//s - structure name, m - member name
	#define	_NS_TOOLS_SIZE_OF_STRUCT(s,m)		sizeof(((s *)0 )->m)


	#define _NS_TOOLS_MAKE_QWORD(hi, lo)    (  (unsigned long long(unsigned long(hi) & 0xffffffff) << 32 ) | unsigned long long(unsigned long(lo) & 0xffffffff)  )

	#define	_NS_TOOLS_CT_TYPE_IC_REQUEST_PREFIX		L"{{{{{"
	#define	_NS_TOOLS_CT_TYPE_IC_REQUEST_POSTFIX	L"}}}}}"
	#define	_NS_TOOLS_CT_TYPE_IC_RESPONSE_PREFIX	L"[[[[["
	#define	_NS_TOOLS_CT_TYPE_IC_RESPONSE_POSTFIX	L"]]]]]"
	#define	_NS_TOOLS_CT_TYPE_MSR_DATA_PREFIX		L"((((("
	#define	_NS_TOOLS_CT_TYPE_MSR_DATA_POSTFIX		L")))))"

	#define	_NS_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND		((unsigned long long)10000000)
	#define	_NS_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE		((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND*60)
	#define	_NS_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR			((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE*60)
	#define	_NS_TOOLS_CT_TYPE_FILE_TIMEIN_DAY			((unsigned long long)_NS_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR*24)

	#define	_NS_TOOLS_INVALID_SESSION_NUMBER		0xFFFFFFFF

	#define _WIDEN(x) L ## x
	#define _WIDEN2(x) _WIDEN(x)
	#define __WFILE__		_WIDEN2(__FILE__)
	#define __WFUNCTION__	_WIDEN2(__FUNCTION__)

	// wpararm == -1 && lparam == -1 -> you must hide dialog
	#define	_NS_TOOLS_WPARAM_FOR_HIDE		-1
	#define	_NS_TOOLS_LPARAM_FOR_HIDE		-1

	typedef	std::pair<bool, bool>				type_pair_bool_result_bool_complete;
	typedef	std::pair<std::wstring, bool>		type_pair_string_bool;

	typedef	std::vector<unsigned char>			type_v_buffer;
	typedef	std::vector<unsigned long>			type_v_ul_buffer;

	typedef	std::vector<char>					type_v_s_buffer;
	typedef	std::vector<wchar_t>				type_v_ws_buffer;
	typedef	std::shared_ptr< type_v_buffer >	type_ptr_v_buffer;

	typedef std::vector<std::string>				type_v_string;
	typedef std::vector<std::wstring>				type_v_wstring;
	typedef	std::vector< type_v_wstring>		type_v_v_wstring;
	typedef std::list<std::wstring>				type_list_wstring;
	typedef std::list<std::string>				type_list_string;
	typedef	std::vector< type_list_wstring>		type_v_list_wstring;

	typedef std::list<type_ptr_v_buffer>		type_list_ptr_v_buffer;
	typedef std::list<type_v_buffer>			type_list_v_buffer;
	typedef	std::pair<_ns_tools::type_ptr_v_buffer, _ns_tools::type_ptr_v_buffer>	type_pair_ptr_v_req_ptr_v_rsp;
	typedef	std::list< _ns_tools::type_pair_ptr_v_req_ptr_v_rsp>	type_list_of_pair_v_req_ptr_v_rsp;

	typedef	std::set<std::wstring>				type_set_wstring;
	typedef	std::set<std::string>				type_set_string;

	typedef	std::deque<std::wstring>			type_deque_wstring;
	typedef	std::deque<std::string>				type_deque_string;

	typedef	std::deque<type_v_buffer>			type_dequeu_v_buffer;
	typedef	std::deque<type_ptr_v_buffer>		type_dequeu_ptr_v_buffer;

	typedef	std::pair<std::wstring, unsigned long>	type_pair_string_color;
	typedef	std::list< type_pair_string_color>		type_list_pair_string_color;

	typedef	std::shared_ptr<std::wfstream>		type_ptr_wfstream;
	typedef	std::shared_ptr<std::fstream>		type_ptr_fstream;
	typedef	std::pair<std::wstring, type_ptr_wfstream>	type_pair_wfilename_ptr_wfstream;
	typedef	std::pair<std::string, type_ptr_fstream>	type_pair_filename_ptr_fstream;
	typedef	std::pair<std::wstring, type_ptr_fstream>	type_pair_wfilename_ptr_fstream;

	typedef	std::set< type_pair_wfilename_ptr_wfstream >	type_set_pair_wfilename_ptr_wfstream;
	typedef	std::set< type_pair_filename_ptr_fstream >		type_set_pair_filename_ptr_fstream;

	typedef	std::map<std::wstring, std::wstring>		type_map_wstring_wstring;
	typedef	std::map<std::wstring, type_ptr_wfstream>	type_map_wfilename_ptr_wfstream;
	typedef	std::map<std::string, type_ptr_fstream>		type_map_filename_ptr_fstream;
	typedef	std::map<std::wstring, type_ptr_fstream>	type_map_wfilename_ptr_fstream;

	typedef	std::shared_ptr<std::thread>	type_ptr_thread;

	typedef	std::array<unsigned char, 20>	type_arrary_digest;


	//under c++98, the embeded enum class is not recognized inside a class, while in c++11 it works.
	// type_found_file_time is used in ct_file
	enum  type_found_file_time
	{
		file_time_create,
		file_time_last_access,
		file_time_last_write
	} ;

	typedef enum : long
	{
		device_io_direct = 1,
		device_io_ng_devmgmt = 2

	}type_device_io_mode;

}
