#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_elpusk.h>
#include <mp_coperation.h>
#include <hid/mp_clibhid_dev.h>


namespace _mp{

    /**
    * this class job.
    * single tone type
    * 1. create & remove device.
    * 2. detect plug in/out device.
    */
    class clibhid
    {
    public:
        typedef std::shared_ptr< clibhid > type_ptr;

        typedef std::pair< clibhid_dev::type_ptr, clibhid_dev_info::type_ptr> pair_ptrs;
        typedef std::map< std::string, clibhid::pair_ptrs > type_map_ptrs;

        /**
        * callback funtion prototype
        * Don't use the member function in this callack function. If use, deadlock!! 
        * @parameter 1'st - inserted devices. 2'nd - removed devices. 3'rd - current device. 4'th - user parameter for callack
        */
        typedef	std::function<void(const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, const clibhid_dev_info::type_set&, void*)>	type_callback_pluginout;

    protected:
        enum {
            _const_dev_pluginout_check_interval_mmsec = 30
        };
    public:
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
        * ONLY for _worker_pluginout.
        * update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        */
        bool _update_dev_set();

        clibhid_dev_info::type_set _get_device_set();

        /**
        * for _worker_pluginout
        * create & remove  device instance of map.
        */
        void _update_device_map();

    protected:
        void* m_p_user;
        clibhid::type_callback_pluginout m_cb;

        std::shared_ptr<std::mutex> m_ptr_mutex_hidapi;//mutex for hidapi library
        bool m_b_ini;
        std::mutex m_mutex;
        clibhid::type_map_ptrs m_map_ptrs;

        std::shared_ptr<std::thread> m_ptr_th_pluginout;
        std::atomic<bool> m_b_run_th_pluginout;

        type_set_usb_filter m_set_usb_filter;

        clibhid_dev_info::type_set m_set_cur_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_inserted_dev_info;//this member will be changed after _update_dev_set ().
        clibhid_dev_info::type_set m_set_removed_dev_info;//this member will be changed after _update_dev_set ().

    private://don't call these functions.
        
        clibhid(const clibhid&);
        clibhid& operator= (const clibhid&);
    };
   
    
}