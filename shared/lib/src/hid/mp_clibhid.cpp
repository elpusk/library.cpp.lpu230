
#include <hid/mp_clibhid.h>
#include <mp_elpusk.h>

#ifdef _WIN32
#ifdef _DEBUG
#include <atltrace.h>
#endif
#endif

#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
#include <pthread.h>
#endif

namespace _mp{
    std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> clibhid::get_connected_device(const chid_briage::type_ptr& ptr_hid_api_briage)
    {
        std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> set_connected_dev;
        do {
            if (!ptr_hid_api_briage) {
                continue;
            }

            std::lock_guard<std::mutex> lock(ptr_hid_api_briage->get_mutex_for_hidapi());
            set_connected_dev = ptr_hid_api_briage->hid_enumerate();
        } while (false);
        return set_connected_dev;
    }


    clibhid& clibhid::get_instance(const _mp::type_set_usb_filter& set_usb_filter /*= _mp::type_set_usb_filter()*/)
    {
		static bool b_first = true;
        static clibhid::type_ptr ptr_obj;
        if (b_first) {
            b_first = false;
            if (set_usb_filter.empty()) {
                ptr_obj = std::shared_ptr<clibhid>(new clibhid());
            }
            else {
                ptr_obj = std::shared_ptr<clibhid>(new clibhid(set_usb_filter));
            }
        }

        return *ptr_obj;
    }

    clibhid& clibhid::get_manual_instance()
    {
        static bool b_first = true;
        static clibhid::type_ptr ptr_obj;
        if (b_first) {
            b_first = false;
            ptr_obj = std::shared_ptr<clibhid>(new clibhid(true));
        }

        return *ptr_obj;
    }


    bool clibhid::is_ini() const
    {
        return m_b_ini;
    }

    void clibhid::set_usb_filter(const _mp::type_set_usb_filter& set_usb_filter)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
		m_set_usb_filter = set_usb_filter;
    }

    void clibhid::set_dev_pluginout_check_interval(long long ll_mmsec)
    {
        m_atll_dev_pluginout_check_interval_mmsec.store( ll_mmsec, std::memory_order_relaxed);
    }

    void clibhid::set_dev_tx_by_api_check_interval(long long ll_mmsec)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto item : m_map_pair_ptrs) {
            if (item.second.first) {
                item.second.first->set_tx_by_api_check_interval(ll_mmsec);
            }
        }//
    }

    void clibhid::set_dev_rx_by_api_in_rx_worker_check_interval(long long ll_mmsec)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto item : m_map_pair_ptrs) {
            if (item.second.first) {
                item.second.first->set_rx_by_api_in_rx_worker_check_interval(ll_mmsec);
            }
        }//
    }

    void clibhid::set_dev_rx_q_check_interval(long long ll_mmsec)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
			
        for( auto item : m_map_pair_ptrs) {
            if (item.second.first) {
                item.second.first->set_rx_q_check_interval(ll_mmsec);
            }
		}//
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

    void clibhid::_ini(const chid_briage::type_ptr& ptr_briage /*= chid_briage::type_ptr()*/)
    {
        //
        // for supporting, virtual device. code is create child of _hid_api_briage class.
        if (!ptr_briage) {
            m_ptr_hid_api_briage = std::make_shared<chid_briage>();//create single instance of virtual hidapi library
        }
        else {
            m_ptr_hid_api_briage = ptr_briage; // 이미 생성된 briage api 사용.
        }

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

                if (!ptr_info->get_path_by_string().empty()) {
                    continue; // not primitive path.
                }

                if (ptr_info->get_vendor_id() != _mp::_elpusk::const_usb_vid) {
                    continue;
                }
                if (ptr_info->get_product_id() != _mp::_elpusk::_lpu237::const_usb_pid
                    && ptr_info->get_product_id() != _mp::_elpusk::_lpu238::const_usb_pid) {
                    continue;
                }

                // for lpu237 & lpu238 primitive path only
                m_map_pair_ptrs_lpu237.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));
            }//end for

            //
            m_b_run_th_pluginout = true;
            m_ptr_th_pluginout = std::shared_ptr<std::thread>(new std::thread(clibhid::_worker_pluginout, std::ref(*this)));
