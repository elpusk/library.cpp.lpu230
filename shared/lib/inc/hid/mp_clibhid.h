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
#include <chid_briage.h>


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
        * @brief get connected devices. static indepent fcuntion
        *
        * @return  each item's
        *
        *	1'st - std::string, device path,
        *
        *	2'nd - unsigned short, usb vendor id,
        *
        *	3'th - unsigned short, usb product id,
        *
        *	4'th - int, usb interface number,
        *
        *	5'th - std::string, extra data
        */
        static std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > get_connected_device(const chid_briage::type_ptr& ptr_hid_api_briage);

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
        * @param set_usb_filter - if this parameter is empty, default filter will be set to lpu237, lpu238, hidbootloader.
        *   
        *   this will affected to constructor only.
        * 
        * @return the instance of clibhid class.
        */
        static clibhid& get_instance( const _mp::type_set_usb_filter& set_usb_filter = _mp::type_set_usb_filter() );

        /**
        * @brief create singleton instance of clibhid class.(device manager) WITH MANUAL mode.
        * 
        *   in this mode, all zeros packet is ignored.
        * 
        *   get_instance() and get_manual_instance() must be used exculsively.
        * 
        */
        static clibhid& get_manual_instance();

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


        chid_briage::type_ptr get_briage();

        /**
        * @brief Consider the equipment to be removed
        */
        bool consider_to_be_removed(int n_vid, int n_pid);

        /**
        * @brief Cancel considering the equipment as removed
        */
        bool cancel_considering_dev_as_removed(int n_vid =-1, int n_pid=-1);
        
    protected:
        clibhid();
        clibhid(bool b_manual,bool b_remove_all_zero_in_report);
		clibhid(const _mp::type_set_usb_filter& set_usb_filter);

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
        void _ini(const chid_briage::type_ptr & ptr_briage = chid_briage::type_ptr());

    protected:
        bool m_b_remove_all_zero_in_report; // default false, all zeros value report is ignored

        bool m_b_manual;

        // virtual hidapi library instance
        //_hid_api_briage::type_ptr m_ptr_hid_api_briage;
        chid_briage::type_ptr m_ptr_hid_api_briage;

		void* m_p_user;// user parameter for callback(m_cb)
		clibhid::type_callback_pluginout m_cb; // callback function for plugin out.(change of device list)
        std::atomic_llong m_atll_dev_pluginout_check_interval_mmsec;

		bool m_b_ini;//true - ini, false - not ini of this manager
        std::mutex m_mutex;

        // this usb device must be considerated to plugout.
        // protected by m_mutex
        _mp::type_set_usb_id m_set_usb_id_considerated_to_remove;

		// key : device path(primitive & compositive), value : first - device instance, second - device info instance
        clibhid::type_map_pair_ptrs m_map_pair_ptrs;

        // key : device path(lpu237 primitive only), value : first - device instance, second - device info instance
        clibhid::type_map_pair_ptrs m_map_pair_ptrs_lpu237;

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