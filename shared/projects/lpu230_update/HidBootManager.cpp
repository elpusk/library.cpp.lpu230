#include "HidBootManager.h"
#include <memory>
#include <algorithm>


#ifdef	WIN32
#include <atldef.h>
#include <atltrace.h>
#endif

#include <mp_csystem_.h>

CHidBootManager::CHidBootManager(void) :
	m_bIniOk(true),
	m_n_evt_Kill(-1), m_n_evt_Do(-1), m_n_evt_Stop(-1), m_n_evt_Pause(-1), m_n_evt_Resume(-1),
	m_DoStatus(DS_IDLE),
	m_hDev(NULL),
	m_nRomItemIndex(-1),
	m_nCurRomReadOffset(0)
{
	::memset( &m_Header, 0, sizeof m_Header );
	//
	m_n_evt_Kill = m_waiter.generate_new_event();
	m_n_evt_Do = m_waiter.generate_new_event();
	m_n_evt_Stop = m_waiter.generate_new_event();
	m_n_evt_Pause = m_waiter.generate_new_event();
	m_n_evt_Resume = m_waiter.generate_new_event();

	_create_cb_worker();
	//
	m_n_evt_cb_kill_woker = m_waiter_cb.generate_new_event();
	m_n_evt_cb_push = m_waiter_cb.generate_new_event();
	_create_worker();
}

bool CHidBootManager::_create_cb_worker()
{
	bool bResult = true;

	if (m_ptr_worker_cb)
		return bResult;

	m_ptr_worker_cb = std::make_shared<std::thread>(&CHidBootManager::_woker_for_cb, this);
	if (!m_ptr_worker_cb)
		bResult = false;
	//

	return bResult;
}

bool CHidBootManager::_kill_cb_worker()
{
	bool bResult = true;

	if (m_ptr_worker_cb) {

		m_waiter_cb.set(m_n_evt_cb_kill_woker);

		if (m_ptr_worker_cb->joinable()) {
			m_ptr_worker_cb->join();
		}
	}

	return bResult;
}

void CHidBootManager::_woker_for_cb()
{
	bool b_run(true);
	do {

		int n_result = m_waiter_cb.wait_for_one_at_time();
		if (n_result == m_n_evt_cb_kill_woker) {
			b_run = false;// the end of worker.
			continue;
		}
		if (n_result != m_n_evt_cb_push) {
			m_waiter_cb.reset(n_result);
			CHidBootManager::_type_tupel_cb tupel_cb = pop_cb();

			//run callback
			if (std::get<0>(tupel_cb) == nullptr) {
				continue;
			}

			std::get<0>(tupel_cb)(std::get<1>(tupel_cb), std::get<2>(tupel_cb), std::get<3>(tupel_cb) );

			continue;
		}

	} while (b_run);

#ifdef _WIN32
	ATLTRACE(L"Exit CHidBootManager::_woker_for_cb().");
#endif

}

void CHidBootManager::_push_cb(CHidBootManager::type_cb cb, int n_msg, WPARAM wparam, LPARAM lparam)
{
	std::lock_guard<std::mutex> lock(m_mutex_cb);
	m_q_cb.push(std::make_tuple(cb,n_msg,wparam,lparam));
}

CHidBootManager::_type_tupel_cb CHidBootManager::pop_cb()
{
	CHidBootManager::_type_tupel_cb tuple_cb(nullptr, -1, 0, 0);
	do {
		std::lock_guard<std::mutex> lock(m_mutex_cb);
		if (m_q_cb.empty()) {
			continue;
		}
		tuple_cb = m_q_cb.front();
		m_q_cb.pop();

	} while (false);
	return tuple_cb;
}

CHidBootManager::~CHidBootManager(void)
{
	_kill_worker();
	_kill_cb_worker();
}

CHidBootManager *CHidBootManager::GetInstance()
{
	static CHidBootManager BootMgmt;

	if( BootMgmt.IsInitialOk() )
		return &BootMgmt;
	else
		return NULL;
}

bool CHidBootManager::load_rom_library(const std::filesystem::path& path_abs_full_rom_dll)
{
	bool b_result(false);
	do {
		if (!std::filesystem::exists(path_abs_full_rom_dll)) {
			continue;
		}

		if (m_ptr_rom) {
			m_ptr_rom.reset();
		}

		m_ptr_rom = std::make_shared<CRom>(path_abs_full_rom_dll.wstring().c_str());
		if(!m_ptr_rom){
			continue;
		}
		if (!m_ptr_rom->is_ini()) {
			m_ptr_rom.reset();
			continue;
		}

		b_result = true;

	} while (false);
	return b_result;
}

