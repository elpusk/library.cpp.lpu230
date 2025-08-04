#pragma once

#include <memory>
#include <mp_type.h>

class cbootloader_lpu237
{
public:
	typedef std::shared_ptr<cbootloader_lpu237> type_ptr;

public:
	cbootloader_lpu237();

	virtual ~cbootloader_lpu237();

private:
	// m_s_dev_path_lpu237_primitive is 
	std::wstring m_s_lpu237_dev_primitive_path; // the device path of lpu237 primitive device
	std::wstring m_s_lpu237_model_name;
	std::wstring m_s_lpu237_system_version; // the system version of lpu237 device
	unsigned short m_w_lpu237_device_index;
	unsigned long m_n_lpu237_session; // (_MP_TOOLS_INVALID_SESSION_NUMBER)
	unsigned char m_c_lpu237_in_id;
	unsigned char m_c_lpu237_in_out;

	std::wstring m_s_bootloader_dev_path; // the device path of bootloader device
	unsigned char m_c_bootloader_in_id;
	unsigned char m_c_bootloader_in_out;
	std::wstring m_s_bootloader_abs_rom_file;


private://don't call these methods
	cbootloader_lpu237(const cbootloader_lpu237&) = delete; // no copy constructor
	cbootloader_lpu237& operator=(const cbootloader_lpu237&) = delete; // no assignment operator
};