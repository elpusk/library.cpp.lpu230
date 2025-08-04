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
#include <hid/_vhid_api_briage.h>


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
            _const_dev_pluginout_check_interval_mmsec = 100
        };
    public:
        /**
        * @brief create singleton instance of clibhid class.(device manager)
        * 
        *   1. set default device filter to lpu237, lpu238, hidbootloader.
        * 
        *   2. create _vhid_api_briage instance.
        * 
        *   3. get the current connected device list.
        * 
        *   4. create & open clibhid_dev instatnce. (virtual device - using _vhid_api_briage instance) and register these to map.
        * 
        *   5. create & run device plug in/out thread.( clibhid::_worker_pluginout() ) 
        * 
        * @return the instance of clibhid class.
        */
        static clibhid& get_instance();

        ~clibhid();

        bool is_ini() const;

        void set_callback_pluginout(clibhid::type_callback_pluginout cb, void* p_user);

        clibhid_dev_info::type_set get_cur_device_set();

        clibhid_dev::type_wptr get_device(const clibhid_dev_info& dev_info);
        clibhid_dev::type_wptr get_device(const std::string& s_path);
        clibhid_dev::type_wptr get_device(const std::wstring& sw_path);

    protected:
        clibhid();

        clibhid_dev::type_wptr _get_device(const std::string& s_path);

        clibhid_dev::type_wptr _get_device(const clibhid_dev_info& dev_info);

        clibhid_dev::type_wptr _get_device(const std::wstring& sw_path);

    protected:

        /**
        * detect device plug in & out
        */
        static void _worker_pluginout(clibhid& lib);

        /**
        * @description - update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        * ONLY for _worker_pluginout.
        * @return - true - changed, false - not changed.
        */
        bool _update_dev_set();

        clibhid_dev_info::type_set _get_device_set();

        /**
        * for _worker_pluginout
        * create & remove  device instance of map.
        */
        void _update_device_map();

    protected:

        // virtual hidapi library instance
        _hid_api_briage::type_ptr m_ptr_hid_api_briage;

		void* m_p_user;// user parameter for callback(m_cb)
		clibhid::type_callback_pluginout m_cb; // callback function for plugin out.(change of device list)

		bool m_b_ini;//true - ini, false - not ini of this manager
        std::mutex m_mutex;

		// key : device path(primitive & compositive), value : first - device instance, second - device info instance
        clibhid::type_map_pair_ptrs m_map_pair_ptrs;

        // key : device path(lpu237 primitive only), value : first - device instance, second - device info instance
        clibhid::type_map_pair_ptrs m_map_pair_ptrs_lpu237;

		std::shared_ptr<std::thread> m_ptr_th_pluginout; //thread for detect plug in/out device.
		std::atomic<bool> m_b_run_th_pluginout; //true - run, false - stop of pluginout thread.

		type_set_usb_filter m_set_usb_filter; //filtering device list.

        clibhid_dev_info::type_set m_set_cur_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_inserted_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_removed_dev_info;//this member will be changed after _update_dev_set ().

    private://don't call these functions.
        
        clibhid(const clibhid&);
        clibhid& operator= (const clibhid&);
    };
   
    
}