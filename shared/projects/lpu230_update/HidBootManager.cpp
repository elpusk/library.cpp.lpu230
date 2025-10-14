#include "HidBootManager.h"
#include <memory>
#include <algorithm>
#include <functional>

#ifdef	WIN32
#include <atldef.h>
#include <atltrace.h>
#endif

#include <mp_cstring.h>
#include <mp_csystem_.h>
#include <mp_cwait.h>

#include "cshare.h"

#define	__DISABLE_REAL_TXRX__
// DONT INCLUDE cshare.h

CHidBootManager::CHidBootManager(void) :
	m_bIniOk(true)
{
	_create_cb_worker();
	//
	m_n_evt_cb_kill_woker = m_waiter_cb.generate_new_event();
	m_n_evt_cb_push = m_waiter_cb.generate_new_event();
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

bool CHidBootManager::do_erase_in_worker(int n_sec)
{
	//Deb_Printf( L"..do_erase_in_worker.\n" );

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
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

#ifndef __DISABLE_REAL_TXRX__
	if( _DDL_TxRx(m_pair_dev_ptrs, &vReq[0], 64, 64, &vReplay[0], &nRx, 64 ) ){

		while( _is_zero_packet( vReplay ) ){
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			nRx = _DDL_read(m_pair_dev_ptrs, &vReplay[0], 64, 64 );

			if( nRx != 64 ){
				bResult = false;
				break;// exit break
			}
		}//end while

		if( bResult ){
			if( pReplay->cResult != HIDB_REP_RESULT_SUCCESS ){

				//retry
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				nRx = _DDL_read(m_pair_dev_ptrs, &vReplay[0], 64, 64 );

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
#else
	bResult = true;
#endif // __DISABLE_REAL_TXRX__

	return bResult;
}

bool CHidBootManager::do_write_sector(
	int n_sec
	, const std::vector<unsigned char>& v_sector
)
{
	cshare& sh(cshare::get_instance());
	bool bResult = true;

	size_t n_offset = 0;
	size_t n_loop = CHidBootBuffer::C_SECTOR_SIZE / CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET;
	if (CHidBootBuffer::C_SECTOR_SIZE % CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET != 0) {
		n_loop += 1;
	}

	int n_copy = 0;
	HidBLRequest* pReq = NULL;
	HidBLReplay* pReplay = NULL;

	std::vector<unsigned char> vReq(CHidBootBuffer::C_PACKET_SIZE);
	std::vector<unsigned char> vReplay(CHidBootBuffer::C_PACKET_SIZE);

	vReq.resize(CHidBootBuffer::C_PACKET_SIZE,0);
	vReplay.resize(CHidBootBuffer::C_PACKET_SIZE,0);

	//
	pReq = reinterpret_cast<HidBLRequest*>(&vReq[0]);
	pReplay = reinterpret_cast<HidBLReplay*>(&vReplay[0]);

	//build request packet
	pReq->cCmd = HIDB_REQ_CMD_WRITE;
	pReq->wLen = CHidBootBuffer::C_SECTOR_SIZE;
	pReq->dwPara = n_sec;

	for (uint16_t w_chain = 0; w_chain < (uint16_t)(n_loop - 1); w_chain++) {
		pReq->wChain = w_chain;
		memset(&pReq->sData[0], 0xff, CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET);

		if ((CHidBootBuffer::C_SECTOR_SIZE - n_offset) >= CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET) {
			n_copy = CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET;
		}
		else{
			n_copy = (int)(CHidBootBuffer::C_SECTOR_SIZE - n_offset);
		}
		memcpy(pReq->sData, &v_sector[n_offset], n_copy);
		n_offset += n_copy;

#if defined(_WIN32) && defined(_DEBUG)
		// 하나의 packet 의 데이터 필드 크기는 54 byte 이므로, 4096 크기의 sector 하나를
		// 전송하기 위해서는 76 번의 packet 전송이 필요하다.
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//
#ifndef __DISABLE_REAL_TXRX__
		if (_DDL_write(m_pair_dev_ptrs, &vReq[0], 64, 64)) {

			if (w_chain == (uint16_t)(n_loop - 1)) {

				int nRx;
				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					nRx = _DDL_read(m_pair_dev_ptrs, &vReplay[0], 64, 64);
					if (nRx != 64) {
						bResult = false;
						break;// exit break
					}

				} while (_is_zero_packet(vReplay));

				if (bResult) {
					if (pReplay->cResult != HIDB_REP_RESULT_SUCCESS) {
						bResult = false;
						break; //exit for
					}
				}
			}
		}
		else {
			bResult = false;
			break; //exit for
		}
#else
		bResult = true;
#endif //__DISABLE_REAL_TXRX__

	}//end for

	//////////////////////////////////////


	//_tprintf( L" * sector = %d.\n",nSec );
	//
	return bResult;
}

int CHidBootManager::_DDL_GetList(std::list<std::wstring>& ListDev, int nVid, int nPid, int nInf)
{
	_mp::clibhid& mclibhid(_mp::clibhid::get_manual_instance());
	_mp::type_set_usb_filter set_usb_filter;
	set_usb_filter.emplace(nVid, nPid, nInf);

	mclibhid.set_usb_filter(set_usb_filter);

	mclibhid.update_dev_set_in_manual();
	_mp::clibhid_dev_info::type_set set_dev_info = mclibhid.get_cur_device_set();

	ListDev.clear();

	for (auto item : set_dev_info) {
		ListDev.push_back(
			item.get_path_by_wstring()
		);
	}//end for

	return ListDev.size();
}

CHidBootManager::type_pair_handle CHidBootManager::_DDL_open(const std::wstring& szDevicePath)
{
	_mp::clibhid& mclibhid(_mp::clibhid::get_manual_instance());
	_mp::clibhid_dev_info::type_set set_dev_info = mclibhid.get_cur_device_set();

	_mp::clibhid_dev::type_ptr ptr_dev;
	_mp::clibhid_dev_info::type_ptr ptr_info;
	_mp::clibhid::pair_ptrs dev_ptrs(ptr_dev, ptr_info);

	CHidBootManager::type_pair_handle pair_dev_ptrs("", dev_ptrs);

	for (auto item : set_dev_info) {
		if (item.get_path_by_wstring() != szDevicePath) {
			continue;
		}
		//
		bool b_lpu23x_specific_protocol(false), b_remove_all_zero_rx(true);
		ptr_dev = std::make_shared<_mp::clibhid_dev>(
			item
			, mclibhid.get_briage().get()
			, b_lpu23x_specific_protocol
			, b_remove_all_zero_rx
			);// create & open clibhid_dev.
		if (ptr_dev->is_open()) {
			ptr_info = std::make_shared<_mp::clibhid_dev_info>(item);
			dev_ptrs = std::make_pair(ptr_dev, ptr_info);
			pair_dev_ptrs = std::make_pair(item.get_path_by_string(), dev_ptrs);
		}
		break;
	}//end for

	return pair_dev_ptrs;
}

bool CHidBootManager::_DDL_close(CHidBootManager::type_pair_handle hDevice) {
	bool b_result(false);

	do {
		if (hDevice.first.empty()) {
			continue;
		}

		hDevice.second.first.reset();
		hDevice.second.second.reset();
		hDevice.first.clear();

		b_result = true;
	} while (false);
	return b_result;
}

int CHidBootManager::_DDL_write(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nTx, int nOutReportSize) 
{
	int n_data(0);

	do {
		if (hDev.first.empty()) {
			continue;
		}

		if (!hDev.second.first) {
			continue;
		}
		if (lpData == NULL) {
			continue;
		}
		if (nTx <= 0) {
			continue;
		}

		_mp::type_v_buffer v_tx(nOutReportSize, 0);
		_mp::type_v_buffer v_rx;

		std::copy(&lpData[0], &lpData[nTx], &v_tx[0]);

		if (!cshare::io_write_sync(hDev.second.first, v_tx)) {
			continue;
		}

		n_data = nTx;
	} while (false);
	return n_data;
}

int CHidBootManager::_DDL_read(CHidBootManager::type_pair_handle hDev, unsigned char* lpData, int nRx, int nInReportSize) 
{
	int n_data(0);

	do {
		if (hDev.first.empty()) {
			continue;
		}

		if (!hDev.second.first) {
			continue;
		}
		if (lpData == NULL) {
			continue;
		}
		if (nRx <= 0) {
			continue;
		}

		_mp::type_v_buffer v_rx;
		_mp::cwait w;
		int n_w = w.generate_new_event();
		std::tuple<int, _mp::cwait*, _mp::type_v_buffer*> param(n_w, &w,&v_rx);
		hDev.second.first->start_read(0, 
			[](_mp::cqitem_dev& qi, void* p_user)->std::pair<bool, std::vector<size_t>> {
				std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>* p_param = (std::tuple<int, _mp::cwait*, _mp::type_v_buffer*>*)p_user;

				if (qi.is_complete()) {
					
					int n = std::get<0>(*p_param);
					_mp::cwait* p_w = std::get<1>(*p_param);
					_mp::type_v_buffer* p_v_rx = std::get<2>(*p_param);
					*p_v_rx = qi.get_rx();
					p_w->set(n);//trigger
				}
				
				return std::make_pair(qi.is_complete(), std::vector<size_t>());
			}
		, &param);

		w.wait_for_one_at_time();

		n_data = v_rx.size();
		if(nRx>= v_rx.size()){
			memcpy(lpData, &v_rx[0], v_rx.size());
		}
		else {
			memcpy(lpData, &v_rx[0], nRx);
		}
	} while (false);
	return n_data;
}

bool CHidBootManager::_DDL_TxRx(CHidBootManager::type_pair_handle hDev, unsigned char* lpTxData, int nTx, int nOutReportSize, unsigned char* lpRxData, int* p_nRx, int nInReportSize) 
{
	int n_data(0);

	do {
		if (hDev.first.empty()) {
			continue;
		}

		if (!hDev.second.first) {
			continue;
		}
		if (lpTxData == NULL) {
			continue;
		}
		if (nTx <= 0) {
			continue;
		}
		if (lpRxData == NULL) {
			continue;
		}
		if (p_nRx == NULL) {
			continue;
		}

		_mp::type_v_buffer v_tx(nOutReportSize, 0);
		_mp::type_v_buffer v_rx;

		//std::copy(&lpTxData[0], &lpTxData[nTx], &v_tx[1]);
		std::copy(&lpTxData[0], &lpTxData[nTx], &v_tx[0]);

		if (!cshare::io_write_read_sync(hDev.second.first, v_tx, v_rx, 64)) {
			continue;
		}

		/*
		if (!cshare::io_write_sync(hDev.second.first, v_tx)) {
			continue;
		}

		if (!cshare::io_read_sync(hDev.second.first, v_rx,64)) {
			continue;
		}
		*/
		n_data = v_rx.size();
		if (*p_nRx >= v_rx.size()) {
			memcpy(lpRxData, &v_rx[0], v_rx.size());
		}
		else {
			memcpy(lpRxData, &v_rx[0], *p_nRx);
		}

	} while (false);
	return n_data;
}



std::tuple<bool, bool, uint32_t, uint32_t> CHidBootManager::_get_sector_info_from_device()
{
	//Deb_Printf(L".._get_sector_info_from_device.\n");

	bool bResult = true;
	bool b_exist(false);
	uint32_t n_start_sec(0);
	uint32_t n_sec_area(0);

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

	if (_DDL_TxRx(m_pair_dev_ptrs, &vReq[0], 64, 64, &vReplay[0], &nRx, 64)) {

		while (_is_zero_packet(vReplay)) {

			nRx = _DDL_read(m_pair_dev_ptrs, &vReplay[0], 64, 64);

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

bool CHidBootManager::SelectDevice(const std::string& s_device_path, size_t n_fw_size)
{
	bool b_result(false);
	
	cshare& sh(cshare::get_instance());

	std::wstring s = _mp::cstring::get_unicode_from_mcsc(s_device_path);
	std::list<std::wstring>::iterator iter = m_listDev.begin();

	
	std::vector<int> v_erase_sec_index;
	std::vector<int> v_write_sec_index;

	do {
		if (iter == std::end(m_listDev)) {
			continue;
		}
		//
		m_pair_dev_ptrs = _DDL_open(s);
		if (!m_pair_dev_ptrs.first.empty()) {
			b_result = true;
		}
		bool b_exist_sec_info(false);
		uint32_t n_sec_number_of_start_sec(1); // bootloader 끝난 다음 섹터 번호.

		// app area 의 섹터의 수.(flash sector 수 - bootloader sector 수)
		// fw 의 크기에 맞게 조정됨.
		uint32_t n_the_number_of_sec_of_app_data(7); 
		uint32_t n_need_fw_sec = 0;//fw 를 저장하기 위해 필요한 sector 의 수 

		std::tie(b_result, b_exist_sec_info, n_sec_number_of_start_sec, n_the_number_of_sec_of_app_data) = _get_sector_info_from_device();
		if (!b_result) {
			continue;
		}

		n_need_fw_sec = n_fw_size / CHidBootBuffer::C_SECTOR_SIZE;
		if (n_fw_size % CHidBootBuffer::C_SECTOR_SIZE != 0) {
			++n_need_fw_sec;
		}
		if (n_need_fw_sec == 0) {
			continue;
		}

		if (!b_exist_sec_info) {
			// 기본값 사용.
			n_sec_number_of_start_sec = 1;
			n_the_number_of_sec_of_app_data = 7;

			// fw size 에 맞게 아레 에서 조정되므로 아래 코드 remark 됨. 그러나 돌아가는 설명을 위해 지우지는 않음.
			// LPC1343 를 위한 기본 값으로 지우는 섹터번호의 순서를 저장.
			//v_erase_sec_index = { 1,2,3,4,5,6,7 }; //1~6 app area, 7 system variables

			// LPC1343 를 위한 기본 값으로 기록하는 섹터번호의 순서를 저장.
			// 복구 확률을 높이기 위해 마지막 첫 섹터를 마지막에 기록.
			// 7 sector(마지막 섹터) 는 지우기만하면, fw 가 실행 되면서 기본값을 기록하도록 되어 있어서, 기록 할 필요 없음.
			//v_write_sec_index = { 2,3,4,5,6,1 }; 
		}

		if (n_need_fw_sec > (n_the_number_of_sec_of_app_data-1)) {
			// flash memory sector 중 마지막 sector 는 항상 system area 로 사용됨.
			// app area 의 섹터 수 보다 더 많은 fw 를 기록할 수 없음.
			continue;
		}

		// 지우기 위한 섹터 번호와 기록하기 위한 섹터 번호를 설정.
		for (int ui = 0; ui < n_need_fw_sec; ui++) {
			v_erase_sec_index.push_back(n_sec_number_of_start_sec + ui);
		}//end for
		v_erase_sec_index.push_back(n_sec_number_of_start_sec + n_the_number_of_sec_of_app_data - 1); // system area(항상 마지막 섹터) 도 지우기 위해 추가.

		// 주어진 fw 를 쓰기 위해 실제 필요한 섹터의 수만 처리하며, 처리 속도를 높이기 위해 필요한 계산.
		for (int ui = 0; ui < n_need_fw_sec-1; ui++) {
			v_write_sec_index.push_back(n_sec_number_of_start_sec + ui + 1);
		}//end for
		v_write_sec_index.push_back( n_sec_number_of_start_sec ); // 복구 확률을 높이기 위해 마지막 첫 섹터를 마지막에 기록.

	} while (false);

	if (b_result) {
		sh.set_erase_sec_index(v_erase_sec_index).set_write_sec_index(v_write_sec_index);
	}
	else {
		//reset secter index vextor.
		sh.set_erase_sec_index().set_write_sec_index();
	}

	return b_result;
}

bool CHidBootManager::UnselectDevice()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if(!m_pair_dev_ptrs.first.empty()){
		_DDL_close(m_pair_dev_ptrs);
		m_pair_dev_ptrs.first.clear();
		m_pair_dev_ptrs.second.first.reset();
		m_pair_dev_ptrs.second.second.reset();
	}

	return bResult;
}

bool CHidBootManager::GotoApp()
{
	bool bResult = true;
	std::lock_guard<std::mutex> lock(m_mutex_main);

	if(!m_pair_dev_ptrs.first.empty()){
		unsigned char sTx[64];
		int nRx = 0;

		HidBLRequest* pReq = NULL;
		memset( &sTx, 0, sizeof(sTx) );

		pReq = reinterpret_cast<HidBLRequest*>(sTx);

		pReq->cCmd = HIDB_REQ_CMD_RUN;

		if( _DDL_write(m_pair_dev_ptrs, sTx, 64, 64 ) ){
			_DDL_close(m_pair_dev_ptrs);
			m_pair_dev_ptrs.first.clear();
			m_pair_dev_ptrs.second.first.reset();
			m_pair_dev_ptrs.second.second.reset();
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
