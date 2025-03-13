
#include <hid/mp_clibhid_dev.h>

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32
#ifdef _DEBUG
#include <atltrace.h>
#endif
#endif

namespace _mp {
    clibhid_dev::clibhid_dev(const clibhid_dev_info & info, _hid_api_briage* p_hid_api_briage) :
        m_n_dev(-1),
        m_b_run_th_worker(false),
        m_dev_info(info),
        m_b_detect_replugin(false),
        m_p_hid_api_briage(p_hid_api_briage)
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


    /**
    * @brief Staring Write an Output report to a HID device.
    * the fist byte must be report ID. start write operation.
    */
    void clibhid_dev::start_write(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb)
    {
        cqitem_dev::type_ptr ptr(new cqitem_dev(v_tx,false,cb,p_user_for_cb));
        m_q_ptr.push(ptr);
    }

    /**
    * @brief Staring Read an Input report from a HID device.
    * Input reports are returned
    * to the host through the INTERRUPT IN endpoint. The first byte will
    * contain the Report number if the device uses numbered reports.
    *  start read operation.
    */
    void clibhid_dev::start_read(cqitem_dev::type_cb cb, void* p_user_for_cb)
    {
        cqitem_dev::type_ptr ptr(new cqitem_dev(_mp::type_v_buffer(0), true, cb, p_user_for_cb));
        m_q_ptr.push(ptr);
    }

    /**
    * @brief Staring Write an Output report to a HID device. and read operation
    * the fist byte must be report ID. start write operation.
    */
    void clibhid_dev::start_write_read(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb)
    {
        cqitem_dev::type_ptr ptr_tx(new cqitem_dev(v_tx, true, cb, p_user_for_cb));
        m_q_ptr.push(ptr_tx);
    }

