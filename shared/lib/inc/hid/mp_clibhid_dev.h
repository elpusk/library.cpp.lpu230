#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_cqitem_dev.h>
#include <mp_clog.h>

#include <usb/mp_clibusb.h>
#include <hid/mp_clibhid_dev_info.h>

#ifndef _WIN32
#include <pthread.h>
#endif

namespace _mp {
    class clibhid_dev
    {
    public:
        typedef std::shared_ptr<clibhid_dev> type_ptr;
        typedef std::weak_ptr<clibhid_dev> type_wptr;
        typedef std::unique_ptr<clibhid_dev> type_uptr;
        typedef _mp::cqueue<_mp::cqitem_dev::type_ptr> type_q_ptr;

    private:
        typedef _mp::cqueue<_mp::type_ptr_v_buffer> _type_q_rx_ptr_v;

    private:
        enum {
            _const_dev_io_check_interval_mmsec = 50
        };
        enum {
            _const_dev_rx_check_interval_mmsec = 50
        };
        enum {
            _const_dev_rx_recover_interval_usec = 1000
        };
        enum {
            _const_dev_rx_flush_interval_mmsec = 2
        };

        enum {
            _const_lost_packet_retry_counter = 3
        };

    public:
        clibhid_dev(
            const clibhid_dev_info & info,
            std::shared_ptr<std::mutex> ptr_mutex_hidpai
        ) :
            m_p_dev(nullptr), 
            m_b_run_th_worker(false),
            m_dev_info(info),
            m_b_detect_replugin(false),
            m_ptr_mutex_hidpai(ptr_mutex_hidpai)
        {
            hid_device* p_dev = hid_open_path(info.get_path_by_string().c_str());
            if (p_dev) {

                int n_none_blocking = 1;//enable non-blocking.

                if (hid_set_nonblocking(p_dev, n_none_blocking) == 0) {
                    m_p_dev = p_dev;
                    m_b_run_th_worker = true;
                    m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(clibhid_dev::_worker_using_thread, std::ref(*this)));
                }
                else {
                    hid_close(m_p_dev);
                    m_p_dev = nullptr;
                }
            }

        }
        ~clibhid_dev()
        {
            m_b_run_th_worker = false;
            hid_close(m_p_dev);

            if (m_ptr_th_worker) {
                
                if (m_ptr_th_worker->joinable()) {
                    m_ptr_th_worker->join();
                }
            }
            m_p_dev = nullptr;
        }

        bool is_detect_replugin()
        {
            return m_b_detect_replugin;
        }

        bool is_open()
        {
            std::lock_guard<std::mutex> lock(*m_ptr_mutex_hidpai);
            if (m_p_dev)
                return true;
            else
                return false;
        }

