#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include <mp_type.h>
#include <hid/mp_clibhid_dev_info.h>
#include <mp_cqueue.h>
#include <mp_cqitem_dev.h>
#include <mp_clog.h>

namespace _mp {
    class clibhid_dev
    {
    public:
        typedef std::shared_ptr<clibhid_dev> type_ptr;
        typedef std::weak_ptr<clibhid_dev> type_wptr;
        typedef std::unique_ptr<clibhid_dev> type_uptr;
        typedef _mp::cqueue<_mp::cqitem_dev::type_ptr> type_q_ptr;

    private:
        enum {
            _const_dev_io_check_interval_mmsec = 10
        };

    public:
        clibhid_dev(const clibhid_dev_info & info) :
            m_p_dev(nullptr), m_b_run_th_worker(false), m_dev_info(info)
        {
            hid_device* p_dev = hid_open_path(info.get_path_by_string().c_str());
            if (p_dev) {
                // Set the hid_read() function to be non-blocking.
                if (hid_set_nonblocking(p_dev, 1) == 0) {
                    m_p_dev = p_dev;
                    m_b_run_th_worker = true;
                    m_ptr_th_worker = std::shared_ptr<std::thread>(new std::thread(clibhid_dev::_worker, std::ref(*this)));
                }
                else {
                    _close();
                }
            }

        }
        ~clibhid_dev()
        {
            if (m_ptr_th_worker) {
                m_b_run_th_worker = false;
                if (m_ptr_th_worker->joinable()) {
                    m_ptr_th_worker->join();
                }
            }

            _close();
        }

        bool is_open()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_p_dev)
                return true;
            else
                return false;
        }

        std::vector<unsigned char> get_report_desciptor()
        {
            std::vector<unsigned char> v(4096,0);//HID_API_MAX_REPORT_DESCRIPTOR_SIZE

            do {
                std::lock_guard<std::mutex> lock(m_mutex);

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

        bool _close()
        {
            bool b_result = false;

            do {
                if (m_p_dev == nullptr) {
                    continue;
                }
                hid_close(m_p_dev);
                m_p_dev = nullptr;
                //
                b_result = true;
            } while (false);
            return b_result;
        }

        /**
        * @brief Write an Output report to a HID device.
        * the fist byte must be report ID.
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
                    if (n_result < 0) {
                        b_result = false;
                        break;
                    }
                    if (n_result != v_report.size()) {
                        b_result = false;
                        break;
                    }
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
            int n_offset = 0;

            do {
                b_continue = false;

                size_t n_report = m_dev_info.get_size_in_report();
                if (n_report <= 0) {
                    continue;
                }

                v_rx.resize(n_report, 0);

                int n_result = hid_read(m_p_dev, &v_rx[n_offset], v_rx.size() - n_offset);
                if (n_result == 0 && n_offset == 0) {
                    v_rx.resize(0);
                    b_result = true;
                    continue;//exit none data
                }

                if (n_result < 0) {
                    v_rx.resize(0);
                    continue;//error exit
                }

                if (n_result > (v_rx.size() - n_offset)) {
                    continue;//error exit
                }
                if (n_result < (v_rx.size() - n_offset)) {
                    //need more 
                    n_offset += n_result;
                    b_continue = true;
                    continue;
                }

                b_result = true;
            } while (b_continue);

            return b_result;
        }

    private:
        /**
        * device io worker
        */
        static void _worker(clibhid_dev& obj)
        {
            cqitem_dev::type_ptr ptr_new, ptr_cur;
            _mp::type_v_buffer v_rx;
            int n_size_in_report(obj.m_dev_info.get_size_in_report());

            while (obj.m_b_run_th_worker) {
                v_rx.resize(n_size_in_report, 0);//reset rx buffer

                do {
                    std::lock_guard<std::mutex> lock(obj.m_mutex);
                    if (!ptr_cur) {
                        //pumpping mode....... none processing request.
                        if (!obj.m_q_ptr.try_pop(ptr_new)|| !ptr_new) {
                            obj._read(v_rx);//pumpping.
                            continue;
                        }

                        //processing new request.
                        if (!ptr_new->is_empty_tx()) {
                            if (!obj._write(ptr_new->get_tx())) {//ERROR TX
                                clog::get_instance().trace(L"[E] - %ls - %ls - obj._write().\n", __WFILE__, __WFUNCTION__);
                                ptr_new->set_result(cqitem_dev::result_error,type_v_buffer(0),L"TX");
                                continue;
                            }
                        }
                        if (!ptr_new->is_need_rx()) {
                            //map be cancel request.......
                            // none TX & no need rx!
                            ptr_new->set_result(cqitem_dev::result_success,type_v_buffer(0));
                            continue;
                        }
                        //need response....... re-read at next loop.
                        ptr_cur = ptr_new;
                        ptr_new.reset();
                        continue;
                    }

                    //////////////////////////////////////
                    // here current request exist.......
                    //ready for rx mode.
                    if (!obj._read(v_rx)) {//ERROR RX
                        clog::get_instance().trace(L"[E] - %ls - %ls - obj._read().\n", __WFILE__, __WFUNCTION__);
                        ptr_cur->set_result(cqitem_dev::result_error,type_v_buffer(0), L"RX");
                        continue;
                    }

                    if (v_rx.size() != 0) {
                        //RX OK.
                        ptr_cur->set_result(cqitem_dev::result_success, v_rx);
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
                    if (!ptr_new->is_empty_tx()) {
                        if (!obj._write(ptr_new->get_tx())) {//ERROR TX
                            ptr_new->set_result(cqitem_dev::result_error, type_v_buffer(0), L"TX");
                            continue;
                        }
                    }
                    if (!ptr_new->is_need_rx()) {
                        ptr_new->set_result(cqitem_dev::result_success, type_v_buffer(0));
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
            }//end while
        }

    private:
        std::mutex m_mutex;
        hid_device* m_p_dev;
        clibhid_dev_info m_dev_info;

        std::shared_ptr<std::thread> m_ptr_th_worker;
        std::atomic<bool> m_b_run_th_worker;
        _mp::clibhid_dev::type_q_ptr m_q_ptr;//thread safty

    private:
        clibhid_dev();
        clibhid_dev(const clibhid_dev&);
        clibhid_dev& operator=(const clibhid_dev&);

    };//the end of class clibhid_dev

}