    /**
    * @brief Staring cancel operation.
    */
    void clibhid_dev::start_cancel(cqitem_dev::type_cb cb, void* p_user_for_cb)
    {
        //cancel -> tx is none, no need rx!
        cqitem_dev::type_ptr ptr(new cqitem_dev(_mp::type_v_buffer(0), false, cb, p_user_for_cb));
        m_q_ptr.push(ptr);
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
                    if (n_result == 0) {
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
                            
                    std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_rx_check_interval_mmsec));
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
                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_rx_check_interval_mmsec));
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

    std::pair<bool, bool> clibhid_dev::_process_new_request_and_set_result(cqitem_dev::type_ptr& ptr_req)
    {
        bool b_request_is_cancel(false);
        bool b_complete(true);
        bool b_result(false);
        _mp::type_v_buffer v_rx(0);


        do {
            switch (ptr_req->get_request_type()) {
            case cqitem_dev::req_only_tx:
                b_result = _process_only_tx(ptr_req);
                break;
            case cqitem_dev::req_tx_rx:
                std::tie(b_result,b_complete, v_rx) = _process_tx_rx(ptr_req);
                break;
            case cqitem_dev::req_only_rx:
                std::tie(b_result, b_complete, v_rx) = _process_only_rx(ptr_req);
                break;
            case cqitem_dev::req_cancel:
                b_request_is_cancel = true;
            default:
                b_result = _process_cancel(ptr_req);//result is ignored
                ptr_req->set_result(cqitem_dev::result_success, type_v_buffer(0), L"SUCCESS CANCEL REQ");
                continue;
            }//end switch

            if (b_complete) {
                if (b_result) {
                    ptr_req->set_result(cqitem_dev::result_success, v_rx, L"SUCCESS");
                }
                else {
                    ptr_req->set_result(cqitem_dev::result_error, v_rx, L"ERROR");
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

    /**
    * receving data
    * @return 
    *   first : processing result( true - none error ), 
    *   second : processing complete(true), need more processing(false. in this case. first must be true)
    *   third : rx data from device.
    */
    std::tuple<bool, bool, _mp::type_v_buffer> clibhid_dev::_process_only_rx(cqitem_dev::type_ptr& ptr_req)
    {
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
                    m_b_detect_replugin = true;//need this device instance removed
                }
                continue;
            }

            if (v_rx.size() == 0) {
                b_complete = false;
                continue;
            }

            //RX OK.
            if (ptr_req->get_request_type() == cqitem_dev::req_tx_rx) {
                if (v_rx[0] != 'R') {//lpu237 specific protocol
                    // very important code - fix miss-matching txrx protocol. 
                    // 펨웨어에서, msr 이나 ibutton 데이터를 보내려고, usb buffer 에 데이타를 쓰고 있는 때,
                    // tx 가 전송되면, api 는 tx 에 대한 응답으로 msr 이나 ibutton 데이터를 받을수 있다.
                    // 이러한 경우 프로토콜 미스로 문제가 생기므로, 이 msr 이나 ibutton 은 무시되어야 한다. 무조건 !
                    b_complete = false;
                    v_rx.resize(0);
                    //
                    clog::get_instance().trace(L"T[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                    clog::get_instance().log_fmt(L"[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                    //
                    continue; //and need re-read.
                }
            }

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

            std::tie(b_result, b_complete, v_rx) = _process_only_rx(ptr_req);

        } while (false);
        return std::make_tuple(b_result, b_complete,v_rx);
    }

    /**
    * @brief receiving worker thread. this thread will be construct & destruct on _worker().
    * from device, receving a data by in-report size unit. and it enqueue to m_q_ptr.
    */
    void clibhid_dev::_worker_rx()
    {
#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"start clibhid_dev::_worker_rx(0x%x).\n", m_n_dev);
#endif
#endif
        size_t n_report = m_dev_info.get_size_in_report();
        if (n_report <= 0) {
#ifdef _WIN32
#ifdef _DEBUG
            ATLTRACE(L"Exit[E] clibhid_dev::_worker_rx(0x%x).\n", m_n_dev);
#endif
#endif
            return;
        }

        bool b_need_deep_recover = false;

        _mp::type_v_buffer v_report_in(n_report, 0);

        while (m_b_run_th_worker) {

            if (b_need_deep_recover) {
                std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                continue;//recover is impossible!
            }

            v_report_in.assign(v_report_in.size(), 0);//reset contents
            int n_result = 0;

            int n_offset(0);
            int n_max_try = 50;//50msec
            int n_retry = n_max_try;

            do {
                if (!m_p_hid_api_briage) {
                    break;//nothing to do
                }

                // this code is deadlock!!!!
                n_result = m_p_hid_api_briage->api_read(m_n_dev, &v_report_in[n_offset], v_report_in.size() - n_offset, v_report_in.size());//wait block or not by initialization
                if (n_result == 0) {
                    if (n_offset == 0) {
                        //_mp::clog::get_instance().log_fmt(L"[W] %ls : .\n", __WFUNCTION__);
                        break;//re-read transaction
                    }

                    //already transaction is started!!!
                    --n_retry;
                    if (n_retry <= 0) {
                        //lost a packet
                        m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(0));
                        _mp::clog::get_instance().log_fmt(L"[E] %ls : lost packet - push zero size buffer.\n", __WFUNCTION__);
                        break;//restart transaction
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                    continue;//retry
                }
                if (n_result < 0) {
                    // warning
                    b_need_deep_recover = true;//this case - recover is impossible!

                    std::wstring s_error(m_p_hid_api_briage->api_error(m_n_dev));
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] rx return = %d. - %ls.\n", n_result, s_error.c_str());

                    m_q_rx_ptr_v.push(_mp::type_ptr_v_buffer());//indicate error of device usb io.
                    break;
                }
                //here n_result > 0 recevied some data.
                n_offset += n_result;
                if (n_offset >= v_report_in.size()) {
                    //one report complete.
                    m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(v_report_in));
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[H] v_rx.size() = %u.\n", v_report_in.size());
                    _mp::clog::get_instance().log_data_in_debug_mode(v_report_in, std::wstring(), L"\n");
                    break;
                }
                std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                n_retry = n_max_try;
            } while (n_offset< v_report_in.size());

            // for reducing CPU usage rates.
            std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
        }//end while

        std::wstringstream ss;
        ss << std::this_thread::get_id();
        std::wstring s_id = ss.str();
        //_mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str()); ,, dead lock

#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"Exit clibhid_dev::_worker_rx(0x%08x-%s).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
#endif
    }

    /**
    * device io worker
    */
    void clibhid_dev::_worker()
    {
#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"start clibhid_dev::_worker(0x%08x-%s).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
#endif

        cqitem_dev::type_ptr ptr_new, ptr_cur;
        int n_size_in_report(m_dev_info.get_size_in_report());
        _mp::type_v_buffer v_rx(n_size_in_report, 0);

		// thread for rx
        std::shared_ptr<std::thread> ptr_th_rx(new std::thread(&clibhid_dev::_worker_rx, this));

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
        bool b_read = true;
        bool b_expected_replugin = false;
        bool b_complete(true);
        bool b_request_is_cancel(false);

        while(m_b_run_th_worker){

            do {
                if (!m_q_ptr.try_pop(ptr_new)) {
                    // 새로운 명령 없음.
                    if (!ptr_cur) {
                        // 현재 작업 주인 것 없으면.
                        std::tie(b_read, b_expected_replugin) = _pump();//pumpping.
                        continue;
                    }

                    // 현재 작업 중인 것 있으면,
                }
                else {
                    // 새로운 명령이 들어오면. 
                    /*
                    if (ptr_cur) {
                        // 기존 명령 취소.
                        _process_cancel(ptr_cur);//result is ignored
                        ptr_cur->set_result(cqitem_dev::result_cancel, type_v_buffer(0), L"CANCELLED BY ANOTHER REQ");
                        // callback 의 return 값이 관계없이 현재 작업은 제거.
                        ptr_cur->run_callback();
                        ptr_cur.reset();
                    }
                    */

                    // pop된 새로운 명령의 경우.
                    if (!b_read) {
                        // 새로운 명령을 pop  했지만, 현재 rx 가 에러 상태이므로 새로운 명령은 바로 에러로 처리.
                        clog::get_instance().trace(L"T[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                        clog::get_instance().log_fmt(L"[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                        ptr_new->set_result(cqitem_dev::result_error, type_v_buffer(0), L"RX");
                        continue;
                    }

                    // 새로운 명령을 처리.
                    std::tie(b_complete,b_request_is_cancel) = _process_new_request_and_set_result(ptr_new);
                    if (b_request_is_cancel) {
                        // 이 경우 b_request_is_cancel 는 항상 true.
                        if (ptr_cur) {
                            // 현재 실행 중인 명령이 있으면, cancel request 에 의해 취소 되므로, 
                            // ptr_cur 의 결과를 설정한다.
                            ptr_cur->set_result(cqitem_dev::result_cancel, type_v_buffer(0), L"CANCELLED BY ANOTHER REQ");
                        }
                        continue;
                    }
                    if (b_complete) {
                        continue; // 새로운 명령은 처리됨.
                    }

                    // 새로운 명령 처리가 더 필요하므로, 현재 명령으로 설정.
                    ptr_cur = ptr_new;
                    ptr_new.reset();
                }

                // 현재 명령으로 설정된 명령을 위해 계속 수신 시도함.
                std::tie(b_read, b_complete, v_rx) = _process_only_rx(ptr_cur);
                if (!b_complete) {
#if defined(_WIN32) && defined(_DEBUG)
                    ATLTRACE(L" =======(%s) _process_only_rx.\n", _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
                    continue;
                }
                if (b_read) {
                    ptr_cur->set_result(cqitem_dev::result_success, v_rx, L"SUCCESS");
                }
                else {
                    ptr_cur->set_result(cqitem_dev::result_error, v_rx, L"ERROR-_process_only_rx()");
                }
            } while (false);//main do-while(false)

            // notify 는 항상 실행하고 있던 것 부터하고, 신규는 그 나중에 알림을 한다.
            if (ptr_cur) {
                if (ptr_cur->is_complete()) {
                    if (ptr_cur->run_callback()) {
                        ptr_cur.reset();
                    }
                    else {//re read more
                        //user want to read more operation.
                    }
                }
            }

            if (ptr_new) {
                if (ptr_new->is_complete()) {
                    ptr_new->run_callback();//impossible re-read
                }
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

#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"Exit clibhid_dev::_worker(0x%08x-%s).\n", m_n_dev, _vhid_info::get_type_wstring_from_compositive_map_index(m_n_dev).c_str());
#endif
#endif

    }

}