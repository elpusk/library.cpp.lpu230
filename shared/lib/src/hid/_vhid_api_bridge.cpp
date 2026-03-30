#include <chrono>
#include <algorithm>
#include <mp_elpusk.h>

#include <hid/_vhid_info_lpu237.h>
#include <hid/_vhid_api_bridge.h>
#include <hid/_vhid_info.h>

#ifdef _WIN32
#ifdef _DEBUG
//#undef __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY__

#include <atltrace.h>
#endif
#endif

#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
#include <pthread.h>
#endif

/**
* member function bodies
*/
_vhid_api_bridge::_vhid_api_bridge() : _hid_api_bridge()
{
    m_s_class_name = L"_vhid_api_bridge";
}

_vhid_api_bridge::_vhid_api_bridge(_mp::clog* p_clog) : _hid_api_bridge(p_clog)
{
    m_s_class_name = L"_vhid_api_bridge";
}

_vhid_api_bridge::~_vhid_api_bridge()
{
    m_map_ptr_hid_info.clear();
}

std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > _vhid_api_bridge::hid_enumerate()
{
    /**
    * 1'st - device path, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number, 5'th - extra data
    */
    std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > set_dev_in, set_dev_out;

    // get pysical device list
    set_dev_in = _hid_api_bridge::hid_enumerate();//연결된 HID list 를 얻음. 

    for (auto item : set_dev_in) {
        std::string s_path(std::get<0>(item));
        unsigned short w_vid(std::get<1>(item));
        unsigned short w_pid(std::get<2>(item));
        int n_interface(std::get<3>(item));
        std::string s_extra_path;
        std::set<std::string> set_v_extra_path;
        //
        //각 물리적 device path 에 가상 device path 도 추가 한다.
        
        auto out_item = std::make_tuple(
            s_path,
            w_vid,
            w_pid,
            n_interface,
            std::string()
        );

        //add virtual devices for lpu237
        const _vhid_info::type_set_path_type& set_path_type(_vhid_info::get_extra_paths(w_vid, w_pid, n_interface));

        for (auto extra_item : set_path_type) {
            std::get<4>(out_item) = std::get<0>(extra_item);
            set_dev_out.insert(out_item);
        }//end for

    }//end for

    return set_dev_out;
}

std::tuple<bool, int, bool> _vhid_api_bridge::is_open(const char* path) const
{
    std::lock_guard<std::mutex> lock(m_mutex_for_map);
    return _is_open(path);
}

