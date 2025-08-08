
#include <hid/mp_clibhid_dev.h>

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32
#ifdef _DEBUG
//#undef __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY__
//#define __THIS_FILE_ONLY_Q__
#include <atltrace.h>
#endif
#endif

namespace _mp {
    clibhid_dev::clibhid_dev(const clibhid_dev_info & info, _hid_api_briage* p_hid_api_briage) :
		m_atll_rx_by_api_in_rx_worker_check_interval_mmsec(clibhid_dev::_const_dev_rx_by_api_in_rx_worker_check_interval_mmsec),
		m_atll_rx_q_check_interval_mmsec(clibhid_dev::_const_dev_rx_q_check_interval_mmsec),
        m_n_dev(-1),
        m_b_run_th_worker(false),
        m_dev_info(info),
        m_b_detect_replugin(false),
        m_p_hid_api_briage(p_hid_api_briage),
        m_n_debug_line(0)
    {
       if (!m_p_hid_api_briage) {
            return;
        }

        bool b_composite(false);
        std::string s_path_composite(info.get_path_by_string());

        std::tie(b_composite, std::ignore, std::ignore, std::ignore) = _vhid_info::is_path_compositive_type(s_path_composite);

        int n_dev = -1;

        do {
            if (!b_composite) {
                //primitive type
                n_dev = m_p_hid_api_briage->api_open_path(info.get_path_by_string().c_str());
                if (n_dev < 0) {
                    continue; //open error
                }

                int n_none_blocking = 1;//enable non-blocking.

                if (m_p_hid_api_briage->api_set_nonblocking(n_dev, n_none_blocking) == 0) {
                    m_n_dev = n_dev;
                    m_b_run_th_worker = true;
                    m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(&clibhid_dev::_worker, this));
#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
                    pthread_setname_np(m_ptr_th_worker->native_handle(), "clibhid_dev");
#endif
                }
                else {
                    m_p_hid_api_briage->api_close(n_dev);
                    m_n_dev = -1;
                }
                continue;
            }

            // compositive type
            n_dev = m_p_hid_api_briage->api_open_path(info.get_path_by_string().c_str());
            if (n_dev < 0) {
                continue;
            }
            m_n_dev = n_dev;
            m_b_run_th_worker = true;
            m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(&clibhid_dev::_worker, this));
#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
            pthread_setname_np(m_ptr_th_worker->native_handle(), "clibhid_dev");
#endif
        } while (false);
    }

    clibhid_dev::~clibhid_dev()
    {
        m_b_run_th_worker = false;

        if (m_p_hid_api_briage && m_n_dev >= 0) {
            m_p_hid_api_briage->api_close(m_n_dev);
        }

        if (m_ptr_th_worker) {
                
            if (m_ptr_th_worker->joinable()) {
                m_ptr_th_worker->join();
            }
        }
        m_n_dev = -1;
    }

    bool clibhid_dev::is_detect_replugin()
    {
        return m_b_detect_replugin;
    }

    bool clibhid_dev::is_open() const
    {
        if (m_n_dev>=0)
            return true;
        else
            return false;
    }

    bool clibhid_dev::is_support_shared_open() const
    {
        return m_dev_info.is_support_shared_open();
    }

    std::vector<unsigned char> clibhid_dev::get_report_desciptor()
    {
        std::vector<unsigned char> v(4096,0);//HID_API_MAX_REPORT_DESCRIPTOR_SIZE

        do {
            if (!m_p_hid_api_briage) {
                continue;
            }
            int n_result = 0;
            n_result = m_p_hid_api_briage->api_get_report_descriptor(m_n_dev, &v[0], v.size());
            if (n_result < 0) {
                continue;
            }

            v.resize(n_result);

        } while (false);
        return v;
    }

    void clibhid_dev::start_write(size_t n_req_uid,const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb_for_packet cb_for_packet, void* p_user_for_cb, unsigned long n_session_number/*=0*/)
    {
        cqitem_dev::type_ptr ptr(new cqitem_dev(n_req_uid,v_tx, false, cb_for_packet, p_user_for_cb, n_session_number));

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_Q__))
        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
            ATLTRACE(L" =======(%ls)[%u:%u] push req(%ls) -> clibhid_dev.\n",
                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                ptr->get_session_number(), ptr->get_uid(),
                ptr->get_request_type_by_string().c_str()
            );
        }
#endif

        m_q_ptr.push(ptr);
    }

    void clibhid_dev::start_read(size_t n_req_uid, cqitem_dev::type_cb_for_packet cb_for_packet, void* p_user_for_cb, unsigned long n_session_number/*=0*/)
    {
        cqitem_dev::type_ptr ptr(new cqitem_dev(n_req_uid,_mp::type_v_buffer(0), true, cb_for_packet, p_user_for_cb, n_session_number));

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_Q__))
        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
            ATLTRACE(L" =======(%ls)[%u:%u] push req(%ls) -> clibhid_dev.\n",
                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                ptr->get_session_number(), ptr->get_uid(),
                ptr->get_request_type_by_string().c_str()
            );
        }