#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
            pthread_setname_np(m_ptr_th_pluginout->native_handle(), "clibhid");
#endif
        }

    }

    bool clibhid::update_dev_set_in_manual()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return _update_dev_set();
    }

    chid_briage::type_ptr clibhid::get_briage()
    {
        return m_ptr_hid_api_briage;
    }

    clibhid::clibhid() : clibhid(false)
    {
    }

    clibhid::clibhid(bool b_manual) :
        m_b_ini(false)
        , m_b_run_th_pluginout(false)
        , m_p_user(NULL)
        , m_cb(nullptr)
        , m_atll_dev_pluginout_check_interval_mmsec(clibhid::_const_default_dev_pluginout_check_interval_mmsec)
        , m_b_manual(b_manual)
    {
        if (!b_manual) {
            // 기본 값은 자동 처리 모드.
            //setup usb filter
            // 이 필터에 등록된 장치만 관리함.
            //default - support lpu237, lpu238, hidbootloader
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu237::const_usb_pid, _elpusk::_lpu237::const_usb_inf_hid); //lpu237
            m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::_lpu238::const_usb_pid, _elpusk::_lpu238::const_usb_inf_hid); //lpu238
            //m_set_usb_filter.emplace(_elpusk::const_usb_vid, _elpusk::const_usb_pid_hidbl, _elpusk::const_usb_inf_hidbl); //hidbootloader

            _ini(); //ptr_briage instance will ne created in _ini()
        }
        else{
            m_ptr_hid_api_briage = std::make_shared<chid_briage>();//create single instance of virtual hidapi library

            if (m_ptr_hid_api_briage->is_ini()) {
                m_b_ini = true;
                //수동 모드에서는 현재 연결된 장비를 얻지 않고
                // 쓰레드로 device plug in.out 을 검사하지 않는다.
            }

        }
    }

    clibhid::clibhid(const _mp::type_set_usb_filter& set_usb_filter) :
        m_b_ini(false)
        , m_b_run_th_pluginout(false)
        , m_p_user(NULL)
        , m_cb(nullptr)
        , m_atll_dev_pluginout_check_interval_mmsec(clibhid::_const_default_dev_pluginout_check_interval_mmsec)
		, m_set_usb_filter(set_usb_filter)
        , m_b_manual(false)
    {
        _ini();
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
		m_map_pair_ptrs_lpu237.clear();//remove lpu237 primitive path

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
		long long ll_check_interval = clibhid::_const_default_dev_pluginout_check_interval_mmsec;
		int n_reload_cnt = 0;

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

            ++n_reload_cnt;
            if (n_reload_cnt > (2000/ ll_check_interval)) {
                n_reload_cnt = 0;
                auto ll = lib.m_atll_dev_pluginout_check_interval_mmsec.load(std::memory_order_relaxed);
                if (ll_check_interval != ll) {
                    ll_check_interval = ll;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(ll_check_interval));
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

        /**
        * don't use the hid_enumerate() of hidapi. it will occur packet-losting. 
        */
        if (!m_ptr_hid_api_briage) {
            return st;
        }

        std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > set_dev;
        set_dev = m_ptr_hid_api_briage->hid_enumerate();

        for (auto item : set_dev) {
            st.emplace(
                std::get<0>(item).c_str(),
                std::get<1>(item),
                std::get<2>(item),
                std::get<3>(item),
                std::get<4>(item)
            );
        }//end for

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
				m_map_pair_ptrs_lpu237.erase(item.first);
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
                m_map_pair_ptrs_lpu237.erase(item.get_path_by_string());
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

                if (!ptr_info->get_path_by_string().empty()) {
                    continue; // not primitive path.
                }
                if (ptr_info->get_vendor_id() != _mp::_elpusk::const_usb_vid) {
                    continue;
                }
                if (ptr_info->get_product_id() != _mp::_elpusk::_lpu237::const_usb_pid
                    && ptr_info->get_product_id() != _mp::_elpusk::_lpu238::const_usb_pid) {
                    continue;
                }
                // for lpu237 & lpu238 primitive path only
                m_map_pair_ptrs_lpu237.emplace(item.get_path_by_string(), std::make_pair(ptr_dev, ptr_info));

            }//end for
        } while (false);
    }
}