int _vhid_api_bridge::api_open_path(const char* path)
{
    int n_map_index(_vhid_info::const_map_index_invalid);
    int n_primitive_map_index(_vhid_info::const_map_index_invalid);
    _mp::type_bm_dev t(_mp::type_bm_dev_unknown);
    std::string s_primitive;
    bool b_support_shared_open(false);
    bool b_compositive(false);

    if (path == nullptr) {
        if (m_p_clog) {
            m_p_clog->log_fmt(L"[E] %ls : path is null.\n", __WFUNCTION__);
            m_p_clog->trace(L"[E] %ls : path is null.\n", __WFUNCTION__);
        }
        return _vhid_info::const_map_index_invalid;
    }
    std::tie(b_compositive, t, s_primitive, b_support_shared_open) = _vhid_info::is_path_compositive_type(std::string(path));
    bool b_open(false);

    // primitive(물리적 device) 가 open 되었는지 확인.
    std::tie(b_open, n_primitive_map_index, std::ignore) = _hid_api_bridge::is_open(s_primitive.c_str());

    if (!b_open) {
        // primitive 를 open 한다.
        n_map_index = n_primitive_map_index = _hid_api_bridge::api_open_path(s_primitive.c_str());
        if (n_primitive_map_index < 0) {
            if (m_p_clog) {
                m_p_clog->log_fmt(L"[E] %ls : for compositive, open primitive error.\n", __WFUNCTION__);
                m_p_clog->trace(L"[E] %ls : for compositive, open primitive error.\n", __WFUNCTION__);
            }
            return _vhid_info::const_map_index_invalid;;// for compositive, open primitive error.
        }

#if defined(_WIN32) && defined(_DEBUG)
        if (path) {
            ATLTRACE("_create_new_map_primitive_item : index = 0x%x.(%s)\n", n_primitive_map_index, path);
        }
        else {
            ATLTRACE("_create_new_map_primitive_item : index = 0x%x.\n", n_primitive_map_index);
        }
#endif
    }

    // 여기는 primitive open 이미 되어 있거나, 방금 open 성공한 경우.
    if (b_compositive) {
        //convert primitive index to composite index.
        n_map_index = _vhid_info::get_compositive_map_index_from_primitive_map_index(t, n_primitive_map_index);
    }
    _vhid_info::type_ptr ptr_vhid_info;

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);

        auto it = m_map_ptr_hid_info.find(n_primitive_map_index);
        if (it == std::end(m_map_ptr_hid_info)) {
            std::string s_in_path(path);
            //물리적 device 와 연결된 primitive & 모든 compositive device 의 open counter 를 관리 할 _vhid_info 객체 생성.
            /**
            * 1'st - device path, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number, 5'th - extra data
            */
            std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > set_primitive_dev;

            // get physical device list
            set_primitive_dev = _hid_api_bridge::hid_enumerate();//연결된 HID list 를 얻음. 

            for (auto item : set_primitive_dev) {
                std::string s_path(std::get<0>(item));
                unsigned short w_vid(std::get<1>(item));
                unsigned short w_pid(std::get<2>(item));
                //
                if (w_vid != _mp::_elpusk::const_usb_vid) {
                    continue;
                }

                if (w_pid == _mp::_elpusk::_lpu237::const_usb_pid) {
                    if (s_in_path.find(s_path) == 0) {
                        // lpu237 장비면.
                        m_map_ptr_hid_info[n_primitive_map_index] = ptr_vhid_info = std::make_shared<_vhid_info_lpu237>(t); // in constructure, type open counter is increased.
#if defined(_WIN32) && defined(_DEBUG)
                        ATLTRACE(L"_create_new_map_compositive_item : index = 0x%x.\n", n_map_index);
#endif
                        break; //exit for
                    }
                }
                else if (w_pid == _mp::_elpusk::_lpu238::const_usb_pid) {
                    if (s_in_path.find(s_path) == 0) {
                        // lpu238 장비면....... ,아직 lpu238 지원 않함.
                        if (m_p_clog) {
                            std::wstring _ws(_mp::cstring::get_unicode_from_mcsc(path));
                            m_p_clog->log_fmt(L"[E] %ls : _create_new_map_compositive_item : not support : lpu238 device(%ls).\n", __WFUNCTION__, _ws.c_str());
                            m_p_clog->trace(L"[E] %ls : _create_new_map_compositive_item : not support : lpu238 device.\n", __WFUNCTION__, _ws.c_str());
                        }
#if defined(_WIN32) && defined(_DEBUG)
                        ATLTRACE(L"_create_new_map_compositive_item : not support : lpu238 device.\n");
#endif
                    }
                }
                else if (w_pid == _mp::_elpusk::const_usb_pid_hidbl) {
                    if (s_in_path.find(s_path) == 0) {
                        //hid bootloader.
                        m_map_ptr_hid_info[n_primitive_map_index] = ptr_vhid_info = std::make_shared<_vhid_info_lpu237>(t); // in constructure, type open counter is increased.
#if defined(_WIN32) && defined(_DEBUG)
                        ATLTRACE(L"_create_new_map_compositive_item hid boot : index = 0x%x.\n", n_map_index);
#endif
                        break; //exit for
                    }
                }
            }//end for

            if (!ptr_vhid_info) {
                if (m_p_clog) {
                    std::wstring _ws(_mp::cstring::get_unicode_from_mcsc(path));
                    m_p_clog->log_fmt(L"[E] %ls : create vhid_info object(%ls).\n", __WFUNCTION__, _ws.c_str());
                    m_p_clog->trace(L"[E] %ls : create vhid_info object(%ls).\n", __WFUNCTION__, _ws.c_str());
                }
            }
            //ptr_vhid_info 가 생성되시 open cnt 가 자동 inc 하므로 생성 후에는 딱히 조정 불요.
            continue;
        }

        // 이미 ptr_vhid_info 객체가 있는 경우.
        ptr_vhid_info = it->second;

        //already opened 
        if (!ptr_vhid_info->can_be_open(t, b_support_shared_open)) {
            n_map_index = _vhid_info::const_map_index_invalid;
            continue; // this type cannt be open by shared-mode.
        }

        // compositive shared open or primitive open
        bool b_adjustment(false);
        int n_open_cnt(0);

        std::tie(b_adjustment, n_open_cnt, std::ignore) = ptr_vhid_info->ajust_open_cnt(t, true);
        if (!b_adjustment) {
            n_map_index = _vhid_info::const_map_index_invalid;
            continue;//open failure....... previous opened compositive isn't support shared open! 
        }
        
        if (b_compositive && n_open_cnt == 1) {
            // compositive 가 처음 open 되면 수신 q만 추가.
            // 수신 q 만 추가함.
            _add_rx_q(n_map_index);
        }

    } while (false);

    
    return n_map_index;
}