#endif
        m_q_ptr.push(ptr);
    }

    void clibhid_dev::start_write_read(size_t n_req_uid, const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb_for_packet cb_for_packet, void* p_user_for_cb, unsigned long n_session_number/*=0*/)
    {
        cqitem_dev::type_ptr ptr_tx(new cqitem_dev(n_req_uid,v_tx, true, cb_for_packet, p_user_for_cb, n_session_number));

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_Q__))
        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
            ATLTRACE(L" =======(%ls)[%u:%u] push req(%ls) -> clibhid_dev.\n",
                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                ptr_tx->get_session_number(), ptr_tx->get_uid(),
                ptr_tx->get_request_type_by_string().c_str()
            );
        }
#endif
        m_q_ptr.push(ptr_tx);
    }

    void clibhid_dev::start_cancel(size_t n_req_uid, cqitem_dev::type_cb_for_packet cb_for_packet, void* p_user_for_cb, unsigned long n_session_number/*=0*/)
    {
        //cancel -> tx is none, no need rx!
        cqitem_dev::type_ptr ptr(new cqitem_dev(n_req_uid,_mp::type_v_buffer(0), false, cb_for_packet, p_user_for_cb, n_session_number));

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_Q__))
        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
            ATLTRACE(L" =======(%ls)[%u:%u] push req(%ls) -> clibhid_dev.\n",
                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                ptr->get_session_number(), ptr->get_uid(),
                ptr->get_request_type_by_string().c_str()
            );
        }
