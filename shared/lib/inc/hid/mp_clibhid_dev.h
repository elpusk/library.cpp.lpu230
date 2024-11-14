#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_cqitem_dev.h>
#include <mp_clog.h>

#include <hid/mp_clibhid_dev_info.h>

namespace _mp {
    class clibhid_dev
    {
    public:
        typedef std::shared_ptr<clibhid_dev> type_ptr;
        typedef std::weak_ptr<clibhid_dev> type_wptr;
        typedef std::unique_ptr<clibhid_dev> type_uptr;
        typedef _mp::cqueue<_mp::cqitem_dev::type_ptr> type_q_ptr;

    protected:
        typedef _mp::cqueue<_mp::type_ptr_v_buffer> _type_q_rx_ptr_v;

    protected:
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
            const clibhid_dev_info& info,
            std::shared_ptr<std::mutex> ptr_mutex_hidpai
        );
        ~clibhid_dev();

        bool is_detect_replugin();

        bool is_open();

        std::vector<unsigned char> get_report_desciptor();


        /**
        * @brief Staring Write an Output report to a HID device.
        * the fist byte must be report ID. start write operation.
        */
        void start_write(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb);

        /**
        * @brief Staring Read an Input report from a HID device.
        * Input reports are returned
        * to the host through the INTERRUPT IN endpoint. The first byte will
        * contain the Report number if the device uses numbered reports.
        *  start read operation.
        */
        void start_read(cqitem_dev::type_cb cb, void* p_user_for_cb);

        /**
        * @brief Staring Write an Output report to a HID device. and read operation
        * the fist byte must be report ID. start write operation.
        */
        void start_write_read(const std::vector<unsigned char>& v_tx, cqitem_dev::type_cb cb, void* p_user_for_cb);

        /**
        * @brief Staring cancel operation.
        */
        void start_cancel(cqitem_dev::type_cb cb, void* p_user_for_cb);

    protected:

        /**
        * called by _worker_using_thread()
        */
        std::pair<bool, bool> _pump();
        /**
        * called by _worker_using_thread()
        * @return first( true - need more processing, false - complete transaction with or without error.)
        * @return second( true - request is tx, rx pair, false - else), if first is complete, second must be false.
        */
        std::pair<bool, bool> _tx_and_check_need_more_processing_with_set_result(cqitem_dev::type_ptr& ptr_req, std::mutex& mutex_rx);

        void _clear_rx_q();

        /**
        * @brief Write an Output report to a HID device.
        * the fist byte must be report ID.(automatic add 
        * @param v_tx - this will be sent by split-send as in-report size
        */
        bool _write(const std::vector<unsigned char>& v_tx);

        /**
        * @brief Read an Input report from a HID device.
        * Input reports are returned
        * to the host through the INTERRUPT IN endpoint. The first byte will
        * contain the Report number if the device uses numbered reports.
        * @param the size of v_rx will be set in report size after receiving successfully.
        * @return
        * If no packet was available to be reads, this function returns true with size is zero.
        */
        bool _read(std::vector<unsigned char>& v_rx);

        /**
        * read by report unit
        * @return first true - rx success, second true - mayne device is plug out and in at error status.  
        */
        std::pair<bool, bool> _read_using_thread(std::vector<unsigned char>& v_rx);

    protected:
        static void _worker_rx(clibhid_dev& obj, std::mutex& mutex_rx);
        /**
        * device io worker
        */
        static void _worker_using_thread(clibhid_dev& obj);

    protected:
        std::shared_ptr<std::mutex> m_ptr_mutex_hidpai;
        int m_n_dev;//valid is zero or positive
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