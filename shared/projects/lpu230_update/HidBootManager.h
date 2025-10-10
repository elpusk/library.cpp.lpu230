#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <tuple>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <filesystem>
#include <atomic>
#include <queue>
#include <cstdint>

#include <mp_type.h>
#include <mp_cwait.h>
#include <mp_cfile.h>
#include <hid/mp_clibhid.h>

#include "HidBootBuffer.h"

// DONT INCLUDE cshare.h

class CHidBootManager
{
public:
	typedef std::pair< std::string, _mp::clibhid::pair_ptrs >	type_pair_handle;
	// callback function type
	// int : user defined message type
	// WPARAM : WPARAM of user parameter
	// LPARAM : LPARAM of user parameter
	typedef std::function<void(int, WPARAM , LPARAM )> type_cb;

	// first : callback function
	// second : user defined message type
	typedef std::pair<CHidBootManager::type_cb, int> type_pair_cb;
	typedef std::vector< type_pair_cb > type_v_pair_cb;

	enum {
		C_DO_OK = 0,
		C_DO_ERROR = 1,
		C_DO_ING = 2
	};

	enum {//WPARAM of message
		C_WP_ERASE = 0,
		C_WP_READFILE = 1,
		C_WP_SENDDATA = 2,
		C_WP_COMPLETE = 100,
		C_WP_SET_PROGRESS_RANGE = 200
	};

	enum {
		C_LP_ERROR = 0,
		C_LPSUCCESS = 1
	};

	enum WorkStep
	{
		WSTEP_INI = 0,
		WSTEP_DEV_OUT = 1,
		WSTEP_DRV_IN = 2,
		WSTEP_DELETE = 3,
		WSTEP_COPY = 4,
		WSTEP_DRV_OUT = 5,
		WSTEP_DEV_IN = 6
	};

private:
	// second - message type
	typedef std::tuple< CHidBootManager::type_cb, int, WPARAM, LPARAM > _type_tupel_cb;
	typedef std::queue< CHidBootManager::_type_tupel_cb > _type_q_tupel_cb;

	enum {
		C_KILL_WAIT_TIME = 5000,
		C_WAIT_IDLE = 30
	};

	enum Do_Status
	{
		DS_IDLE,
		DS_READFILE,
		DS_SENDDATA,
		DS_ERASE
	};

public:
	virtual ~CHidBootManager(void);

	static CHidBootManager* GetInstance();
	//
	bool load_rom_library(const std::filesystem::path & path_abs_full_rom_dll);

	std::shared_ptr<CRom> get_rom_library() { return m_ptr_rom; }

	/**
	* @brief add notify callback function.
	* @param cb : callback function.
	* @param uNotifyMsg : user defined message type.
	*/
	size_t add_notify_cb(CHidBootManager::type_cb cb, int uNotifyMsg);

	//
	bool start_update(int nFirmwareIndex, const std::wstring& sFirmware, CHidBootManager::type_cb cb, int uNotifyMsg);
	bool Stop();
	bool Pause();
	bool Resume();

	bool GotoApp();

	int GetDeviceList();
	bool SelectDevice(const std::string& s_device_path, size_t n_fw_size);

	bool UnselectDevice();

	bool IsInitialOk() { return m_bIniOk; }

	uint32_t get_app_area_size() const
	{
		return m_RBuffer.get_app_area_size();
	}

	int get_the_number_of_app_sector() const
	{
		return m_RBuffer.get_the_number_of_app_sector();
	}

	static unsigned int get_firmware_size(const std::wstring& sRomfileName, int nIndex)
	{
		unsigned int n_size = 0;

		if (sRomfileName.empty())
			return n_size;
		if (nIndex < 0) {
			return (unsigned int)std::filesystem::file_size(std::filesystem::path(sRomfileName));
		}
		//
		CRom rom(sRomfileName.c_str());
		CRom::ROMFILE_HEAD header;

			::memset(&header, 0, sizeof header);
		if (CRom::result_success != rom.LoadHeader(sRomfileName.c_str(), &header)) {
				//fail load Header of rom
				return n_size;
		}

		if (header.dwItem <= (uint32_t)nIndex)
			return n_size;
		//
		n_size = header.Item[nIndex].dwSize;

		return n_size;
	}

public:
	bool do_erase_in_worker(int n_sec);
	bool do_write_sector(
		int n_sec
		,const std::vector<unsigned char>& v_sector
	);

private:
	CHidBootManager(void);
	//
	bool _create_cb_worker();
	bool _kill_cb_worker();
	void _woker_for_cb();
	//
	void _push_cb(CHidBootManager::type_cb cb, int n_msg, WPARAM wparam, LPARAM lparam);
	CHidBootManager::_type_tupel_cb pop_cb();