#endif
        m_q_ptr.push(ptr);
    }

    type_bm_dev clibhid_dev::get_type() const
    {
        return m_dev_info.get_type();
    }

    clibhid_dev& clibhid_dev::set_rx_q_check_interval(long long n_interval_mmsec)
    {
        if (m_p_hid_api_briage) {
			m_p_hid_api_briage->set_req_q_check_interval_in_child(n_interval_mmsec);
        }

		m_atll_rx_q_check_interval_mmsec.store(n_interval_mmsec, std::memory_order_relaxed);
        return *this;
    }

    clibhid_dev& clibhid_dev::set_rx_by_api_in_rx_worker_check_interval(long long n_interval_mmsec)
    {
        if (m_p_hid_api_briage) {
			m_p_hid_api_briage->set_hid_read_interval_in_child(n_interval_mmsec);
        }
        m_atll_rx_by_api_in_rx_worker_check_interval_mmsec.store(n_interval_mmsec, std::memory_order_relaxed);
		return *this;
    }

    clibhid_dev& clibhid_dev::set_tx_by_api_check_interval(long long n_interval_mmsec)
    {
        if (m_p_hid_api_briage) {
            m_p_hid_api_briage->set_hid_write_interval_in_child(n_interval_mmsec);
        }
		return *this;
    }

    /**
    * called by _worker()
    */
    std::pair<bool, bool> clibhid_dev::_pump()
    {
        bool b_read_is_normal = true;
        bool b_expected_replugin = false;

        int n_size_in_report(m_dev_info.get_size_in_report());
        _mp::type_v_buffer v_rx(n_size_in_report, 0);

        std::tie(b_read_is_normal, b_expected_replugin) = _read_using_thread(v_rx);//pumpping.
        m_q_rx_ptr_v.clear();//pumpping.

        if (!b_read_is_normal) {
            clog::get_instance().trace(L"T[W] - %ls - _read_using_thread() in pump.\n", __WFUNCTION__);
            clog::get_instance().log_fmt(L"[W] - %ls - _read_using_thread() in pump.\n", __WFUNCTION__);
            if (b_expected_replugin) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                ATLTRACE(L" =******= DECTECT plugout.\n");
#endif
                m_b_detect_replugin = true;//need this device instance removed
            }
        }
        return std::make_pair(b_read_is_normal, b_expected_replugin);
    }

    void clibhid_dev::_clear_rx_q_with_lock()
    {
        int n_flush = 0;
        _mp::type_ptr_v_buffer ptr_flush_buffer;

        std::lock_guard<std::mutex> lock(_hid_api_briage::get_mutex_for_hidapi());

        while (m_q_rx_ptr_v.try_pop(ptr_flush_buffer)) {
            n_flush++;
            std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_rx_flush_interval_mmsec));
        }//end while
        m_q_rx_ptr_v.clear();//reset rx queue.
        clog::get_instance().log_fmt_in_debug_mode(L"[D] - %ls - flush cnt = %d.\n", __WFUNCTION__, n_flush);

    }

    /**
    * @brief Write an Output report to a HID device.
    * the fist byte must be report ID.(automatic add 
    * @param v_tx - this will be sent by split-send as in-report size
    * @param b_req_is_tx_rx_pair - true(after done tx, need to receive response.)
    */
    bool clibhid_dev::_write_with_lock(const std::vector<unsigned char>& v_tx, bool b_req_is_tx_rx_pair)
    {
        bool b_result(false);
        size_t n_start = 0;
        size_t n_offset = 0;

        do {
            if (!m_p_hid_api_briage) {
                continue;
            }
            if (v_tx.size() == 0) {
                continue;
            }
            int n_report = m_dev_info.get_size_out_report();
            if (n_report <= 0) {
                continue;
            }

            //the number of tx reports.
            size_t n_loop = v_tx.size() / n_report;
            if ( v_tx.size() % n_report ) {
                ++n_loop;
            }
#ifdef _WIN32
            ++n_report;//for report ID
            n_start = 1;
#endif  
            _mp::type_v_buffer v_report(n_report, 0);

            int n_result = -1;
            b_result = true;

            _hid_api_briage::type_next_io next(_hid_api_briage::next_io_none);

            for (size_t i = 0; i < n_loop; i++) {
                std::fill(v_report.begin(), v_report.end(), 0);
                if ((v_tx.size() - n_offset) >= (v_report.size() - n_start)) {
                    std::copy(v_tx.begin() + n_offset, v_tx.begin() + n_offset+ (v_report.size() - n_start), v_report.begin() + n_start);
                }
                else {
                    std::copy(v_tx.begin() + n_offset, v_tx.end(), v_report.begin() + n_start);
                }
                
                if (i + 1 == n_loop) {
                    //last write
                    if (b_req_is_tx_rx_pair) {
                        next = _hid_api_briage::next_io_read;
                    }
                    else {
                        next = _hid_api_briage::next_io_none;
                    }
                }
                else {
                    next = _hid_api_briage::next_io_write;
                }
                n_result = m_p_hid_api_briage->api_write(m_n_dev, &v_report[0], v_report.size(), next);
                //_mp::clog::get_instance().log_data_in_debug_mode(v_report, L"v_report = ", L"\n");
                if (n_result < 0) {
                    b_result = false;
                    _mp::clog::get_instance().log_fmt(L"[E] hid_write() < 0(n_result : n_offset : v_tx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_tx.size());
                    break;
                }
                if (n_result != v_report.size()) {
                    _mp::clog::get_instance().log_fmt(L"[E] hid_write() != v_report.size() (n_result : n_offset : v_tx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_tx.size());
                    b_result = false;
                    break;
                }

                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] (n_result : n_offset : v_tx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_tx.size());

                n_offset += n_result;

            }//end for
                
        } while (false);
        return b_result;
    }

    /**
    * read by report unit
    * @return first true - rx success, second true - mayne device is plug out and in at error status.  
    */
    std::pair<bool,bool> clibhid_dev::_read_using_thread(std::vector<unsigned char>& v_rx)
    {
        bool b_result(false);
        bool b_continue(false);
        bool b_expected_replugin(false);//only meaned in b_result is false
#ifdef _WIN32            
        int n_start_offset = 0;//none for report ID
#else
        int n_start_offset = 0;//for report ID
#endif
        int n_offset = n_start_offset;
        int n_rety_for_lost_packet = 0;

        size_t n_report = m_dev_info.get_size_in_report();
        if (n_report <= 0) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] get_size_in_report = 0.\n");
            return std::make_pair(b_result, b_expected_replugin);//error
        }

        v_rx.resize(n_report + n_start_offset,0);

		long long ll_rx_q_check_interval_mmsec = m_atll_rx_q_check_interval_mmsec.load(std::memory_order_relaxed);

        do {
            b_continue = false;

            _mp::type_ptr_v_buffer ptr_v;
            int n_result = 0;

            //get RX queue
            if (!m_q_rx_ptr_v.try_pop(ptr_v)) {
                n_result = 0;//None rx
            }
            else {
                if (!ptr_v) {
                    n_result = -1;//error. recover impossible
                }
                else {
                    n_result = (int)ptr_v->size();
                    if (n_result > 0) {
                        std::copy(ptr_v->begin(), ptr_v->end(), &v_rx[n_offset]);
                    }
                    else if (n_result == 0) {
                        //lost packet
                        n_result = -2;//error. lost packet
                    }
                }
            }
            //
            if (n_result == 0) {
                if (n_offset == n_start_offset) {
                    v_rx.resize(0);
                    b_result = true;
                    continue;//exit none data
                }
                if (n_offset >= v_rx.size()) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 1(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                    b_result = true;
                    continue;//exit complete
                }

                //here n_offset < v_rx.size()
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 2(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                ++n_rety_for_lost_packet;

                if (n_rety_for_lost_packet < clibhid_dev::_const_lost_packet_retry_counter) {
                            
                    std::this_thread::sleep_for(std::chrono::milliseconds(ll_rx_q_check_interval_mmsec));
                    b_continue = true;
                    continue;//retry rx
                }
                //error lost packet                         
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[E] lost packet(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                v_rx.resize(0);
                continue;//error exit
            }

            if (n_result < 0) {
                if (n_result == -1) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 3critical(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                    b_expected_replugin = true;
                }
                if (n_result == -2) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 3lost(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                }

                v_rx.resize(0);
                continue;//error exit
            }

            //here! n_result > 0
            n_offset += n_result;
            //
            if (n_offset > v_rx.size()) {
                //here may be some packet is losted, and get another packet!. error
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 4(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                v_rx.resize(0);
                continue;//error exit
            }

            if (n_offset < v_rx.size()) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 5(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
 
                //need more 
                std::this_thread::sleep_for(std::chrono::milliseconds(ll_rx_q_check_interval_mmsec));
                n_rety_for_lost_packet = 0;
                b_continue = true;
                continue;
            }
            //here! n_result == v_rx.size()
            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 6(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());

            // complete read
            b_result = true;
        } while (b_continue);

        return std::make_pair(b_result, b_expected_replugin);
    }

    std::pair<bool, bool> clibhid_dev::_process_new_request_and_set_result(cqitem_dev::type_ptr& ptr_req_new, cqitem_dev::type_ptr& ptr_req_cur)
    {
        bool b_request_is_cancel(false);
        bool b_complete(true);
        bool b_result(false);
        _mp::type_v_buffer v_rx(0);


        do {
            // 현재 실행 중인 명령 있으면, 새로운 것 실행하기 전에 취소.
            // ptr_cur 의 결과를 설정한다.
            if (ptr_req_cur) {
                ptr_req_cur->set_result(cqitem_dev::result_cancel, type_v_buffer(0), L"CANCELLED BY ANOTHER REQ AUTO");
            }

            switch (ptr_req_new->get_request_type()) {
            case cqitem_dev::req_only_tx:
                b_result = _process_only_tx(ptr_req_new);
                break;
            case cqitem_dev::req_tx_rx:
                std::tie(b_result,b_complete, v_rx) = _process_tx_rx(ptr_req_new);
                break;
            case cqitem_dev::req_only_rx:
                std::tie(b_result, b_complete, v_rx) = _process_only_rx(ptr_req_new);
                break;
            case cqitem_dev::req_cancel:
                b_request_is_cancel = true;
            default:
                //b_result = _process_cancel(ptr_req_new);//result is ignored
                ptr_req_new->set_result(cqitem_dev::result_success, type_v_buffer(0), L"SUCCESS CANCEL REQ");
                continue;
            }//end switch

            if (b_complete) {
                if (b_result) {
                    ptr_req_new->set_result(cqitem_dev::result_success, v_rx, L"SUCCESS");
                }
                else {
                    ptr_req_new->set_result(cqitem_dev::result_error, v_rx, L"ERROR");
                }
            }
        } while (false);


        return std::make_pair(b_complete,b_request_is_cancel);
    }

    bool clibhid_dev::_process_only_tx(cqitem_dev::type_ptr& ptr_req)
    {
        bool b_result(false);

        do {
            bool b_req_is_tx_rx_pair(false);

            if (ptr_req->get_request_type() == cqitem_dev::req_tx_rx) {
                b_req_is_tx_rx_pair = true;
            }

            _clear_rx_q_with_lock();//flush rx q

            if (!_write_with_lock(ptr_req->get_tx(), b_req_is_tx_rx_pair)) {//ERROR TX
                continue;
            }
            b_result = true;

        } while (false);

        return b_result;
    }

    bool clibhid_dev::_process_cancel(cqitem_dev::type_ptr& ptr_req)
    {
        bool b_result(false);

        do {
            _clear_rx_q_with_lock();//flush rx q

            if (!m_p_hid_api_briage) {
                continue;
            }

            int n_result = m_p_hid_api_briage->api_write(m_n_dev, NULL, 0, _hid_api_briage::next_io_none);
            if (n_result < 0) {
                _mp::clog::get_instance().log_fmt(L"[E] hid_write()-cancel < 0(n_result) = (%d).\n", n_result);
                continue;
            }
            if (n_result != 0) {
                _mp::clog::get_instance().log_fmt(L"[E] hid_write()-cancel != 0 (n_result) = (%d).\n", n_result);
                continue;
            }

            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] (n_result) = (%d).\n", n_result);

            b_result = true;

        } while (false);

        return b_result;
    }

    std::tuple<bool, bool, _mp::type_v_buffer> clibhid_dev::_process_only_rx(cqitem_dev::type_ptr& ptr_req)
    {
        // ptr_req->get_request_type() == cqitem_dev::req_tx_rx 일때는 _process_only_rx_for_txrx() 를 사용하라!
        bool b_complete(true);
        bool b_result(false);
        _mp::type_v_buffer v_rx(0);

        do {
            bool b_expected_replugin = false;
            //
            std::tie(b_result, b_expected_replugin) = _read_using_thread(v_rx);
            if (!b_result) {//ERROR RX
                clog::get_instance().trace(L"T[E] - %ls - _read_using_thread().\n", __WFUNCTION__);
                clog::get_instance().log_fmt(L"[E] - %ls - _read_using_thread().\n", __WFUNCTION__);

                if (b_expected_replugin) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    ATLTRACE(L" =******= DECTECT plugout.\n");
#endif
                    m_b_detect_replugin = true;//need this device instance removed
                }
                continue;
            }

            if (v_rx.size() == 0) {
                b_complete = false;
                continue;
            }

            //RX OK.

        } while (false);
        return std::make_tuple(b_result, b_complete, v_rx);
    }

    /**
    * transmit and receving data
    *   first : processing result( true - none error ),
    *   second : processing complete(true), need more processing(false. in this case. first must be true)
    *   third : rx data from device.
    */
    std::tuple<bool, bool, _mp::type_v_buffer> clibhid_dev::_process_tx_rx(cqitem_dev::type_ptr& ptr_req)
    {
        bool b_complete(true);
        bool b_result(false);
        _mp::type_v_buffer v_rx(0);

        do {
            b_result = _process_only_tx(ptr_req);
            if (!b_result) {
                continue;
            }
            //

            std::tie(b_result,v_rx) = _process_only_rx_for_txrx(ptr_req);

        } while (false);
        return std::make_tuple(b_result, b_complete,v_rx);
    }

    std::tuple<bool, _mp::type_v_buffer> clibhid_dev::_process_only_rx_for_txrx(cqitem_dev::type_ptr& ptr_req)
    {
        bool b_result(false);
        _mp::type_v_buffer v_rx(0);
        bool b_wait(false);
        int n_try_cnt = clibhid_dev::_const_dev_rx_retry_counter_in_txrx_req;

        do {
            if (b_wait) {
                if (n_try_cnt <= 0) {
                    b_result = false;
                    b_wait = false;
                    clog::get_instance().trace(L"T[E] - %ls - over retry read.\n", __WFUNCTION__);
                    clog::get_instance().log_fmt(L"[E] - %ls - over retry read.\n", __WFUNCTION__);
                    continue;// 읽기 최대 시도 회수 지남.
                }

                --n_try_cnt;
                // 다시 읽기 시도를 위해 시간 지연을 줌.
                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_io_check_interval_mmsec)); 
            }

            b_wait = false;
            bool b_expected_replugin = false;
            //
            std::tie(b_result, b_expected_replugin) = _read_using_thread(v_rx);
            if (!b_result) {//ERROR RX
                clog::get_instance().trace(L"T[E] - %ls - _read_using_thread().\n", __WFUNCTION__);
                clog::get_instance().log_fmt(L"[E] - %ls - _read_using_thread().\n", __WFUNCTION__);

                if (b_expected_replugin) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    ATLTRACE(L" =******= DECTECT plugout.\n");
#endif
                    m_b_detect_replugin = true;//need this device instance removed
                }
                continue;
            }

            if (v_rx.size() == 0) {
                b_wait = true; // 에러는 아닌데 받은게 없어서 좀더 기다림.
                continue;
            }

            //RX OK.
            if (v_rx[0] != 'R') {//lpu237 specific protocol // ptr_req->get_request_type() == cqitem_dev::req_tx_rx 일때만 적용.
                // very important code - fix miss-matching txrx protocol. 
                // 펨웨어에서, msr 이나 ibutton 데이터를 보내려고, usb buffer 에 데이타를 쓰고 있는 때,
                // tx 가 전송되면, api 는 tx 에 대한 응답으로 msr 이나 ibutton 데이터를 받을수 있다.
                // 이러한 경우 프로토콜 미스로 문제가 생기므로, 이 msr 이나 ibutton 은 무시되어야 한다. 무조건 !
                b_wait = true; // 다시 읽기 시도.
                v_rx.resize(0);
                //
                clog::get_instance().trace(L"T[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                clog::get_instance().log_fmt(L"[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                //
                continue; //and need re-read.
            }

        } while (b_wait);
        return std::make_tuple(b_result, v_rx);
    }

    /**
    * @brief receiving worker thread. this thread will be construct & destruct on _worker().
    * from device, receving a data by in-report size unit. and it enqueue to m_q_ptr.
    */
    void clibhid_dev::_worker_rx()
    {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
        ATLTRACE(L" =======start clibhid_dev::_worker_rx(0x%08x).\n", m_n_dev);
#endif
        size_t n_report = m_dev_info.get_size_in_report();
        if (n_report <= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
            ATLTRACE(L" =======Exit[E] clibhid_dev::_worker_rx(0x%08x).\n", m_n_dev);
#endif
            return;
        }

        bool b_need_deep_recover = false;

        _mp::type_v_buffer v_report_in(n_report, 0);

		int n_reload_counter = 0;
		long long ll_rx_by_api_in_rx_worker_check_interval_mmsec = m_atll_rx_by_api_in_rx_worker_check_interval_mmsec.load(std::memory_order_relaxed);

        while (m_b_run_th_worker) {

            if (b_need_deep_recover) {
                std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                continue;//recover is impossible!
            }

            v_report_in.assign(v_report_in.size(), 0);//reset contents
            int n_result = 0;

            do {
                if (!m_p_hid_api_briage) {
                    continue;//nothing to do
                }
                n_result = m_p_hid_api_briage->api_read(m_n_dev, &v_report_in[0], v_report_in.size(), v_report_in.size());//wait block or not by initialization
                if (n_result == 0) {
                    continue;//re-read transaction
                }
                if (n_result < 0) {
                    // warning
                    b_need_deep_recover = true;//this case - recover is impossible!

                    std::wstring s_error(m_p_hid_api_briage->api_error(m_n_dev));
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] rx return = %d. - %ls.\n", n_result, s_error.c_str());

                    m_q_rx_ptr_v.push(_mp::type_ptr_v_buffer());//indicate error of device usb io.
                    continue;
                }
                //here n_result > 0 recevied some data.
                if (n_result < v_report_in.size()) {
                    // _vhid_api_briage::api_read() 는 에러가 아닌 경우,항상 in-report 크기로 rx 를 반환하는데 이런 경우는 뭔가 엄청 잘못
                    m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(0));
                    _mp::clog::get_instance().log_fmt(L"[E] %ls : lost packet - push zero size buffer.\n", __WFUNCTION__);
                    continue;
                }
                //one report complete.
                m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(v_report_in));
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[H] v_rx.size() = %u.\n", v_report_in.size());
                _mp::clog::get_instance().log_data_in_debug_mode(v_report_in, std::wstring(), L"\n");

            } while (false);

            ++n_reload_counter;
            if (n_reload_counter > (2000/ ll_rx_by_api_in_rx_worker_check_interval_mmsec)) {
				n_reload_counter = 0;
                // check if rx_by_api_in_rx_worker_check_interval_mmsec is changed.
                auto ll = m_atll_rx_by_api_in_rx_worker_check_interval_mmsec.load(std::memory_order_relaxed);
                if (ll_rx_by_api_in_rx_worker_check_interval_mmsec != ll) {
                    ll_rx_by_api_in_rx_worker_check_interval_mmsec = ll;
                }
            }
            // for reducing CPU usage rates.
            std::this_thread::sleep_for(std::chrono::milliseconds(ll_rx_by_api_in_rx_worker_check_interval_mmsec));
        }//end while

        std::wstringstream ss;
        ss << std::this_thread::get_id();
        std::wstring s_id = ss.str();
        //_mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str()); ,, dead lock

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
        ATLTRACE(L" =======Exit clibhid_dev::_worker_rx(0x%08x-%s).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
    }

    void clibhid_dev::_worker()
    {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
        ATLTRACE(L" =======start clibhid_dev::_worker(0x%08x-%s).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
        std::deque< cqitem_dev::type_ptr > dq_ptr_cur_old;
        //cqitem_dev::type_ptr ptr_cur_old; ///////
        cqitem_dev::type_ptr ptr_new, ptr_cur;
        int n_size_in_report(m_dev_info.get_size_in_report());
        _mp::type_v_buffer v_rx(n_size_in_report, 0);

		// thread for rx
        std::shared_ptr<std::thread> ptr_th_rx(new std::thread(&clibhid_dev::_worker_rx, this));

#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
        pthread_setname_np(ptr_th_rx->native_handle(), "_worker_rx");
#endif

#ifndef _WIN32
        //set priority level of _worker_rx()
        int n_priority = 90;// 10;
        int n_policy = SCHED_FIFO;//SCHED_RR
        pthread_t nativeHandle = ptr_th_rx->native_handle();
        sched_param sch_params;
        sch_params.sched_priority = n_priority;
        if (pthread_setschedparam(nativeHandle, n_policy, &sch_params)) {//NEED root 
            clog::get_instance().trace(L"T[E] - %ls - Failed to set thread priority.\n", __WFUNCTION__);
            clog::get_instance().log_fmt(L"[E] - %ls - Failed to set thread priority.\n", __WFUNCTION__);
        }

#endif
        bool b_read_ok = true;
        bool b_expected_replugin = false;
        bool b_complete(true);
        bool b_request_is_cancel(false);

        while(m_b_run_th_worker){

            do {
                if (!m_q_ptr.try_pop(ptr_new)) {
                    // 새로운 명령 없음.
                    if (!ptr_cur) {
                        // 현재 작업 주인 것 없으면.
                        // 처리 안된, async 가 있으면, ptr_cur 에 다시 assing 해서 다음 loop 에서 실행.
                        if (!dq_ptr_cur_old.empty()) {
                            ptr_cur = dq_ptr_cur_old.front();
                            dq_ptr_cur_old.pop_front();

                            ptr_cur->reset_result();

#if defined(_WIN32) && defined(_DEBUG)
                            if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                                if (m_n_debug_line != (__LINE__ + 1)) {
                                    m_n_debug_line = __LINE__;
                                    ATLTRACE(L" =======(%ls) old cur req(%ls) -> cur\n",
                                        _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                        ptr_cur->get_request_type_by_string().c_str()
                                    );
                                }
                            }
#endif
                            continue; // go ptr_cur.callback, ptr_new.callback
                        }
                        // pumping
                        std::tie(b_read_ok, b_expected_replugin) = _pump();//pumpping.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                            if (m_n_debug_line != (__LINE__+1)) {
                                m_n_debug_line = __LINE__;
                                ATLTRACE(L" =======(%ls) pump().\n",
                                    _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str()
                                );
                            }
                        }
#endif
                        continue;// go ptr_cur.callback, ptr_new.callback
                    }

                    // 현재 작업 중인 것 있으면,
                }
                else {
                    // pop된 새로운 명령의 경우.
                    if (!b_read_ok) {
                        // 새로운 명령을 pop  했지만, 현재 rx 가 에러 상태이므로 새로운 명령은 바로 에러로 처리.
                        clog::get_instance().trace(L"T[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                        clog::get_instance().log_fmt(L"[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                        ptr_new->set_result(cqitem_dev::result_error, type_v_buffer(0), L"RX");
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                            if (m_n_debug_line != (__LINE__+1)) {
                                m_n_debug_line = __LINE__;
                                ATLTRACE(L" =======(%ls) new req(%ls).b_read_ok.false.\n",
                                    _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                    ptr_new->get_request_type_by_string().c_str()
                                );
                            }
                        }
#endif
                        continue;// go ptr_cur.callback, ptr_new.callback
                    }

                    if (ptr_cur) {
                        // 처리하던 것이 있으면,
                        if (ptr_cur->get_request_type() == cqitem_dev::req_only_rx && ptr_new->get_request_type() == cqitem_dev::req_only_rx) {
                            // 현재 처리 중인 것과 새로 받은 것이 모두 rx_only 의 경우,
                            // 새로운 것은 기존 것과 같이 처리 되므로, 무시하고, 해당 app 사용이 종료 될때 처리를 위해
                            // 저장만 함.
                            dq_ptr_cur_old.push_back(ptr_new);
                            ptr_new.reset();
                            continue;
                        }
                    }

                    // 새로운 명령을 처리.
                    std::tie(b_complete,b_request_is_cancel) = _process_new_request_and_set_result(ptr_new, ptr_cur);

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__ + 1)) {
                            m_n_debug_line = __LINE__;

                            std::wstring __s_complete(L"complete"), __s_cancel(L"req is cancel");
                            if (!b_complete) {
                                __s_complete = L"ing";
                            }
                            if (!b_request_is_cancel) {
                                __s_cancel = L"req not cancel";
                            }
                            std::wstring __s_new(L"none"), __s_cur(L"none");
                            size_t __n_s_new(0), __n_uid_new(0);
                            if (ptr_new) {
                                __s_new = ptr_new->get_request_type_by_string();
                                __n_s_new = ptr_new->get_session_number();
                                __n_uid_new = ptr_new->get_uid();
                            }
                            size_t __n_s_cur(0), __n_uid_cur(0);
                            if (ptr_cur) {
                                __s_cur = ptr_cur->get_request_type_by_string();
                                __n_s_cur = ptr_cur->get_session_number();
                                __n_uid_cur = ptr_cur->get_uid();
                            }

                            ATLTRACE(L" =======(%ls) _process_new_request_and_set_result( new:%ls:%u:%u , cur:%ls:%u:%u ) = (%ls,%ls)\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                __s_new.c_str(),
                                __n_s_new, __n_uid_new,
                                __s_cur.c_str(),
                                __n_s_cur, __n_uid_cur,
                                __s_complete.c_str(),
                                __s_cancel.c_str()
                            );
                        }
                    }
