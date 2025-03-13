
#include <hid/mp_clibhid.h>


#ifdef _WIN32
#ifdef _DEBUG
#include <atltrace.h>
#endif
#endif

namespace _mp{

        clibhid& clibhid::get_instance()
        {
            static clibhid obj;
            return obj;
        }

        bool clibhid::is_ini() const
        {
            return m_b_ini;
        }

        void clibhid::set_callback_pluginout(clibhid::type_callback_pluginout cb, void *p_user)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cb = cb;
            m_p_user = p_user;
        }

        clibhid_dev_info::type_set clibhid::get_cur_device_set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_set_cur_dev_info;
        }

        clibhid_dev::type_wptr clibhid::get_device(const clibhid_dev_info & dev_info)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(dev_info);
        }
        clibhid_dev::type_wptr clibhid::get_device(const std::string& s_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(s_path);
        }
        clibhid_dev::type_wptr clibhid::get_device(const std::wstring& sw_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _get_device(sw_path);
        }

        clibhid::clibhid() : m_b_ini(false), m_b_run_th_pluginout(false), m_p_user(NULL), m_cb(nullptr)
        {
            //setup usb filter
			//default - support lpu237, lpu238, hidbootloader
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu237::const_usb_pid, _elpusk::_lpu237::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu238::const_usb_pid, _elpusk::_lpu238::const_usb_inf_hid);
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::const_usb_pid_hidbl, _elpusk::const_usb_inf_hidbl);

            //
            // original code
            // m_ptr_hid_api_briage = std::make_shared<_hid_api_briage>();//create single instance of hidapi library
            // for supporting, virtual device. code is create child of _hid_api_briage class.
			m_ptr_hid_api_briage = std::make_shared<_vhid_api_briage>();//create single instance of virtual hidapi library

            if (m_ptr_hid_api_briage->is_ini()) {
                m_b_ini = true;

                m_set_cur_dev_info = _get_device_set();

                for (const clibhid_dev_info& item : m_set_cur_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_hid_api_briage.get());// create & open clibhid_dev.
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared<clibhid_dev_info>(item);
                    m_map_pair_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for

                //
                m_b_run_th_pluginout = true;
                m_ptr_th_pluginout = std::shared_ptr<std::thread>(new std::thread(clibhid::_worker_pluginout, std::ref(*this)));
            }
        
        }

        clibhid::~clibhid()
        {
            if (m_b_ini) {
                if (m_ptr_th_pluginout) {
                    m_b_run_th_pluginout = false;
                    if (m_ptr_th_pluginout->joinable()) {
                        m_ptr_th_pluginout->join();
                    }
                }
            }

            // 
            for (auto item : m_map_pair_ptrs) {
                item.second.first.reset();
                item.second.second.reset();
            }//
            m_map_pair_ptrs.clear();

            m_ptr_hid_api_briage.reset();//remove briage
        }

        clibhid_dev::type_wptr clibhid::_get_device(const std::string& s_path)
        {
            clibhid_dev::type_wptr wptr;
            do {
                auto it = m_map_pair_ptrs.find(s_path);
                if (it != m_map_pair_ptrs.end()) {
                    wptr = it->second.first;
                }
            } while (false);
            return wptr;
        }

        clibhid_dev::type_wptr clibhid::_get_device(const clibhid_dev_info& dev_info)
        {
            return _get_device(dev_info.get_path_by_string());
        }

        clibhid_dev::type_wptr clibhid::_get_device(const std::wstring & sw_path)
        {
            std::string s_path(cstring::get_mcsc_from_unicode(sw_path));
            return _get_device(s_path);
        }

        /**
        * detect device plug in & out
        */
        void clibhid::_worker_pluginout(clibhid& lib)
        {
            clibhid_dev_info::type_set set_dev;

            while (lib.m_b_run_th_pluginout) {
                do {
                    
                    std::lock_guard<std::mutex> lock(lib.m_mutex);
                    if (lib._update_dev_set()) {
                        //device list is changed
                        lib._update_device_map();

                        if (lib.m_cb) {//call callback.. for dev_ctl
                            lib.m_cb(lib.m_set_inserted_dev_info, lib.m_set_removed_dev_info, lib.m_set_cur_dev_info, lib.m_p_user);
                        }
                    }
                    
                } while (false);

                std::this_thread::sleep_for(std::chrono::milliseconds(clibhid::_const_dev_pluginout_check_interval_mmsec));
            }//end while

            std::wstringstream ss;
            ss << std::this_thread::get_id();
            std::wstring s_id = ss.str();
            //_mp::clog::get_instance().log_fmt(L"[I] exit : %ls : id = %ls.\n", __WFUNCTION__, s_id.c_str());<< dead lock

#ifdef _WIN32
#ifdef _DEBUG
            ATLTRACE(L"Exit clibhid::_worker_pluginout().\n");
#endif
#endif
        }

        /**
		* @description - update m_set_removed_dev_info, m_set_inserted_dev_info and m_set_cur_dev_info.
        * ONLY for _worker_pluginout.
		* @return - true - changed, false - not changed.
        */
        bool clibhid::_update_dev_set()
        {
            bool b_changed(false);
            clibhid_dev_info::type_set set_dev;

            do {
                set_dev = _get_device_set();

                //set_dev = m_set_cur_dev_info;//for debugging
                //m_ptr_usb_lib->update_device_list();//for debugging
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
                    clog::get_instance().trace(L"T[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - removed : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                for (auto item : m_set_inserted_dev_info) {
                    clog::get_instance().trace(L"T[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    clog::get_instance().log_fmt(L"[I] - %ls - inserted : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                }

                if (m_set_cur_dev_info.empty()) {
                    clog::get_instance().trace(L"T[I] - current : none.\n");
                }
                else {
                    for (auto item : m_set_cur_dev_info) {
                        clog::get_instance().trace(L"T[I] - %ls - current : %ls.\n", __WFUNCTION__, item.get_path_by_wstring().c_str());
                    }
                }

                //
                b_changed = true;
            } while (false);
            return b_changed;
        }

        clibhid_dev_info::type_set clibhid::_get_device_set()
        {
            clibhid_dev_info::type_set st;

            struct hid_device_info* p_devs = NULL;

            do {
                /**
                * don't use the hid_enumerate() of hidapi. it will occur packet-losting. 
                */
                if (!m_ptr_hid_api_briage) {
                    return st;
                }
                auto set_dev = m_ptr_hid_api_briage->hid_enumerate();

                for (auto item : set_dev) {
                    st.emplace(
                        std::get<0>(item).c_str(),
                        std::get<1>(item),
                        std::get<2>(item),
                        std::get<3>(item),
                        std::get<4>(item)
                    );
                }//end for
            } while (false);

            // checks the device critial error 
            clibhid_dev_info::type_set st_must_be_removed;
            for (auto item : m_map_pair_ptrs) {
                if (!item.second.first) {
                    continue;
                }
                if (!item.second.first->is_detect_replugin()) {
                    continue;
                }
                //critical error .. remove device forcelly
                if (item.second.second) {
                    st_must_be_removed.insert(*item.second.second);
                }
            }//end for

            clibhid_dev_info::type_set st_after_removed = coperation::subtract<clibhid_dev_info>(st, st_must_be_removed);
            //
            if (!m_set_usb_filter.empty()) {
                //filtering.. 
                clibhid_dev_info::type_set set_union_found;
                std::for_each(std::begin(m_set_usb_filter), std::end(m_set_usb_filter), [&](const type_tuple_usb_filter& filter) {
                    clibhid_dev_info::type_set set_found;
                    if (clibhid_dev_info::filter_dev_info_set(set_found, st_after_removed, filter) > 0) {
                        set_union_found.insert(std::begin(set_found), std::end(set_found));
                    }
                    });

                st_after_removed.swap(set_union_found);
            }

            return st_after_removed;
        }

        /**
        * for _worker_pluginout
        * create & remove  device instance of map.
        */
        void clibhid::_update_device_map()
        {
            do {
                if (!m_ptr_hid_api_briage) {
                    continue;
                }

                //remove
                for (auto item : m_set_removed_dev_info) {
                    m_map_pair_ptrs.erase(item.get_path_by_string());
                }

                //insert
                for (const clibhid_dev_info & item : m_set_inserted_dev_info) {
                    auto ptr_dev = std::make_shared<clibhid_dev>(item, m_ptr_hid_api_briage.get()); // create & open clibhid_dev.
                    if (!ptr_dev) {
                        continue;
                    }
                    if (!ptr_dev->is_open()) {
                        continue;
                    }
                    auto ptr_info = std::make_shared< clibhid_dev_info>(item);
                    m_map_pair_ptrs.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
                }//end for
            } while (false);
        }
  
    
}