size_t CHidBootManager::add_notify_cb(CHidBootManager::type_cb cb, int uNotifyMsg)
{
	if (!m_v_pair_cb.empty()) {
		m_v_pair_cb.clear();
	}
	m_v_pair_cb.push_back(std::make_pair(cb, uNotifyMsg));
	return m_v_pair_cb.size()-1;
}


bool CHidBootManager::start_update( int nFirmwareIndex, const std::wstring & sFirmware, CHidBootManager::type_cb cb, int uNotifyMsg)
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( sFirmware.empty() || !m_ptr_rom)
		bResult = false;
	else{
		//setting parameters
		m_sFirmware = sFirmware;

		add_notify_cb(cb, uNotifyMsg);
		m_nRomItemIndex = nFirmwareIndex;

		if( m_nRomItemIndex > -1 ){
			if(m_ptr_rom->LoadHeader( sFirmware.c_str(), &m_Header ) != CRom::result_success ){
				m_sFirmware = L"";
				bResult = false;
			}
			else{
				if( m_ptr_worker ){
					m_nCurRomReadOffset = CHidBootBuffer::C_SECTOR_SIZE;
					m_DoStatus = DS_READFILE;
					m_waiter.set(m_n_evt_Do);
				}
				else
					bResult = false;
			}
		}
		else{
			if( m_Firmware.is_open() )
				m_Firmware.close();
			//
			std::string s_firm(_mp::cstring::get_mcsc_from_unicode(sFirmware));
			m_Firmware.open(s_firm, std::ios::binary );
			if( !(m_Firmware) ){
				m_sFirmware = L"";
				bResult = false;
			}
			else{
				if( m_ptr_worker ){

					m_Firmware.seekg(CHidBootBuffer::C_SECTOR_SIZE );
					m_DoStatus = DS_READFILE;
					m_waiter.set(m_n_evt_Do);
				}
				else
					bResult = false;
			}
		}
	}

	return bResult;
}

bool CHidBootManager::Stop()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( m_ptr_worker ){
		m_waiter.set(m_n_evt_Stop );
	}
	else
		bResult = false;

	return bResult;
}

bool CHidBootManager::Pause()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( m_ptr_worker ){
		m_waiter.set(m_n_evt_Pause );
	}
	else
		bResult = false;

	return bResult;
}

bool CHidBootManager::Resume()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if(m_ptr_worker){
		m_waiter.set(m_n_evt_Resume );
	}
	else
		bResult = false;

	return bResult;
}

bool CHidBootManager::_create_worker()
{
	bool bResult = true;

	if(m_ptr_worker)
		return bResult;

	m_ptr_worker = std::make_shared<std::thread>(&CHidBootManager::_woker_for_update, this);
	if( !m_ptr_worker)
		bResult = false;
	//

	return bResult;
}

bool CHidBootManager::_kill_worker()
{
	bool bResult = true;

	if(m_ptr_worker){

		m_waiter.set(m_n_evt_Kill );

		if (m_ptr_worker->joinable()) {
			m_ptr_worker->join();
		}
	}

	return bResult;
}