#endif

                    if (b_request_is_cancel) {
                        // 이 경우 b_complete 는 항상 true.
                        // ptr_new 는 cancel req 로 ptr_cur가 있는 경우, ptr_cur 를 cancel 시킴.
                        // 따라서 구조상, ptr_cur 에 대한 cancel nortificate 먼저 하고, 
                        // ptr_new 에 대한 nortificate 나중에.(notification AREA)
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                            if (m_n_debug_line != (__LINE__+1)) {
                                m_n_debug_line = __LINE__;
                                ATLTRACE(L" =======(%ls)%u:%u: new req(%ls).b_request_is_cancel.true.\n",
                                    _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                    ptr_new->get_session_number(), ptr_new->get_uid(),
                                    ptr_new->get_request_type_by_string().c_str()
                                );
                            }
                        }
#endif
                        continue;// go ptr_cur.callback, ptr_new.callback
                    }
                    if (b_complete) {
                        // ptr_new 가 동기명령인 경우는 여기에.
                        // 기존 ptr_cur 이 있는 경우, 동기명령 ptr_new 에 의해 취소되므로,
                        // 따라서 구조상, ptr_cur 에 대한 cancel notificate 먼저 하고,
                        // ptr_new 에 대한 nortificate 나중에.(notification AREA)
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                            if (m_n_debug_line != (__LINE__+1)) {
                                m_n_debug_line = __LINE__;
                                ATLTRACE(L" =======(%ls) new req(%ls).b_complete.true.\n",
                                    _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                    ptr_new->get_request_type_by_string().c_str()
                                );
                            }
                        }
