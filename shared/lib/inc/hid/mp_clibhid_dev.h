#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_cqitem_dev.h>
#include <mp_clog.h>

#include <hid/mp_clibhid_dev_info.h>
#include <hid/_vhid_api_briage.h>

namespace _mp {
    /**
	* @brief HID device class
	* support async io by thread. - _worker()
    * support rx pumpping for linux by thread. - _worker_rx()
    */
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
            _const_dev_io_check_interval_mmsec = 3//50
        };
        enum {
            _const_dev_rx_check_interval_mmsec = 4// min is 1 msec in full speed hid device. but one report needs 4 in-packets.
        };
        enum {
            _const_dev_rx_recover_interval_usec = 1000
        };
        enum {
            _const_dev_rx_flush_interval_mmsec = 1
        };

        enum {
            _const_lost_packet_retry_counter = 300  // times of clibhid_dev::_const_dev_rx_check_interval_mmsec
        };

    public:

        /**
		* @brief constructor.
		* when the instance is created, the device is opened. and the worker thread is started.(receiving is started automatically.)
		* @param info - device information
		* @param p_vhid_api_briage - virtual HID API bridge
        */
        clibhid_dev(const clibhid_dev_info& info, _hid_api_briage *p_hid_api_briage);
        ~clibhid_dev();

        bool is_detect_replugin();

        bool is_open() const;

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
        * @brief called by _worker()
        */
        std::pair<bool, bool> _pump();

        /**
		* @brief clear all queue item of the received data.
        */
        void _clear_rx_q_with_lock();

        /**
        * @brief Write an Output report to a HID device.
        * the fist byte must be report ID.(automatic add 
        * @param v_tx - this will be sent by split-send as in-report size
        * @param b_req_is_tx_rx_pair - true(after done tx, need to receive response.)
        */
        bool _write_with_lock(const std::vector<unsigned char>& v_tx, bool b_req_is_tx_rx_pair);

        /**
        * @brief read by report unit
        * @return first true - rx success, second true - mayne device is plug out and in at error status.  
        */
        std::pair<bool, bool> _read_using_thread(std::vector<unsigned char>& v_rx);


        /**
        * @brief process new request and set result but Don't notify.
        * 
        * @param ptr_req - new request.
        * 
        * @return first - true(the new request is processed comppletely with success or error), false - need more time to process it.
        * 
        *   second - true(the new request is cancel request!, In this case, first must be true), false(else)
        */
        std::pair<bool,bool> _process_new_request_and_set_result(cqitem_dev::type_ptr& ptr_req);

        bool _process_only_tx(cqitem_dev::type_ptr& ptr_req);

        bool _process_cancel(cqitem_dev::type_ptr& ptr_req);

        /**
        * @brief receving data
        * @return
        *   first : processing result( true - none error ),
        *   second : processing complete(true), need more processing(false. in this case. first must be true)
        *   third : rx data from device.
        */
        std::tuple<bool, bool, _mp::type_v_buffer> _process_only_rx(cqitem_dev::type_ptr& ptr_req);

        /**
        * @brief transmit and receving data
        * @return
        *   first : processing result( true - none error ),
        *   second : processing complete(true), need more processing(false. in this case. first must be true)
        *   third : rx data from device.
        */
        std::tuple<bool, bool, _mp::type_v_buffer> _process_tx_rx(cqitem_dev::type_ptr& ptr_req);

    protected:
        /**
        * @brief receiving worker thread. this thread will be construct & destruct on _worker().
        * from device, receving a data by in-report size unit. and it enqueue to m_q_ptr.
        */
        void _worker_rx();
        /**
        * @brief device io worker
        */
        void _worker();

    protected:
        _hid_api_briage* m_p_hid_api_briage;//virtual hidapi library instance

		int m_n_dev;//valid is zero or positive. map index of primitive or composite map.
        clibhid_dev_info m_dev_info;

		std::shared_ptr<std::thread> m_ptr_th_worker;//thread for io worker(_worker())
        std::atomic<bool> m_b_run_th_worker;
		_mp::clibhid_dev::type_q_ptr m_q_ptr;//for io worker(m_ptr_th_worker,_worker()  ), request queue

		_mp::clibhid_dev::_type_q_rx_ptr_v m_q_rx_ptr_v;//for rx worker, receive queue

		std::atomic<bool> m_b_detect_replugin;// error status. if true, need to remove this instance.

    private:
        clibhid_dev();
        clibhid_dev(const clibhid_dev&);
        clibhid_dev& operator=(const clibhid_dev&);

    };//the end of class clibhid_dev

}