        std::vector<unsigned char> get_report_desciptor()
        {
            std::vector<unsigned char> v(4096,0);//HID_API_MAX_REPORT_DESCRIPTOR_SIZE

            do {
                std::lock_guard<std::mutex> lock(*m_ptr_mutex_hidpai);

                int n_result = 0;
                n_result = hid_get_report_descriptor(m_p_dev, &v[0], v.size());
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
        void start_write(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb)
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
        void start_read(cqitem_dev::type_cb cb, void* p_user_for_cb)
        {
            cqitem_dev::type_ptr ptr(new cqitem_dev(_mp::type_v_buffer(0), true, cb, p_user_for_cb));
            m_q_ptr.push(ptr);
        }

        /**
        * @brief Staring Write an Output report to a HID device. and read operation
        * the fist byte must be report ID. start write operation.
        */
        void start_write_read(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb)
        {
            cqitem_dev::type_ptr ptr(new cqitem_dev(v_tx, true, cb, p_user_for_cb));
            m_q_ptr.push(ptr);
        }

        /**
        * @brief Staring cancel operation.
        */
        void start_cancel(cqitem_dev::type_cb cb, void* p_user_for_cb)
        {
            //cancel -> tx is none, no need rx!
            cqitem_dev::type_ptr ptr(new cqitem_dev(_mp::type_v_buffer(0), false, cb, p_user_for_cb));
            m_q_ptr.push(ptr);
        }

    private:

        /**
        * called by _worker_using_thread()
        */
        std::pair<bool, bool> _pump()
        {
            bool b_read_is_normal = true;
            bool b_expected_replugin = false;

            int n_size_in_report(m_dev_info.get_size_in_report());
            _mp::type_v_buffer v_rx(n_size_in_report, 0);

            std::tie(b_read_is_normal, b_expected_replugin) = _read_using_thread(v_rx);//pumpping.
            m_q_rx_ptr_v.clear();//pumpping.

            if (!b_read_is_normal) {
                clog::get_instance().trace(L"[W] - %ls - obj._read_using_thread() in pump.\n", __WFUNCTION__);
                clog::get_instance().log_fmt(L"[W] - %ls - obj._read_using_thread() in pump.\n", __WFUNCTION__);
                if (b_expected_replugin) {
                    m_b_detect_replugin = true;//need this device instance removed
                }
            }
            return std::make_pair(b_read_is_normal, b_expected_replugin);
        }
        /**
        * called by _worker_using_thread()
        * @return first( true - need more processing, false - complete transaction with or without error.)
        * @return second( true - request is tx, rx pair, false - else), if first is complete, second must be false.
        */
        std::pair<bool,bool> _tx_and_check_need_more_processing_with_set_result(cqitem_dev::type_ptr & ptr_req, std::mutex& mutex_rx)
        {
            bool b_need_more_processing(false);
            bool b_req_is_tx_rx_pair(false);

            do {

                do {
                    std::lock_guard<std::mutex> lock(*m_ptr_mutex_hidpai);//guard write
                    _clear_rx_q();//flush rx q
                } while (false);

                if (!ptr_req->is_empty_tx()) {
                    std::lock_guard<std::mutex> lock(*m_ptr_mutex_hidpai);//guard write
                    //std::lock_guard<std::mutex> lock(mutex_rx);//guard write
                    if (!_write(ptr_req->get_tx())) {//ERROR TX
                        ptr_req->set_result(cqitem_dev::result_error, type_v_buffer(0), L"TX");
                        continue;
                    }
                }
                if (!ptr_req->is_need_rx()) {
                    //map be cancel request.......
                    // none TX & no need rx!
                    ptr_req->set_result(cqitem_dev::result_success, type_v_buffer(0));
                    continue;
                }

                b_need_more_processing = true;
            } while (false);

            if (!ptr_req->is_empty_tx() && ptr_req->is_need_rx()) {
                b_req_is_tx_rx_pair = true;
            }
            if (!b_need_more_processing) {
                b_req_is_tx_rx_pair = false;
            }
            return std::make_pair(b_need_more_processing,b_req_is_tx_rx_pair);
        }

        void _clear_rx_q()
        {
            int n_flush = 0;
            _mp::type_ptr_v_buffer ptr_flush_buffer;

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
        */
        bool _write(const std::vector<unsigned char>& v_tx)
        {
            bool b_result(false);
            size_t n_start = 0;
            size_t n_offset = 0;

            do {
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

                for (size_t i = 0; i < n_loop; i++) {
                    std::fill(v_report.begin(), v_report.end(), 0);
                    if ((v_tx.size() - n_offset) >= (v_report.size() - n_start)) {
                        std::copy(v_tx.begin() + n_offset, v_tx.begin() + n_offset+ (v_report.size() - n_start), v_report.begin() + n_start);
                    }
                    else {
                        std::copy(v_tx.begin() + n_offset, v_tx.end(), v_report.begin() + n_start);
                    }
                
                    n_result = hid_write(m_p_dev, &v_report[0], v_report.size());
                    _mp::clog::get_instance().log_data_in_debug_mode(v_report, L"v_report = ", L"\n");
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
        * @brief Read an Input report from a HID device.
        * Input reports are returned
        * to the host through the INTERRUPT IN endpoint. The first byte will
        * contain the Report number if the device uses numbered reports.
        * @param the size of v_rx will be set in report size after receiving successfully.
        * @return
        * If no packet was available to be reads, this function returns true with size is zero.
        */
        bool _read(std::vector<unsigned char>& v_rx)
        {
            bool b_result(false);
            bool b_continue(false);
#ifdef _WIN32            
            int n_start_offset = 0;//none for report ID
#else
            int n_start_offset = 0;//for report ID
#endif
            int n_offset = n_start_offset;

            do {
                b_continue = false;

                size_t n_report = m_dev_info.get_size_in_report();
                if (n_report <= 0) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] get_size_in_report = %u.\n", n_report);
                    continue;
                }

                v_rx.resize(n_report+ n_start_offset, 0);

                //int n_result = hid_read(m_p_dev, &v_rx[n_offset], v_rx.size() - n_offset);
                int n_result = hid_read_timeout(m_p_dev, &v_rx[n_offset], v_rx.size() - n_offset, clibhid_dev::_const_dev_rx_check_interval_mmsec);
                if (n_result == 0) {
                    if (n_offset == n_start_offset) {
                        //_mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] (n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                        v_rx.resize(0);
                        b_result = true;
                        continue;//exit none data
                    }
                    if (n_offset >= v_rx.size()) {
                        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 1(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                        b_result = true;
                        continue;//exit complete
                    }
                    else {
                        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 2(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                        //std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_rx_check_interval_mmsec));
                        b_continue = true;
                        continue;//retry rx
                    }
                }

                if (n_result < 0) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[E] 3(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                    v_rx.resize(0);
                    continue;//error exit
                }

                //here! n_result > 0
                n_offset += n_result;
                //
                if (n_offset > v_rx.size()) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 4(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                    continue;//error exit
                }
                if (n_offset < v_rx.size()) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 5(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                    //need more 
                    //std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_rx_check_interval_mmsec));
                    b_continue = true;
                    continue;
                }
                //here! n_result == v_rx.size()
                _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] 6(n_result : n_offset : v_rx.size()) = (%d,%d,%u).\n", n_result, n_offset, v_rx.size());
                // complete read
                b_result = true;
            } while (b_continue);

            return b_result;
        }

        /**
        * read by report unit
        * @return first true - rx success, second true - mayne device is plug out and in at error status.  
        */
        std::pair<bool,bool> _read_using_thread(std::vector<unsigned char>& v_rx)
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

    private:
        static void _worker_rx(clibhid_dev& obj,std::mutex & mutex_rx)
        {
            size_t n_report = obj.m_dev_info.get_size_in_report();
            if (n_report <= 0) {
                return;
            }

            bool b_need_deep_recover = false;

            _mp::type_v_buffer v_rx(n_report, 0);

            while (obj.m_b_run_th_worker) {

                if (b_need_deep_recover) {
                    std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                    continue;//recover is impossible!
                }

                v_rx.assign(v_rx.size(), 0);//reset contents
                int n_result = 0;

                int n_offset(0);
                int n_max_try = 3;
                int n_retry = n_max_try;

                do {
                    std::lock_guard<std::mutex> lock(*obj.m_ptr_mutex_hidpai);//read guard
                    //std::lock_guard<std::mutex> lock(mutex_rx);//read guard

                    n_result = hid_read(obj.m_p_dev, &v_rx[n_offset], v_rx.size()- n_offset);//wait block or not by initialization
                    if (n_result == 0) {
                        if (n_offset == 0) {
                            break;//re-read transaction
                        }

                        //already transaction is started!!!
                        --n_retry;
                        if (n_retry <= 0) {
                            //lost a packet
                            obj.m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(0));
                            _mp::clog::get_instance().log_fmt(L"[E] %ls : lost packet\n", __WFUNCTION__);
                            break;//restart transaction
                        }
                        std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                        continue;//retry
                    }
                    if (n_result < 0) {
                        // warning
                        b_need_deep_recover = true;//this case - recover is impossible!

                        std::wstring s_error(hid_error(obj.m_p_dev));
                        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] rx return = %d. - %ls.\n", n_result, s_error.c_str());

                        obj.m_q_rx_ptr_v.push(_mp::type_ptr_v_buffer());//indicate error of device usb io.
                        break;
                    }
                    //here n_result > 0 recevied some data.
                    n_offset += n_result;
                    if (n_offset >= v_rx.size()) {
                        //one report complete.
                        obj.m_q_rx_ptr_v.push(std::make_shared<_mp::type_v_buffer>(v_rx));
                        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[H] v_rx.size() = %u.\n", v_rx.size());
                        _mp::clog::get_instance().log_data_in_debug_mode(v_rx, std::wstring(), L"\n");
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
                    n_retry = n_max_try;
                } while (n_offset< v_rx.size());

                // for reducing CPU usage rates.
                std::this_thread::sleep_for(std::chrono::microseconds(clibhid_dev::_const_dev_rx_recover_interval_usec));
            }//end while

            std::wstringstream ss;
            ss << std::this_thread::get_id();
            std::wstring s_id = ss.str();
            _mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());
        }
        /**
        * device io worker
        */
        static void _worker_using_thread(clibhid_dev& obj)
        {
            cqitem_dev::type_ptr ptr_new, ptr_cur;
            int n_size_in_report(obj.m_dev_info.get_size_in_report());
            _mp::type_v_buffer v_rx(n_size_in_report, 0);
            std::mutex mutex_rx;

            std::shared_ptr<std::thread> ptr_th_rx(new std::thread(clibhid_dev::_worker_rx, std::ref(obj),std::ref(mutex_rx)));

#ifndef _WIN32
            //set priority level of _worker_rx()
            int n_priority = 90;// 10;
            int n_policy = SCHED_FIFO;//SCHED_RR
            pthread_t nativeHandle = ptr_th_rx->native_handle();
            sched_param sch_params;
            sch_params.sched_priority = n_priority;
            if (pthread_setschedparam(nativeHandle, n_policy, &sch_params)) {//NEED root 
                clog::get_instance().trace(L"[E] - %ls - Failed to set thread priority.\n", __WFUNCTION__);
                clog::get_instance().log_fmt(L"[E] - %ls - Failed to set thread priority.\n", __WFUNCTION__);
            }
#endif

            bool b_read_is_normal = true;
            bool b_expected_replugin = false;

            bool b_need_more_processing(false);
            bool b_req_is_tx_rx_pair(false);

            while (obj.m_b_run_th_worker) {

                do {
                    if (!ptr_cur) {
                        //pumpping mode....... none processing request.
                        if (!obj.m_q_ptr.try_pop(ptr_new) || !ptr_new) {
                            std::tie(b_read_is_normal, b_expected_replugin) = obj._pump();//pumpping.
                            continue;
                        }

                        //processing new request.
                        std::tie(b_need_more_processing, b_req_is_tx_rx_pair) = obj._tx_and_check_need_more_processing_with_set_result(ptr_new,mutex_rx);
                        if (!b_need_more_processing) {
                            continue;//complete or error
                        }

                        if (!b_read_is_normal) {
                            // new request but read is error status -_-;;
                            b_req_is_tx_rx_pair = false;
                            clog::get_instance().trace(L"[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                            clog::get_instance().log_fmt(L"[E] - %ls - new request but read is error status.\n", __WFUNCTION__);
                            ptr_new->set_result(cqitem_dev::result_error, type_v_buffer(0), L"RX");
                            continue;
                        }

                        //need response....... re-read at next loop.
                        ptr_cur = ptr_new;
                        ptr_new.reset();
                    }

                    //////////////////////////////////////
                    // here current request exist.......
                    // ready for rx mode.
                    std::tie(b_read_is_normal, b_expected_replugin) = obj._read_using_thread(v_rx);
                    if (!b_read_is_normal) {//ERROR RX
                        b_req_is_tx_rx_pair = false;
                        clog::get_instance().trace(L"[E] - %ls - obj._read_using_thread().\n", __WFUNCTION__);
                        clog::get_instance().log_fmt(L"[E] - %ls - obj._read_using_thread().\n", __WFUNCTION__);
                        ptr_cur->set_result(cqitem_dev::result_error, type_v_buffer(0), L"RX");

                        if (b_expected_replugin) {
                            obj.m_b_detect_replugin = true;//need this device instance removed
                        }
                        continue;
                    }

                    if (v_rx.size() != 0) {
                        //RX OK.
                        if (b_req_is_tx_rx_pair) {
                            if(v_rx[0] != 'R'){
                                //may be response is msr or ibutton data.
                                //therefore , you must pass this response. 
                                clog::get_instance().trace(L"[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                                clog::get_instance().log_fmt(L"[W] - %ls - the missed response is passed.\n", __WFUNCTION__);
                                continue; //and need re-read.
                            }
                        }
                        ptr_cur->set_result(cqitem_dev::result_success, v_rx);
                        b_req_is_tx_rx_pair = false;
                        continue;
                    }

                    //need more waits
                    if (!obj.m_q_ptr.try_pop(ptr_new) || !ptr_new) {
                        continue;//No new request
                    }

                    //the current reuqest canceled.
                    ptr_cur->set_result(cqitem_dev::result_cancel, type_v_buffer(0), L"CANCELLED BY ANOTHER REQ");
                    ptr_cur->run_callback();//impossible re-read
                    ptr_cur.reset();
                    //

                    //processing new request.
                    std::tie(b_need_more_processing, b_req_is_tx_rx_pair) = obj._tx_and_check_need_more_processing_with_set_result(ptr_new,mutex_rx);
                    if (!b_need_more_processing) {
                        continue;
                    }
                    //need response....... re-read at next loop.
                    ptr_cur = ptr_new;
                    ptr_new.reset();

                } while (false);//the end of lock_guard

                if (ptr_new) {
                    if (ptr_new->is_complete()) {
                        ptr_new->run_callback();//impossible re-read
                    }
                    ptr_new.reset();
                }
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

                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid_dev::_const_dev_io_check_interval_mmsec));
            }//end while - worker main loop

            if (ptr_th_rx) {
                if (ptr_th_rx->joinable()) {
                    ptr_th_rx->join();
                }
                ptr_th_rx.reset();
            }

            std::wstringstream ss;
            ss << std::this_thread::get_id();
            std::wstring s_id = ss.str();
            _mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());
        }

    private:
        std::shared_ptr<std::mutex> m_ptr_mutex_hidpai;
        hid_device* m_p_dev;
        clibhid_dev_info m_dev_info;

        std::shared_ptr<std::thread> m_ptr_th_worker;
        std::atomic<bool> m_b_run_th_worker;
        _mp::clibhid_dev::type_q_ptr m_q_ptr;//thread safty

        _mp::clibhid_dev::_type_q_rx_ptr_v m_q_rx_ptr_v;//for rx worker

        std::atomic<bool> m_b_detect_replugin;// critical error

    private:
        clibhid_dev();
        clibhid_dev(const clibhid_dev&);
        clibhid_dev& operator=(const clibhid_dev&);

    };//the end of class clibhid_dev

}