#endif
                        continue; // 새로운 명령은 처리됨.
                    }
/*
                    if (ptr_cur) {
                        // 새로운 명령 처리가 더 필요하므로 기존 명령이 cancel 된 것을,
                        // callback 해서 알려 준다. 
                        // ptr_new 가 비동기명령인 경우는 여기에.
                        // ptr_cur가 있는 경우, ptr_cur 를 cancel 시키고 cancel notificate 를 아래 코드로.
                        ptr_cur->run_callback_for_packet();
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                        if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                            if (m_n_debug_line != (__LINE__ + 1)) {
                                m_n_debug_line = __LINE__;
                                ATLTRACE(L" =======(%ls)%u:%u: cur req(%ls).canceled.\n",
                                    _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                    ptr_cur->get_session_number(),
                                    ptr_cur->get_uid(),
                                    ptr_cur->get_request_type_by_string().c_str()
                                );
                            }
                        }
#endif
                    }
*/
                    // ptr_new 가 비동기명령 시, 응답 수신전인 경우 여기에.
                    // 새로운 명령 처리가 더 필요하므로, 현재 명령으로 설정.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__ + 1)) {
                            m_n_debug_line = __LINE__;
                            std::wstring __s_new(L"none"), __s_cur(L"none");
                            size_t __n_s_new(0), __n_uid_new(0);
                            if (ptr_new) {
                                __s_new = ptr_new->get_request_type_by_string();
                                __n_s_new = ptr_new->get_session_number();
                                __n_uid_new = ptr_new->get_uid();
                            }

                            size_t __n_s_cur(0), __n_uid_cur(0);
                            if (ptr_cur) {
                                __s_cur = ptr_cur->get_request_type_by_string();
                                __n_s_cur = ptr_cur->get_session_number();
                                __n_uid_cur = ptr_cur->get_uid();
                            }
                            ATLTRACE(L" =======(%ls) new req(%ls:%u:%u) -> cur req(%ls:%u:%u).\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                __s_new.c_str(),
                                __n_s_new, __n_uid_new,
                                __s_cur.c_str(),
                                __n_s_cur, __n_uid_cur
                            );
                        }
                    }