//////////////////////////////////////////////
//repeatdly called by worker.......
bool CHidBootManager::_doing_in_worker()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	do {
		if (m_DoStatus == DS_IDLE)
			continue;
		//
		if (m_sFirmware.empty()) {
			bResult = false;
			continue;
		}
		//
		if (DS_ERASE == m_DoStatus) {
			// erase the first sector.
			bResult = _do_erase_in_worker(m_RBuffer.get_erasing_sector_number());
			if (!bResult) {
				PostAnnounce(C_WP_ERASE, C_LP_ERROR);
				m_DoStatus = DS_IDLE;
				continue;
			}
			//
			if (m_RBuffer.is_complete_erase()) {
				PostAnnounce(C_WP_ERASE, C_LPSUCCESS);
				m_DoStatus = DS_SENDDATA;//next step
				continue;
			}
			//
			PostAnnounce(C_WP_ERASE, C_LPSUCCESS+ m_RBuffer.increase_erasing_sector_number());
			continue;
		}

		if (DS_READFILE == m_DoStatus) {
			//all read file data
			int n_fs(_get_firmware_size_at_file());
			if (!m_RBuffer.set_file_size_and_adjust_erase_write_area(n_fs)) {
				PostAnnounce(C_WP_READFILE, C_LP_ERROR);
				m_DoStatus = DS_IDLE;
			}
			else {
				_reset_file_read_pos();
				_load_one_sector_from_file();
				//Deb_Printf(L".. Filled partial buffer.\n");
				_post_progress_range();//set upate-range to file sizeof app.

				PostAnnounce(C_WP_READFILE, C_LPSUCCESS);
				m_DoStatus = DS_ERASE;//next step
			}
			continue;
		}
		//
		if (DS_SENDDATA == m_DoStatus) {
			//send data

			if (_do_send_data_in_worker(false)) {

				if (!m_RBuffer.is_complete_send()) {
					PostAnnounce(C_WP_SENDDATA, C_LPSUCCESS + m_RBuffer.get_sending_sector_number());
				}
				else {
					m_DoStatus = DS_IDLE;
					PostAnnounce(C_WP_COMPLETE, C_LPSUCCESS + m_RBuffer.get_sending_sector_number());
				}
			}
			else {//error

				//retry
				if (_do_send_data_in_worker(true)) {
					if (!m_RBuffer.is_complete_send()) {
						PostAnnounce(C_WP_SENDDATA, C_LPSUCCESS + m_RBuffer.get_sending_sector_number());
					}
					else {
						m_DoStatus = DS_IDLE;
						PostAnnounce(C_WP_COMPLETE, C_LPSUCCESS + m_RBuffer.get_sending_sector_number());
					}
				}
				else {
					PostAnnounce(C_WP_SENDDATA, C_LP_ERROR);
					m_DoStatus = DS_IDLE;//next step
				}
			}
		}
	} while (false);

	return bResult;
}

void CHidBootManager::PostAnnounce( WPARAM wParam, LPARAM lParam /*=0*/ )
{
	if (!m_v_pair_cb.empty()) {
		_push_cb(m_v_pair_cb[0].first, m_v_pair_cb[0].second, wParam, lParam);
	}
}

void CHidBootManager::_post_progress_range()
{
	if (!m_v_pair_cb.empty()) {
		LPARAM n_max_value = m_RBuffer.get_the_number_of_packet_for_app();
		m_v_pair_cb[0].first(m_v_pair_cb[0].second, CHidBootManager::C_WP_SET_PROGRESS_RANGE, n_max_value);
	}
}
bool CHidBootManager::_is_zero_packet(std::vector<unsigned char> & vPacket )
{
	bool bResult = true;
	std::vector<unsigned char>::iterator iter = vPacket.begin();

	for( ; iter != vPacket.end(); ++iter ){

		if( *iter != 0 ){
			bResult = false;
			break;
		}
	}//end for

	return bResult;
}

bool CHidBootManager::_do_send_data_in_worker(bool b_resend_mode)
{
	if (b_resend_mode) {
		//Deb_Printf(L"..ReDoSendData.\n");
	}
	else {
		//Deb_Printf(L".._do_send_data_in_worker.\n");
	}

	bool bResult = true;

	HidBLRequest* pReq = NULL;
	HidBLReplay* pReplay = NULL;

	std::vector<unsigned char> vReq(CHidBootBuffer::C_PACKET_SIZE);
	std::vector<unsigned char> vReplay(CHidBootBuffer::C_PACKET_SIZE);

	vReq.resize(CHidBootBuffer::C_PACKET_SIZE);
	vReplay.resize(CHidBootBuffer::C_PACKET_SIZE);

	//
	pReq = reinterpret_cast<HidBLRequest*>( &vReq[0] );
	pReplay = reinterpret_cast<HidBLReplay*>( &vReplay[0] );

	//build request packet
	int nSec;
	pReq->cCmd = HIDB_REQ_CMD_WRITE;
	pReq->wLen = CHidBootBuffer::C_SECTOR_SIZE;
	//////////////////////////////////////
	if (!b_resend_mode) {
		//in resend mode, No need these settings.
		m_RBuffer.move_read_buffer_position_to_next();
	}
	///////////////////////////////////////

	nSec = m_RBuffer.get_next_send_sector_number();
	if(nSec>=0){
		pReq->wChain = m_RBuffer.get_packet_chain();
		pReq->dwPara = nSec;

		int n_index = m_RBuffer.get_zero_base_index_of_sector(nSec);

		m_RBuffer.get_one_packet_data_from_buffer(n_index, pReq->sData);

		//_tprintf( L" * sector = %d.\n",nSec );

		if( _DDL_write( m_hDev, &vReq[0], 64, 64 ) ){

			if( m_RBuffer.is_complete_get_one_sector_from_buffer()) {
				
				int nRx;
				do{
					nRx = _DDL_read( m_hDev, &vReplay[0], 64, 64 );
					if( nRx != 64 ){
						bResult = false;
						break;// exit break
					}

				}while( _is_zero_packet( vReplay ) );

				if( bResult ){
					if( pReplay->cResult != HIDB_REP_RESULT_SUCCESS ){
							bResult = false;
					}
				}
			}
		}
		else{
			bResult = false;
		}
	}

	if (bResult)
		m_RBuffer.increase_offset_in_one_sector();
	//
	return bResult;
}


