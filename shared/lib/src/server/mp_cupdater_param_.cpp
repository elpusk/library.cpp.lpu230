#include <websocket/mp_win_nt.h>

#include <string>
#include <regex>

#include <mp_type.h>
#include <mp_coffee.h>
#include <server/mp_cupdater_param_.h>

_mp::cupdater_param::cupdater_param(unsigned long n_session_number) :
	m_n_session_number(n_session_number)
{
}

_mp::cupdater_param::~cupdater_param()
{
}

std::pair<bool, std::wstring> _mp::cupdater_param::find(const std::wstring& s_key, bool b_remove_after_found)
{
	bool b_found(false);
	std::wstring s_value;
	auto it = m_map.find(s_key);
	if (it != std::end(m_map)) {
		//found
		b_found = true;
		s_value = it->second;
		if (b_remove_after_found) {
			m_map.erase(it);
		}
	}

	return std::make_pair(b_found, s_value);
}

bool _mp::cupdater_param::is_valid(const std::wstring& s_key) const
{
	bool b_found(false);
	auto it = m_map.find(s_key);
	if (it != std::end(m_map)) {
		//found
		b_found = true;
	}

	return b_found;
}

bool _mp::cupdater_param::can_be_start_firmware_update() const
{
	bool b_result(false);

	do {
		bool b_found(false);
		std::wstring s_key,s_value;

		// mandatory parameters
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_INDICATOR);
		auto it = m_map.find(s_key);
		if (it == std::end(m_map)) {
			continue;
		}
		if(!(it->second.empty())){
			// value must be empty
			continue;
		}

		s_key = std::wstring(L"model_name");
		it = m_map.find(s_key);
		if (it == std::end(m_map)) {
			continue;
		}
		if(it->second.size() > 16){
			// too long
			continue;
		}
		//
		s_key = std::wstring(L"system_version");
		it = m_map.find(s_key);
		if (it == std::end(m_map)) {
			continue;
		}
		// 정규 표현식: 숫자.숫자.숫자.숫자 (각 숫자는 1~3자리)
		static const std::wregex ipv4Pattern(LR"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)");
		if (!std::regex_match(it->second, ipv4Pattern)) {
			// format error
			continue;
		}

		std::wistringstream ss(it->second);
		std::wstring token;
		int count = 0;

		b_result = true;

		while (std::getline(ss, token, L'.')) {
			try {
				int num = std::stoi(token);
				if (num < 0 || num > 255) return false;
			}
			catch (...) {
				//error
				b_result = false;
				break;// exit while
			}
			++count;
		}

		if(!b_result) {
			continue;
		}
		b_result = false;
		if (count != 4) {
			continue;
		}

		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_FW_FILE);
		it = m_map.find(s_key);
		if (it == std::end(m_map)) {
			continue;
		}
		//
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_DEV_PATH);
		it = m_map.find(s_key);
		if (it == std::end(m_map)) {
			continue;
		}

		//////////////////////////////////////////////
		// optional parameters
		// cf2 에서는 "notify" 이나, cf1 호환을 위해서 "_cf_bl_progress_" 를 받아서.
		// generate_command_line_arguments_by_x() 에서 "notify" 로 변환하여 사용.
		s_key = std::wstring(L"_cf_bl_progress_"); 
		it = m_map.find(s_key);
		if (it != std::end(m_map)) {
			if (it->second.compare(L"true") != 0 && it->second.compare(L"false")) {
				continue;
			}
		}

		// cf2 에서는 "quiet" 이나, cf1 호환을 위해서 "_cf_bl_window_" 를 받아서.
		// generate_command_line_arguments_by_x() 에서 "quiet" 로 변환하여 사용.
		s_key = std::wstring(L"_cf_bl_window_");
		it = m_map.find(s_key);
		if (it != std::end(m_map)) {
			if (it->second.compare(L"true") != 0 && it->second.compare(L"false")) {
				continue;
			}
		}

		b_result = true;
	} while (false);
	return b_result;
}

bool _mp::cupdater_param::insert(const std::wstring& s_key, const std::wstring& s_value)
{
	bool b_result(false);
	std::tie(std::ignore,b_result) = m_map.insert(std::make_pair(s_key, s_value));
	return b_result;
}

bool _mp::cupdater_param::insert_file(const std::wstring& s_value_file_full_abs_path)
{
	return insert(std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_FW_FILE), s_value_file_full_abs_path);
}

bool _mp::cupdater_param::insert_device(const std::wstring& s_value_device_path)
{
	return insert(std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_DEV_PATH), s_value_device_path);
}

bool _mp::cupdater_param::insert_run_by_cf2_mode()
{
	std::wstring s_value(L""); // empty value
	return insert(std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_INDICATOR), s_value);
}

bool _mp::cupdater_param::erase(const std::wstring& s_key)
{
	auto it = m_map.find(s_key);
	if (it != std::end(m_map)) {
		m_map.erase(it);
		return true;
	}
	return false;
}

void _mp::cupdater_param::clear()
{
	m_map.clear();
}

bool _mp::cupdater_param::empty() const
{
	return m_map.empty();
}

size_t _mp::cupdater_param::size() const
{
	return m_map.size();
}