#endif
                    if (ptr_cur) {
                        dq_ptr_cur_old.push_back(ptr_cur);
                    }
                    ptr_cur = ptr_new; // ptr_cur 이 overwrite 되어도 내부 q 에서 유지되어 메모리에서 제거 되지 않음. 
                    ptr_new.reset();
                }

                // 현재 명령으로 설정된 명령을 위해 계속 수신 시도함.
                std::tie(b_read_ok, b_complete, v_rx) = _process_only_rx(ptr_cur);
                if (!b_complete) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__+1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%s)%u:%u: _process_only_rx.\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                ptr_cur->get_session_number(),
                                ptr_cur->get_uid()
                            );
                        }
                    }
#endif
                    continue;// go ptr_cur.callback, ptr_new.callback
                }

                if (b_read_ok) {
                    ptr_cur->set_result(cqitem_dev::result_success, v_rx, L"SUCCESS");
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__+1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%s)%u:%u: _process_only_rx : complete.success.\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                ptr_cur->get_session_number(),
                                ptr_cur->get_uid()
                            );
                        }
                    }
#endif
                }
                else {
                    ptr_cur->set_result(cqitem_dev::result_error, v_rx, L"ERROR-_process_only_rx()");
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__+1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%s) _process_only_rx : complete.error.\n", _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
                        }
                    }
