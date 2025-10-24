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
	bool GotoApp();

	int GetDeviceList();
	bool SelectDevice(const std::string& s_device_path, size_t n_fw_size);

	bool UnselectDevice();

	bool IsInitialOk() { return m_bIniOk; }

public:
	/**
	* @return first - true : success processing
	*
	*	second - error information.
	*/
	std::pair<bool, std::string> do_erase_in_worker(int n_sec);

	/**
	* @return first - true : success processing
	* 
	*	second - error information.
	*/
	std::pair<bool,std::string> do_write_sector(
		int n_sec
		,const std::vector<unsigned char>& v_sector
		, std::ofstream& opened_debug_file //for debugging
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

	bool _is_zero_packet(std::vector<unsigned char>& vPacket);

	//
private:
	static int _DDL_GetList(std::list<std::wstring>& ListDev, int nVid, int nPid, int nInf);
	static CHidBootManager::type_pair_handle _DDL_open(const std::wstring & szDevicePath);
	static bool _DDL_close(CHidBootManager::type_pair_handle hDevice);
	static int _DDL_write(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nTx, int nOutReportSize);
	static int _DDL_read(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nRx, int nInReportSize);
	static bool _DDL_TxRx(CHidBootManager::type_pair_handle hDev, unsigned char* lpTxData, int nTx, int nOutReportSize, unsigned char* lpRxData, int* nRx, int nInReportSize);


	/**
	* @return 
	* first - processing result
	* 
	* second - true : exist sector info
	* 
	* third - the sector number of starting app area.(0 based sector number)
	* 
	* forth - the number of sector of app area.( the number of total sector - the number of bootloader area sector )
	*/
	std::tuple<bool, bool, uint32_t, uint32_t> _get_sector_info_from_device();

private:
	//
	std::mutex m_mutex_main;

	CHidBootManager::type_v_pair_cb m_v_pair_cb;

	bool m_bIniOk;

	std::shared_ptr<CRom> m_ptr_rom;

	std::list<std::wstring> m_listDev;

	CHidBootManager::type_pair_handle m_pair_dev_ptrs;// <- HANDLE m_hDev;

	std::mutex m_mutex_cb;
	_mp::cwait m_waiter_cb;
	CHidBootManager::_type_q_tupel_cb m_q_cb; //protected by m_mutex_cb
	int m_n_evt_cb_push;
	int m_n_evt_cb_kill_woker;
	std::shared_ptr<std::thread> m_ptr_worker_cb;

	bool m_b_exist_sec_info;

private://don;t call these methods.
	CHidBootManager(const CHidBootManager&);	//disable copy constructure.
	CHidBootManager& operator=(const CHidBootManager&);
};