void _vhid_api_bridge::api_close(int n_map_index)
{
    do {
        int n_primitive(-1);
        bool b_compositive_type(false);
        std::tie(n_primitive, b_compositive_type) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);
        if (n_primitive < 0) {
            if (m_p_clog) {
                m_p_clog->log_fmt(L"[E] %ls : invalid n_map_index(0x%x).\n", __WFUNCTION__, n_map_index);
                m_p_clog->trace(L"[E] %ls : invalid n_map_index(0x%x).\n", __WFUNCTION__, n_map_index);
            }
            continue;
        }
        //get type from index
        auto t = _vhid_info::get_type_from_compositive_map_index(n_map_index);
        if (t == _mp::type_bm_dev_unknown) {
            if (m_p_clog) {
                m_p_clog->log_fmt(L"[E] %ls : invalid type(0x%x).\n", __WFUNCTION__, t);
                m_p_clog->trace(L"[E] %ls : invalid type(0x%x).\n", __WFUNCTION__, t);
            }
            continue;
        }

        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        auto it = m_map_ptr_hid_info.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info)) {
            if (m_p_clog) {
                m_p_clog->log_fmt(L"[E] %ls : not opened(0x%x).\n", __WFUNCTION__, n_map_index);
                m_p_clog->trace(L"[E] %ls : not opened(0x%x).\n", __WFUNCTION__, n_map_index);
            }
            continue;//not opened
        }

        _vhid_info::type_ptr ptr_vhid_info = it->second;
        bool b_result(false);
        bool b_all_compositive_type_are_zeros(false);

        //decrease open counter
        std::tie(b_result, std::ignore, b_all_compositive_type_are_zeros) = ptr_vhid_info->ajust_open_cnt(t, false);
        if (!b_result) {
            if (m_p_clog) {
                m_p_clog->log_fmt(L"[E] %ls : decrease open cnt(0x%x).\n", __WFUNCTION__, n_map_index);
                m_p_clog->trace(L"[E] %ls : decrease open cnt(0x%x).\n", __WFUNCTION__, n_map_index);
            }
            continue;
        }

        if (b_compositive_type) {
            if (!ptr_vhid_info->is_open(t)) {
                // 해당 가상 장비 사용이 종료되면, rx 도 제거.
                _remove_rx_q(n_map_index);
            }
        }
        // 
        if (b_all_compositive_type_are_zeros) {
            //all compositive type is closed
            m_map_ptr_hid_info.erase(it);
            _hid_api_bridge::api_close(n_primitive);
        }

    } while (false);
}