#endif
                }
            } while (false);//main do-while(false)

            // notify 는 항상 실행하고 있던 것 부터하고, 신규는 그 나중에 알림을 한다.
            // notification AREA
            do{
                if (!ptr_cur) {
                    continue;
                }
                if (!ptr_cur->is_complete()) {
                    continue;
                }
                auto cb_result = ptr_cur->run_callback_for_packet();
                if (!cb_result.first) {
                    //callback 이 false 를 return 하면, rx 를 다시 시도.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__ + 1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%ls)%u:%u: cur req(%ls).reset_result() for re-reading.\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                ptr_cur->get_session_number(),
                                ptr_cur->get_uid(),
                                ptr_cur->get_request_type_by_string().c_str()
                            );
                        }
                    }
#endif
                    ptr_cur->reset_result();
                    continue;
                }

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                    if (m_n_debug_line != (__LINE__ + 1)) {
                        m_n_debug_line = __LINE__;
                        ATLTRACE(L" =======(%ls)%u:%u: cur req(%ls).callback.complete(ptr_cur will be reset)\n",
                            _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                            ptr_cur->get_session_number(),
                            ptr_cur->get_uid(),
                            ptr_cur->get_request_type_by_string().c_str()
                        );
                    }
                }
#endif
                // 아직 reading 기다리는 모든 request 의 uid 얻음. 
                // 동기 명령의 경우는 v_need_next_reading 는 empty.
                auto v_need_next_reading = cb_result.second; 

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                    if (m_n_debug_line != (__LINE__ + 1)) {
                        m_n_debug_line = __LINE__;

                        std::wstring __s_waiter;
                        for (auto __n_w : v_need_next_reading) {
                            __s_waiter += std::to_wstring(__n_w);
                            if (&__n_w != &v_need_next_reading.back()) { // Check if it's not the last element
                                __s_waiter += L",";
                            }
                        }//end for

                        ATLTRACE(L" =======(%ls)%u:%u: cur req(%ls) - waiter : %ls)\n",
                            _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                            ptr_cur->get_session_number(),
                            ptr_cur->get_uid(),
                            ptr_cur->get_request_type_by_string().c_str(),
                            __s_waiter.c_str()
                        );
                    }
                }