bool CHidBootManager::_do_erase_in_worker(int n_sec)
{
	//Deb_Printf( L".._do_erase_in_worker.\n" );

	bool bResult = true;

	HidBLRequest* pReq = NULL;
	HidBLReplay* pReplay = NULL;

	std::vector<unsigned char> vReq(CHidBootBuffer::C_PACKET_SIZE);
	std::vector<unsigned char> vReplay(CHidBootBuffer::C_PACKET_SIZE);

	vReq.resize(CHidBootBuffer::C_PACKET_SIZE);
	vReplay.resize(CHidBootBuffer::C_PACKET_SIZE);

	//
	pReq = reinterpret_cast<HidBLRequest*>( &vReq[0] );
	pReplay = reinterpret_cast<HidBLReplay*>( &vReplay[0] );

	//erase sector 1~7
	pReq->cCmd = HIDB_REQ_CMD_ERASE;
	pReq->dwPara = n_sec;

	int nRx = 64;

	if( _DDL_TxRx( m_hDev, &vReq[0], 64, 64, &vReplay[0], &nRx, 64 ) ){

		while( _is_zero_packet( vReplay ) ){

			nRx = _DDL_read( m_hDev, &vReplay[0], 64, 64 );

			if( nRx != 64 ){
				bResult = false;
				break;// exit break
			}
		}//end while

		if( bResult ){
			if( pReplay->cResult != HIDB_REP_RESULT_SUCCESS ){

				//retry
				nRx = _DDL_read( m_hDev, &vReplay[0], 64, 64 );

				if( nRx == 64 ){
					bResult = true;
				}
				else
					bResult = false;
			}
		}
	}
	else
		bResult = false;

	return bResult;
}

void CHidBootManager::_woker_for_update()
{
	int  n_Result = 0;
	bool bRun = true;
	bool bDo = false;

	do{
		n_Result = m_waiter.wait_for_one_at_time(C_WAIT_IDLE);

		if (n_Result == m_n_evt_Kill) {
			m_waiter.reset(n_Result);
			bRun = false;
		}
		else if (n_Result == m_n_evt_Do) {
			m_waiter.reset(n_Result);
			// Do code
			bDo = true;
		}
		else if (n_Result == m_n_evt_Stop) {
			m_waiter.reset(n_Result);
			// Do code
			bDo = false;
		}
		else if (n_Result == m_n_evt_Pause) {
			m_waiter.reset(n_Result);
			// Do code
			bDo = false;
		}
		else if (n_Result == m_n_evt_Resume) {
			m_waiter.reset(n_Result);
			// Do code
			bDo = true;
		}
		else {
			if (bDo) {
				if (!_doing_in_worker()) {

				}
			}
		}

	}while( bRun );
#ifdef _WIN32
	ATLTRACE(L"Exit CHidBootManager::_woker_for_update().");
#endif
	
}

int CHidBootManager::_DDL_GetList(std::list<std::wstring>& ListDev, int nVid, int nPid, int nInf)
{
	return 0;
}

HANDLE CHidBootManager::_DDL_open(const std::wstring& szDevicePath)
{
	return 0;
}

bool CHidBootManager::_DDL_close(HANDLE hDevice) {
	bool b_result(false);

	do {

	} while (false);
	return b_result;
}