_mp::cupdater_param& _mp::cupdater_param::set_exe_full_abs_path(const std::wstring& s_exe_full_abs_path)
{
	m_s_abs_full_exe_path = s_exe_full_abs_path;
	return *this;
}

std::wstring _mp::cupdater_param::get_exe_full_abs_path_by_wstring() const
{
	return m_s_abs_full_exe_path;
}

std::string _mp::cupdater_param::get_exe_full_abs_path_by_string() const
{
	return _mp::cstring::get_mcsc_from_unicode(m_s_abs_full_exe_path);
}

std::wstring _mp::cupdater_param::generate_command_line_arguments_except_exe_by_wstring() const
{
	std::wstring s_command_line_arguments;

	do {
		if (!can_be_start_firmware_update()) {
			continue;
		}
		//
		std::wstring s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_INDICATOR);

		s_command_line_arguments += L" --";
		s_command_line_arguments += s_key;
		//
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_FW_FILE);
		auto it = m_map.find(s_key);
		s_command_line_arguments += L" --";
		s_command_line_arguments += s_key;
		s_command_line_arguments += L" ";
		s_command_line_arguments += it->second;
		//
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_DEV_PATH);
		it = m_map.find(s_key);
		s_command_line_arguments += L" --";
		s_command_line_arguments += s_key;
		s_command_line_arguments += L" ";
		s_command_line_arguments += it->second;

		// optional parameters
		s_key = std::wstring(L"_cf_bl_progress_");
		it = m_map.find(s_key);
		if( it != std::end(m_map) ) {
			if(it->second.compare(L"true") == 0){
				// notify
				s_command_line_arguments += L" --";
				s_command_line_arguments += std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_NOTIFY_PROGRESS);
			}
		}
		//
		s_key = std::wstring(L"_cf_bl_window_");
		it = m_map.find(s_key);
		if (it != std::end(m_map)) {
			if (it->second.compare(L"false") == 0) {
				// quiet
				s_command_line_arguments += L" --";
				s_command_line_arguments += std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_HIDE_UI);
			}
		}


	} while (false);
	return s_command_line_arguments;
}

std::string _mp::cupdater_param::generate_command_line_arguments_except_exe_by_string() const
{
	return _mp::cstring::get_mcsc_from_unicode(generate_command_line_arguments_except_exe_by_wstring());
}

std::wstring _mp::cupdater_param::generate_command_line_arguments_with_exe_by_wstring() const
{
	return( m_s_abs_full_exe_path + generate_command_line_arguments_except_exe_by_wstring() );
}

std::string _mp::cupdater_param::generate_command_line_arguments_with_exe_by_string() const
{
	std::string s = _mp::cstring::get_mcsc_from_unicode(m_s_abs_full_exe_path);
	return(s + generate_command_line_arguments_except_exe_by_string());
}

std::vector<std::wstring> _mp::cupdater_param::generate_command_line_arguments_except_exe_by_vector_wstring() const
{
	std::vector<std::wstring> v;

	do {
		if (!can_be_start_firmware_update()) {
			continue;
		}
		std::wstring s_key;
		std::wstring s_p;

		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_INDICATOR);
		s_p = L"--";
		s_p += s_key;
		v.push_back(s_p);
		//
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_FW_FILE);
		auto it = m_map.find(s_key);
		s_p = L"--";
		s_p += s_key;
		v.push_back(s_p);
		v.push_back(it->second);
		//
		s_key = std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_DEV_PATH);
		it = m_map.find(s_key);
		s_p = L"--";
		s_p += s_key;
		v.push_back(s_p);
		v.push_back(it->second);
		//
		s_key = std::wstring(L"_cf_bl_progress_");
		it = m_map.find(s_key);
		if (it != std::end(m_map)) {
			if (it->second.compare(L"true") == 0) {
				s_p = L"--";
				s_p += std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_NOTIFY_PROGRESS);
				v.push_back(s_p);
			}
		}
		//
		s_key = std::wstring(L"_cf_bl_window_");
		it = m_map.find(s_key);
		if (it != std::end(m_map)) {
			if (it->second.compare(L"false") == 0) {
				s_p = L"--";
				s_p += std::wstring(_mp::_coffee::CONST_S_CMD_LINE_FW_UPDATE_SET_HIDE_UI);
				v.push_back(s_p);
			}
		}

	} while (false);
	return v;
}

std::vector<std::string> _mp::cupdater_param::generate_command_line_arguments_except_exe_by_vector_string() const
{
	std::vector<std::string> v;

	auto wv = generate_command_line_arguments_except_exe_by_vector_wstring();

	for (auto ws : wv) {
		v.push_back(_mp::cstring::get_mcsc_from_unicode(ws));
	}//end for
	return v;
}

std::vector<std::wstring> _mp::cupdater_param::generate_command_line_arguments_with_exe_by_vector_wstring() const
{
	auto wv = generate_command_line_arguments_except_exe_by_vector_wstring();
	wv.insert(wv.begin(), m_s_abs_full_exe_path);
	return wv;
}

std::vector<std::string> _mp::cupdater_param::generate_command_line_arguments_with_exe_by_vector_string() const
{
	std::vector<std::string> v;

	auto wv = generate_command_line_arguments_with_exe_by_vector_wstring();

	for (auto ws : wv) {
		v.push_back(_mp::cstring::get_mcsc_from_unicode(ws));
	}//end for
	return v;
}