#endif
                std::vector<size_t>::iterator it_next_read = std::find(v_need_next_reading.begin(), v_need_next_reading.end(), ptr_cur->get_uid());
                if (it_next_read != v_need_next_reading.end()) {
                    // ptr_cur 은 아직 완료 안됌.
                    ptr_cur->reset_result();
                    continue;// 계속 reading
                }
                
                if (!v_need_next_reading.empty()) {
                    ptr_cur.reset(); // 완료되어서 메모리에서 삭제.
                    if (dq_ptr_cur_old.empty()) {
                        continue;
                    }

                    ptr_cur = dq_ptr_cur_old.front();
                    dq_ptr_cur_old.pop_front();
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__ + 1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%ls) continue old cur req(%ls:%u:%u) -> cur\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                ptr_cur->get_request_type_by_string().c_str(),
                                ptr_cur->get_session_number(),
                                ptr_cur->get_uid()
                            );
                        }
                    }
#endif
                    continue;
                }
                
                dq_ptr_cur_old.clear();
                ptr_cur.reset(); // 완료되어서 메모리에서 삭제.
            } while (false);

            if (ptr_new) {
                if (ptr_new->is_complete()) {
                    ptr_new->run_callback_for_packet();//impossible re-read
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
                    if (_vhid_info::get_type_from_compositive_map_index(m_n_dev) == type_bm_dev_lpu200_ibutton) {
                        if (m_n_debug_line != (__LINE__ + 1)) {
                            m_n_debug_line = __LINE__;
                            ATLTRACE(L" =======(%ls)%u:%u: new req(%ls).callback.end\n",
                                _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str(),
                                ptr_new->get_session_number(), ptr_new->get_uid(),
                                ptr_new->get_request_type_by_string().c_str()
                            );
                        }
                    }
#endif
                }
                // 새로운 명령은 complete 면, callback 실행 결과와 무관하게 삭제.
                ptr_new.reset();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_io_check_interval_mmsec));

        }// the end of main while

        //terminate rx thread.
        if (ptr_th_rx) {
            if (ptr_th_rx->joinable()) {
                ptr_th_rx->join();
            }
            ptr_th_rx.reset();
        }

        std::wstringstream ss;
        ss << std::this_thread::get_id();
        std::wstring s_id = ss.str();
        //_mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());<<dead lock

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
        ATLTRACE(L" =======Exit clibhid_dev::_worker(0x%08x-%ls).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
    }

}