	void PostAnnounce(WPARAM wParam, LPARAM lParam = 0);

	void _post_progress_range();
	bool _is_zero_packet(std::vector<unsigned char>& vPacket);
	

	//
	bool _create_worker();
	bool _kill_worker();

	bool _doing_in_worker();
	bool _do_send_data_in_worker(bool b_resend_mode);

	void _reset_file_read_pos()
	{
		if (m_nRomItemIndex > -1)
			m_nCurRomReadOffset = 0;
		else
			m_Firmware.seekg(0);
	}
	int _get_firmware_size_at_file()
	{
		int n_fs(0);
		if (m_nRomItemIndex > -1)
			n_fs = m_Header.Item[m_nRomItemIndex].dwSize;
		else {
			m_Firmware.seekg(0, std::ios::end);
			n_fs = m_Firmware.tellg();//distance( istreambuf_iterator<char>(m_Firmware), istreambuf_iterator<char>() );
		}
		return n_fs;
	}

	void _load_one_sector_from_file()
	{
		if (m_nRomItemIndex > -1) {
			m_nCurRomReadOffset = m_RBuffer.load_to_buffer_from_file_current_pos(*m_ptr_rom, m_Header, m_nRomItemIndex, m_nCurRomReadOffset);
		}
		else {
			m_RBuffer.load_to_buffer_from_file_current_pos(m_Firmware);
		}
	}

private:
	static int _DDL_GetList(std::list<std::wstring>& ListDev, int nVid, int nPid, int nInf);
	static CHidBootManager::type_pair_handle _DDL_open(const std::wstring & szDevicePath);
	static bool _DDL_close(CHidBootManager::type_pair_handle hDevice);
	static int _DDL_write(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nTx, int nOutReportSize);
	static int _DDL_read(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nRx, int nInReportSize);
	static bool _DDL_TxRx(CHidBootManager::type_pair_handle hDev, unsigned char* lpTxData, int nTx, int nOutReportSize, unsigned char* lpRxData, int* nRx, int nInReportSize);


	/**
	* return first - processing result
	* second  true - exist sector info
	* third the sector number of starting app area.
	* forth the number of sector of app area.
	*/
	std::tuple<bool, bool, uint32_t, uint32_t> _get_sector_info_from_device();

	void _woker_for_update();

private:
	//
	std::mutex m_mutex_main;

	CHidBootManager::type_v_pair_cb m_v_pair_cb;

	std::shared_ptr<std::thread> m_ptr_worker;

	_mp::cwait m_waiter;

	int m_n_evt_Kill;
	int m_n_evt_Do;
	int m_n_evt_Stop;
	int m_n_evt_Pause;
	int m_n_evt_Resume;

	bool m_bIniOk;

	CRom::ROMFILE_HEAD m_Header;
	std::shared_ptr<CRom> m_ptr_rom;
	int m_nRomItemIndex;
	unsigned int m_nCurRomReadOffset;

	std::ifstream m_Firmware;
	std::wstring m_sFirmware;

	CHidBootBuffer m_RBuffer;

	Do_Status m_DoStatus;

	std::list<std::wstring> m_listDev;

	CHidBootManager::type_pair_handle m_pair_dev_ptrs;// <- HANDLE m_hDev;

	std::mutex m_mutex_cb;
	_mp::cwait m_waiter_cb;
	CHidBootManager::_type_q_tupel_cb m_q_cb; //protected by m_mutex_cb
	int m_n_evt_cb_push;
	int m_n_evt_cb_kill_woker;
	std::shared_ptr<std::thread> m_ptr_worker_cb;

private://don;t call these methods.
	CHidBootManager(const CHidBootManager&);	//disable copy constructure.
	CHidBootManager& operator=(const CHidBootManager&);
};

