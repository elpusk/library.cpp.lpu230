#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <atomic>
#include <functional>

#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_elpusk.h>
#include <mp_coperation.h>
#include <hid/mp_clibhid_dev.h>
//#include <chid_bridge.h>


namespace _mp{

    /**
	* @description
    * this class job.
	* single tone type, device manager.
    * 1. when manager detectes a plugin-out device,manager will create & remove device automatically.
	* 2. detect plug in/out device by worker thread. and notify to user by callback function.
    */
    class clibhid
    {
    public:
        typedef std::shared_ptr< clibhid > type_ptr;

        typedef std::pair< clibhid_dev::type_ptr, clibhid_dev_info::type_ptr> pair_ptrs;
        typedef std::map< std::string, clibhid::pair_ptrs > type_map_pair_ptrs;

        /**
        * callback funtion prototype
        * Don't use the member function in this callack function. If use, deadlock!! 
        * @parameter 1'st - inserted devices. 2'nd - removed devices. 3'rd - current device. 4'th - user parameter for callack
        */
        typedef	std::function<void(const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, void*)>	type_callback_pluginout;

    protected:
        enum {
            _const_default_dev_pluginout_check_interval_mmsec = 100
        };
    public:

        /**
        * @brief create singleton instance of clibhid class.(device manager)
        * @return the instance of clibhid class.
        */
        static clibhid& get_instance();

        /*
        * @brief initialize clibhid instance.
        * 
        *  처음 get_instance() 을 호출 후, 다른 member 를 호출 하기 전에 항상 이 함수를 호출행야 한다,
        *
        *   1. set default device filter to lpu237, lpu238, hidbootloader.
        *
        *   2. create _vhid_api_bridge instance.
        *
        *   3. get the current connected device list.
        *
        *   4. create & open clibhid_dev instatnce. (virtual device - using _vhid_api_bridge instance) and register these to map.
        *
        *   5. create & run device plug in/out thread.( clibhid::_worker_pluginout() )
        *
        * @param set_usb_filter - if this parameter is empty, default filter will be set to lpu237, lpu238, hidbootloader.
        * @return true - ini success
        */
        bool initialize(const _mp::type_set_usb_filter& set_usb_filter = _mp::type_set_usb_filter());

        /*
        * @brief initialize clibhid instance.
        * 
        *  처음 get_instance() 을 호출 후, 다른 member 를 호출 하기 전에 항상 이 함수를 호출행야 한다,
        * 
        *   1. set default device filter to lpu237, lpu238, hidbootloader.
        * 
        *   2. create _vhid_api_bridge instance.
        * 
        *   3. get the current connected device list.
        *
        *   4. create & open clibhid_dev instatnce. (virtual device - using _vhid_api_bridge instance) and register these to map.
        *
        *   5. create & run device plug in/out thread.( clibhid::_worker_pluginout() )
        * 
        * @param b_manual - true : 위 1~2 까지 실행, false : 위 1~5 까지 실행.
        * @prarm b_remove_all_zero_in_report - true : in report 값이 모드 0 이면, 그 수신 in-report 는 무시한다.
        * @return true - ini success 
        */
        bool initialize(bool b_manual);

        ~clibhid();

        bool is_ini() const;

		void set_usb_filter(const _mp::type_set_usb_filter& set_usb_filter);

        void set_dev_pluginout_check_interval(long long ll_mmsec);

        void set_dev_tx_by_api_check_interval(long long ll_mmsec);
        void set_dev_rx_by_api_in_rx_worker_check_interval(long long ll_mmsec);
        void set_dev_rx_q_check_interval(long long ll_mmsec);

        void set_callback_pluginout(clibhid::type_callback_pluginout cb, void* p_user);

        /**
        * @brief in manual mode, before calling this function, you must call update_dev_set_in_manual().
        * 
        *   in auto mode, _worker_pluginout thread will update m_set_cur_dev_info automatically.
        * 
        * @return m_set_cur_dev_info
        */
        clibhid_dev_info::type_set get_cur_device_set();

        clibhid_dev_info::type_set get_removed_device_set();

        clibhid_dev_info::type_set get_inserted_device_set();

        clibhid_dev::type_wptr get_device(const clibhid_dev_info& dev_info);
        clibhid_dev::type_wptr get_device(const std::string& s_path);
        clibhid_dev::type_wptr get_device(const std::wstring& sw_path);

        /**
        * @description - update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        * 
        *   ONLY for  in manual mode. 
        * 
        *   this function dose not create a device io instance,
        * 
        * @return - true - changed, false - not changed.
        */
        bool update_dev_set_in_manual();


        chid_bridge::type_ptr get_bridge();

        /**
        * @brief Consider the equipment to be removed
        */
        bool consider_to_be_removed(int n_vid, int n_pid, const std::wstring& s_pis);

        /**
        * @brief Cancel considering the equipment as removed
        */
        bool cancel_considering_dev_as_removed(int n_vid, int n_pid, const std::wstring& s_pis);
        
    protected:
        clibhid();

        clibhid_dev::type_wptr _get_device(const std::string& s_path);

        clibhid_dev::type_wptr _get_device(const clibhid_dev_info& dev_info);

        clibhid_dev::type_wptr _get_device(const std::wstring& sw_path);

    protected:

        /**
		* @brief detect device plug in & out. this function is for thread.
		*   thread will be started in constructor and stopped in destructor.
        */
        static void _worker_pluginout(clibhid& lib);

        /**
        * @description - update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        * ONLY for _worker_pluginout. and update_dev_set() in manual mode
        * @return - true - changed, false - not changed.
        */
        bool _update_dev_set();

        /**
		* @brief get current connected device list with hid bridge api and filter.
		*   used in _update_dev_set() and constructor.
        */
        clibhid_dev_info::type_set _get_device_set();

        /**
        * @brief for _worker_pluginout.
        *  create & remove  device instance of map.
        *   
        */
        void _update_device_map();

        /**
		* @brief initialize member.
        * 
		*   this is used in constructor only.
        */
        void _ini(const chid_bridge::type_ptr & ptr_bridge = chid_bridge::type_ptr());

    protected:
        bool m_b_manual;

        // virtual hidapi library instance
        //_hid_api_bridge::type_ptr m_ptr_hid_api_bridge;
        chid_bridge::type_ptr m_ptr_hid_api_bridge;

		void* m_p_user;// user parameter for callback(m_cb)
		clibhid::type_callback_pluginout m_cb; // callback function for plugin out.(change of device list)
        std::atomic_llong m_atll_dev_pluginout_check_interval_mmsec;

		bool m_b_ini;//true - ini, false - not ini of this manager
        std::mutex m_mutex;

        // this usb device must be considerated to plugout.
        // protected by m_mutex
        _mp::type_set_usb_id_pis m_set_usb_id_considerated_to_remove;

		// key : device path(primitive & compositive), value : first - device instance, second - device info instance
        clibhid::type_map_pair_ptrs m_map_pair_ptrs;

		std::shared_ptr<std::thread> m_ptr_th_pluginout; //thread for detect plug in/out device.
		std::atomic<bool> m_b_run_th_pluginout; //true - run, false - stop of pluginout thread.

        _mp::type_set_usb_filter m_set_usb_filter; //filtering device list.

        clibhid_dev_info::type_set m_set_cur_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_inserted_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_removed_dev_info;//this member will be changed after _update_dev_set ().

    private://don't call these functions.
        
        clibhid(const clibhid&);
        clibhid& operator= (const clibhid&);
    };
   
    
}