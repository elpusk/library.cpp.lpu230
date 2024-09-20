#pragma once

#include <memory>
#include <mutex>
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

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

    private:
        enum {
            _const_dev_pluginout_check_interval_mmsec = 5// 30
        };
    public:
        static clibhid& get_instance()
        {
            static clibhid obj;
            return obj;
        }

        ~clibhid()
        {
            m_map_ptrs.clear();

            if(m_b_ini){
                if (m_ptr_th_pluginout) {
                    m_b_run_th_pluginout = false;
                    if (m_ptr_th_pluginout->joinable()) {
                        m_ptr_th_pluginout->join();
                    }
                }
                hid_exit();
            }

            m_ptr_usb_lib.reset();
        }

        bool is_ini() const
        {
            return m_b_ini;
        }

        void set_callback_pluginout(clibhid::type_callback_pluginout cb, void *p_user)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cb = cb;
            m_p_user = p_user;
        }

        clibhid_dev_info::type_set get_cur_device_set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_set_cur_dev_info;
        }

        clibhid_dev::type_wptr get_device(const clibhid_dev_info & dev_info)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(dev_info);
        }
        clibhid_dev::type_wptr get_device(const std::string& s_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(s_path);
        }
        clibhid_dev::type_wptr get_device(const std::wstring& sw_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(sw_path);
        }

    private:
        clibhid() : m_b_ini(false), m_b_run_th_pluginout(false), m_p_user(NULL), m_cb(nullptr)
        {
            //setup usb filter
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu237::const_usb_pid, _elpusk::_lpu237::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu238::const_usb_pid, _elpusk::_lpu238::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::const_usb_pid_hidbl, _elpusk::const_usb_inf_hidbl);

            m_ptr_usb_lib = std::make_shared<clibusb>();

            if (hid_init() == 0) {
                m_b_ini = true;

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
                // To work properly needs to be called before hid_open/hid_open_path after hid_init.
                // Best/recommended option - call it right after hid_init.
                hid_darwin_set_open_exclusive(0);
#endif
                m_set_cur_dev_info = _get_device_set();

                for (const clibhid_dev_info& item : m_set_cur_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_usb_lib);
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared< clibhid_dev_info>(item);
                    m_map_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for

                //
                m_b_run_th_pluginout = true;
                m_ptr_th_pluginout = std::shared_ptr<std::thread>(new std::thread(clibhid::_worker_pluginout, std::ref(*this)));
            }
        
        }

        clibhid_dev::type_wptr _get_device(const std::string& s_path)
        {
            clibhid_dev::type_wptr wptr;
            do {
                auto it = m_map_ptrs.find(s_path);
                if (it != m_map_ptrs.end()) {
                    wptr = it->second.first;
                }
            } while (false);
            return wptr;
        }

        clibhid_dev::type_wptr _get_device(const clibhid_dev_info& dev_info)
        {
            return _get_device(dev_info.get_path_by_string());
        }

        clibhid_dev::type_wptr _get_device(const std::wstring & sw_path)
        {
            std::string s_path(cstring::get_mcsc_from_unicode(sw_path));
            return _get_device(s_path);
        }

    private:

        /**
        * detect device plug in & out
        */
        static void _worker_pluginout(clibhid& lib)
        {
            clibhid_dev_info::type_set set_dev;

            while (lib.m_b_run_th_pluginout) {
                do {
                    std::lock_guard<std::mutex> lock(lib.m_mutex);
                    if (lib._update_dev_set()) {
                        //device list is changed
                        lib._update_device_map();

                        if (lib.m_cb) {//call callback
                            lib.m_cb(lib.m_set_inserted_dev_info, lib.m_set_removed_dev_info, lib.m_set_cur_dev_info, lib.m_p_user);
                        }
                    }
                } while (false);

                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid::_const_dev_pluginout_check_interval_mmsec));
            }//end while

            std::wstringstream ss;
            ss << std::this_thread::get_id();
            std::wstring s_id = ss.str();
            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());

        }

        /**
        * ONLY for _worker_pluginout.
        * update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        */
        bool _update_dev_set()
        {
            bool b_changed(false);
            clibhid_dev_info::type_set set_dev;

            do {
                set_dev = _get_device_set();
                //
                if (set_dev.size() == m_set_cur_dev_info.size()) {
                    if (set_dev.empty()) {
                        //no connected device
                        m_set_removed_dev_info.clear();	m_set_inserted_dev_info.clear();
                        continue;
                    }
                    if (set_dev==m_set_cur_dev_info) {
                        //no changned
                        m_set_removed_dev_info.clear();	m_set_inserted_dev_info.clear();
                        continue;
                    }
                }

                m_set_removed_dev_info = coperation::subtract<clibhid_dev_info>(m_set_cur_dev_info, set_dev);
                m_set_inserted_dev_info = coperation::subtract<clibhid_dev_info>(set_dev, m_set_cur_dev_info);
                m_set_cur_dev_info = set_dev;

                for (auto item : m_set_removed_dev_info) {
                    clog::get_instance().trace(L"[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                for (auto item : m_set_inserted_dev_info) {
                    clog::get_instance().trace(L"[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                if (m_set_cur_dev_info.empty()) {
                    clog::get_instance().trace(L"[I] - current : none.\n");
                }
                else {
                    for (auto item : m_set_cur_dev_info) {
                        clog::get_instance().trace(L"[I] - %ls - current : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    }
                }

                //
                b_changed = true;
            } while (false);
            return b_changed;
        }

        /**
        * for _worker_pluginout
        */
        clibhid_dev_info::type_set _get_device_set()
        {
            clibhid_dev_info::type_set st;
            struct hid_device_info* devs = hid_enumerate(0x0, 0x0);

            if (devs) {
                st = clibhid_dev_info::get_dev_info_set(devs);
                hid_free_enumeration(devs);
            }

            m_ptr_usb_lib->update_device_list();

            if (!m_set_usb_filter.empty()) {
                //filtering.. 
                clibhid_dev_info::type_set set_union_found;
                std::for_each(std::begin(m_set_usb_filter), std::end(m_set_usb_filter), [&](const type_tuple_usb_filter& filter) {
                    clibhid_dev_info::type_set set_found;
                    if (clibhid_dev_info::filter_dev_info_set(set_found, st, filter) > 0) {
                        set_union_found.insert(std::begin(set_found), std::end(set_found));
                    }
                    });

                st.swap(set_union_found);
            }

            return st;
        }

        /**
        * for _worker_pluginout
        * create & remove  device instance of map.
        */
        void _update_device_map()
        {
            do {
                //remove
                for (auto item : m_set_removed_dev_info) {
                    m_map_ptrs.erase(item.get_path_by_string());
                }

                //insert
                for (const clibhid_dev_info & item : m_set_inserted_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_usb_lib);
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared< clibhid_dev_info>(item);
                    m_map_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for
            } while (false);
        }

    private:
        clibusb::type_ptr m_ptr_usb_lib;

        void* m_p_user;
        clibhid::type_callback_pluginout m_cb;

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