int _vhid_api_bridge::api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        if (n_primitive == _vhid_info::const_map_index_invalid) {
            continue;
        }

        auto it = m_map_ptr_hid_info.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info)) {
            continue;
        }

        n_result = _hid_api_bridge::api_get_report_descriptor(n_primitive, buf, buf_size);

    } while (false);

    return n_result;
}

int _vhid_api_bridge::api_write(int n_map_index, const unsigned char* data, size_t length, _mp::type_next_io next)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        if (n_primitive == _vhid_info::const_map_index_invalid) {
            continue;
        }

        auto it = m_map_ptr_hid_info.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info)) {
            continue;
        }

        n_result = _hid_api_bridge::api_write(n_primitive, data, length, next);
    } while (false);

    return n_result;
}

int _vhid_api_bridge::api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);

        if (n_primitive == _vhid_info::const_map_index_invalid) {
            if (m_p_clog)
                m_p_clog->log_fmt(L"[E] %ls : invalid map index.\n", __WFUNCTION__);
            continue;
        }

        auto it = m_map_ptr_hid_info.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info)) {
            if (m_p_clog)
                m_p_clog->log_fmt(L"[E] %ls : not found map item.\n", __WFUNCTION__);
            continue;
        }

        n_result = _hid_api_bridge::api_read(n_map_index, data, length, n_report);
    } while (false);

#ifdef _WIN32
#ifdef _DEBUG
    static size_t n_ct(0);
    if (n_result > 2) {
        ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0],rx[1],rx[2]) = (%d, 0x%x, 0x%x, 0x%x)\n", \
            GetCurrentThreadId(), n_map_index,
            _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
            n_ct++, n_result, data[0], data[1], data[2]);
    }
    else if (n_result > 1) {
        ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0],rx[1]) = (%d, 0x%x, 0x%x)\n", \
            GetCurrentThreadId(), n_map_index,
            _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
            n_ct++, n_result, data[0], data[1]);
    }
    else if (n_result > 0) {
        ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0]) = (%d,0x%x)\n",
            GetCurrentThreadId(), n_map_index,
            _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
            n_ct++, n_result, data[0]);
    }

    if (n_result < 0) {
        if (m_p_clog) {
            m_p_clog->log_fmt(L"[E] %ls : n_result = %d.\n", __WFUNCTION__, n_result);
        }
        ATLTRACE(L"0x%08X-ERROR-RX (n_result) = (%d)\n", n_map_index, n_result);
    }
#endif
#endif



    return n_result;
}

const wchar_t* _vhid_api_bridge::api_error(int n_map_index)
{
    int n_primitive(-1);
    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);
    return _hid_api_bridge::api_error(n_map_index);
}

std::tuple<bool, int, bool> _vhid_api_bridge::_is_open(const char* path) const
{
    bool b_open(false);
    int n_primitive_map_index(-1);
    int n_compositive_map_index(-1);
    bool b_exclusive_open(false);

    do {
        _mp::type_bm_dev t(_mp::type_bm_dev_unknown);
        std::string s_primitive;
        bool b_support_shared_open(false);

        std::tie(std::ignore, t, s_primitive, b_support_shared_open) = _vhid_info::is_path_compositive_type(std::string(path));
        std::tie(b_open, n_primitive_map_index, std::ignore) = _hid_api_bridge::is_open(s_primitive.c_str()); //check primitive type

        // the base type is opened ?
        if (!b_open) {
            continue;
        }

        auto it_info = m_map_ptr_hid_info.find(n_primitive_map_index);
        if (it_info == std::end(m_map_ptr_hid_info)) {
            b_open = false; // not opened compositive type
            continue;
        }

        //
        b_exclusive_open = !b_support_shared_open;

    } while (false);

    return std::make_tuple(b_open, n_primitive_map_index, b_exclusive_open);

}