int CHidBootManager::_DDL_write(HANDLE hDev, unsigned char* lpData, int nTx, int nOutReportSize) {
	return 0;
}

int CHidBootManager::_DDL_read(HANDLE hDev, unsigned char* lpData, int nRx, int nInReportSize) {
	return 0;
}

bool CHidBootManager::_DDL_TxRx(HANDLE hDev, unsigned char* lpTxData, int nTx, int nOutReportSize, unsigned char* lpRxData, int* nRx, int nInReportSize) {
	return 0;
}




std::tuple<bool, bool, unsigned long, unsigned long> CHidBootManager::_get_sector_info_from_device()
{
	//Deb_Printf(L".._get_sector_info_from_device.\n");

	bool bResult = true;
	bool b_exist(false);
	unsigned long n_start_sec(0);
	unsigned long n_sec_area(0);

	HidBLRequest* pReq = NULL;
	HidBLReplay* pReplay = NULL;

	std::vector<unsigned char> vReq(CHidBootBuffer::C_PACKET_SIZE,0);
	std::vector<unsigned char> vReplay(CHidBootBuffer::C_PACKET_SIZE,0);

	vReq.resize(CHidBootBuffer::C_PACKET_SIZE);
	vReplay.resize(CHidBootBuffer::C_PACKET_SIZE);

	//
	pReq = reinterpret_cast<HidBLRequest*>(&vReq[0]);
	pReplay = reinterpret_cast<HidBLReplay*>(&vReplay[0]);

	pReq->cCmd = HIDB_REQ_CMD_GET_SECTOR_INFO;

	int nRx = 64;

	if (_DDL_TxRx(m_hDev, &vReq[0], 64, 64, &vReplay[0], &nRx, 64)) {

		while (_is_zero_packet(vReplay)) {

			nRx = _DDL_read(m_hDev, &vReplay[0], 64, 64);

			if (nRx != 64) {
				bResult = false;
				break;// exit break
			}
		}//end while

		if (bResult) {
			if (pReplay->cResult == HIDB_REP_RESULT_SUCCESS) {
				b_exist = true;
				memcpy(&n_start_sec, &pReplay->sData[0], sizeof(n_start_sec));
				memcpy(&n_sec_area, &pReplay->sData[sizeof(n_start_sec)], sizeof(n_sec_area));
			}
		}
	}
	else
		bResult = false;

	return std::make_tuple(bResult,b_exist, n_start_sec,n_sec_area);
}

int CHidBootManager::GetDeviceList()
{
	int nCnt(0);
	std::lock_guard<std::mutex> lock(m_mutex_main);
	nCnt = _DDL_GetList( m_listDev, 0x134b, 0x0243, -1 );//Elpusk HID bootloader
	return nCnt;
}

bool CHidBootManager::SelectDevice( int nSel )
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( nSel < 0 )
		bResult = false;
	else{
		if( m_listDev.size() <= nSel )
			bResult = false;
		else{

			std::list<std::wstring>::iterator iter = m_listDev.begin();

			for( ; iter != m_listDev.end(); ++iter ){
				if( nSel == 0 )
					break;
				else
					nSel--;
			}

			m_hDev = _DDL_open( *iter );
			if( m_hDev == NULL )
				bResult = false;
			//
			auto result = _get_sector_info_from_device();
			if (std::get<0>(result) && std::get<1>(result)) {
				m_RBuffer.reset(std::get<2>(result), std::get<3>(result),true,false);
			}
			else {
				m_RBuffer.reset();
			}
			
		}
	}

	return bResult;
}

bool CHidBootManager::UnselectDevice()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( m_hDev ){
		_DDL_close( m_hDev );
		m_hDev = NULL;
	}

	return bResult;
}

bool CHidBootManager::GotoApp()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if( m_hDev ){
		unsigned char sTx[64];
		int nRx = 0;

		HidBLRequest* pReq = NULL;
		memset( &sTx, 0, sizeof(sTx) );

		pReq = reinterpret_cast<HidBLRequest*>(sTx);

		pReq->cCmd = HIDB_REQ_CMD_RUN;

		if( _DDL_write( m_hDev, sTx, 64, 64 ) ){
			_DDL_close( m_hDev );
			m_hDev = NULL;
		}
		else{
			bResult = false;
		}

	}
	else{
		bResult = false;
	}

	return bResult;

}
