#include <chrono>
#include <algorithm>
#include <mp_elpusk.h>

#include <mp_clog.h>
#include <hid/_vhid_info_lpu237.h>
#include <hid/_vhid_api_briage.h>
#include <hid/_vhid_info.h>

#ifdef _WIN32
#ifdef _DEBUG
#include <atltrace.h>
#endif
#endif

/**
* member function bodies
*/
_vhid_api_briage::_vhid_api_briage() : _hid_api_briage()
{
    m_s_class_name = L"_vhid_api_briage";
}

_vhid_api_briage::~_vhid_api_briage()
{
    m_map_ptr_hid_info_ptr_worker.clear();
}

std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > _vhid_api_briage::hid_enumerate()
{

    /**
    * 1'st - device path, 2'nd - usb vendor id, 3'th - usb product id, 4'th - usb interface number, 5'th - extra data
    */
    std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> > set_dev_in, set_dev_out;

    // get pysical device list
    set_dev_in = _hid_api_briage::hid_enumerate();

    for (auto item : set_dev_in) {
        std::string s_path(std::get<0>(item));
        unsigned short w_vid(std::get<1>(item));
        unsigned short w_pid(std::get<2>(item));
        int n_interface(std::get<3>(item));
        std::string s_extra_path;
        std::set<std::string> set_v_extra_path;
        //
        //add primitive path
        
        auto out_item = std::make_tuple(
            s_path,
            w_vid,
            w_pid,
            n_interface,
            std::string()
        );

        //add virtual devices for lpu237
        for ( auto extra_item : _vhid_info::get_extra_paths(w_vid, w_pid, n_interface)) {
            std::get<4>(out_item) = std::get<0>(extra_item);
            set_dev_out.insert(out_item);
        }//end for

    }//end for

    return set_dev_out;
}

std::tuple<bool, int, bool> _vhid_api_briage::is_open(const char* path) const
{
    std::lock_guard<std::mutex> lock(m_mutex_for_map);
    return _is_open(path);
}

int _vhid_api_briage::api_open_path(const char* path)
{
    int n_map_index(_vhid_info::const_map_index_invalid);
    int n_primitive_map_index(_vhid_info::const_map_index_invalid);
    _mp::type_bm_dev t(_mp::type_bm_dev_unknown);
    std::string s_primitive;
    bool b_support_shared_open(false);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        if (path == nullptr) {
            continue;
        }

        std::tie(std::ignore, t, s_primitive, b_support_shared_open) = _vhid_info::is_path_compositive_type(std::string(path));
        bool b_open(false);

        std::tie(b_open, n_primitive_map_index, std::ignore ) = _hid_api_briage::is_open(s_primitive.c_str());

        if (!b_open) {
            n_map_index = n_primitive_map_index = _hid_api_briage::api_open_path(s_primitive.c_str());
            if (n_primitive_map_index < 0) {
                continue;// for compositive, open primitive error.
            }

#ifdef _WIN32
#ifdef _DEBUG
            ATLTRACE(L"_create_new_map_primitive_item : index = 0x%x.\n", n_primitive_map_index);
#endif
#endif
        }

        // here primitive is opened.
        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive_map_index);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            m_map_ptr_hid_info_ptr_worker[n_primitive_map_index] =
                std::make_pair(
                    std::make_shared<_vhid_info_lpu237>(t), // in constructure, type open counter is increased.
                    std::make_shared<_q_worker>(n_primitive_map_index, this)
                );
            //convert primitive index to composite index.
            n_map_index = _vhid_info::get_compositive_map_index_from_primitive_map_index(t, n_primitive_map_index);

#ifdef _WIN32
#ifdef _DEBUG
            ATLTRACE(L"_create_new_map_compositive_item : index = 0x%x.\n", n_map_index);
#endif
#endif
            continue;
        }

        //already opened 
        if (!it->second.first->can_be_open(t, !b_support_shared_open)) {
            n_map_index = _vhid_info::const_map_index_invalid;
            continue; // thie type cannt be open by shared-mode.
        }

        // compositive shared open or primitive open
        bool b_adjustment(false);

        std::tie(b_adjustment, std::ignore, std::ignore) = it->second.first->ajust_open_cnt(t, true);
        if (!b_adjustment) {
            n_map_index = _vhid_info::const_map_index_invalid;
            continue;//open failure....... previous opened compositive isn't support shared open! 
        }

        //convert primitive index to composite index.
        n_map_index = _vhid_info::get_compositive_map_index_from_primitive_map_index(t, n_primitive_map_index);
        
    } while (false);

    
    return n_map_index;
}

void _vhid_api_briage::api_close(int n_map_index)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);

        int n_primitive(-1);
        bool b_compositive_type(false);
        std::tie(n_primitive, b_compositive_type) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);
        if (n_primitive < 0) {
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            continue;//not opened
        }

        // here opened.
        // 
        //get type from index
        auto t = _vhid_info::get_type_from_compositive_map_index(n_map_index);
        if (t == _mp::type_bm_dev_unknown) {
            continue;
        }

        bool b_result(false);
        bool b_all_compositive_type_are_zeros(false);

        //decrease open counter
        std::tie(b_result,std::ignore,b_all_compositive_type_are_zeros) = it->second.first->ajust_open_cnt(t, false);
        if (!b_result) {
            continue;
        }

        if (b_all_compositive_type_are_zeros) {
            //all compositive type is closed
            m_map_ptr_hid_info_ptr_worker.erase(it);
            _hid_api_briage::api_close(n_primitive);
        }

    } while (false);
}

int _vhid_api_briage::api_set_nonblocking(int n_map_index, int nonblock)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);
    _vhid_api_briage::_q_item::type_ptr ptr_item;

    std::tie(n_primitive,std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);

        if (n_primitive == _vhid_info::const_map_index_invalid) {
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            continue;
        }

        if (it->second.first->get_none_blocking() && nonblock == 1) {
            //already noneblocking
            n_result = 0;
            continue;
        }
        if (!it->second.first->get_none_blocking() && nonblock == 0) {
            //already blocking
            n_result = 0;
            continue;
        }

        ptr_item = std::make_shared<_vhid_api_briage::_q_item>(n_map_index);
        ptr_item->setup_for_set_nonblocking(nonblock);
        if (!it->second.second->push(ptr_item)) {
            continue;
        }

        b_run = true;

    } while (false);

    do {
        if (!b_run) {
            continue;
        }
        //waits response
        ptr_item->waits();
        std::tie(n_result, std::ignore) = ptr_item->get_result();

    } while (false);

    return n_result;
}

int _vhid_api_briage::api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);
    _vhid_api_briage::_q_item::type_ptr ptr_item;

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        if (n_primitive == _vhid_info::const_map_index_invalid) {
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            continue;
        }

        ptr_item = std::make_shared<_vhid_api_briage::_q_item>(n_map_index);
        ptr_item->setup_for_get_report_descriptor(buf, buf_size);

        if (!it->second.second->push(ptr_item)) {
            continue;
        }

        b_run = true;
    } while (false);

    do {
        if (!b_run) {
            continue;
        }
        //waits response
        ptr_item->waits();
        //rx data will be save on buf directly.
        std::tie(n_result, std::ignore) = ptr_item->get_result();

    } while (false);

    return n_result;
}

int _vhid_api_briage::api_write(int n_map_index, const unsigned char* data, size_t length, _hid_api_briage::type_next_io next)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);
    _vhid_api_briage::_q_item::type_ptr ptr_item;

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);
        if (n_primitive == _vhid_info::const_map_index_invalid) {
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            continue;
        }

        ptr_item = std::make_shared<_vhid_api_briage::_q_item>(n_map_index);
        ptr_item->setup_for_write(data, length,next);

        if (!it->second.second->push(ptr_item)) {
            continue;
        }

        b_run = true;
    } while (false);

    do {
        if (!b_run) {
            continue;
        }
        //
        if (next == _hid_api_briage::next_io_none) {
            //waits response
            ptr_item->waits();
            //rx data will be save on buf directly.
            std::tie(n_result, std::ignore) = ptr_item->get_result();
        }
        else {
            n_result = length;//virtual success.
        }
    } while (false);

    return n_result;
}

int _vhid_api_briage::api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report)
{
    bool b_run(false);
    int n_result(-1);
    int n_primitive(-1);
    _vhid_api_briage::_q_item::type_ptr ptr_item;

    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);

    do {
        std::lock_guard<std::mutex> lock(m_mutex_for_map);

        if (n_primitive == _vhid_info::const_map_index_invalid) {
            _mp::clog::get_instance().log_fmt(L"[E] %ls : invalid map index.\n", __WFUNCTION__);
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_primitive);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            _mp::clog::get_instance().log_fmt(L"[E] %ls : not found map item.\n", __WFUNCTION__);
            continue;
        }

        ptr_item = std::make_shared<_vhid_api_briage::_q_item>(n_map_index);
        ptr_item->setup_for_read(data, length,n_report);

        if (!it->second.second->push(ptr_item)) {
            _mp::clog::get_instance().log_fmt(L"[E] %ls : push().\n", __WFUNCTION__);
            continue;
        }

        b_run = true;
    } while (false);

    do {
        if (!b_run) {
            continue;
        }

        //waits response
        ptr_item->waits();
        //rx data will be save on buf directly.
        _mp::type_v_buffer v_rx(0);
        std::tie(n_result, v_rx) = ptr_item->get_result();

#ifdef _WIN32
#ifdef _DEBUG
        static size_t n_ct(0);
        if (n_result > 2) {
            ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0],rx[1],rx[2]) = (%d, 0x%x, 0x%x, 0x%x)\n", \
                GetCurrentThreadId(), n_map_index,
                _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
                n_ct++, n_result, v_rx[0], v_rx[1], v_rx[2]);
        }
        else if (n_result > 1) {
            ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0],rx[1]) = (%d, 0x%x, 0x%x)\n",\
                GetCurrentThreadId(), n_map_index,
                _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
                n_ct++, n_result, v_rx[0], v_rx[1]);
        }
        else if (n_result > 0) {
            ATLTRACE(L"0x%08x:0x%08X(%s)-RX%u (n_result,rx[0]) = (%d,0x%x)\n",
                GetCurrentThreadId(), n_map_index,
                _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
                n_ct++, n_result, v_rx[0]);
        }
#endif
#endif
    } while (false);

    if (n_result < 0) {
        _mp::clog::get_instance().log_fmt(L"[E] %ls : n_result = %d.\n", __WFUNCTION__, n_result);
#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"0x%08X(%s)-ERROR-RX (n_result) = (%d)\n",
            n_map_index,
            _vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str(),
            n_result);
#endif
#endif
    }


    return n_result;
}

const wchar_t* _vhid_api_briage::api_error(int n_map_index)
{
    int n_primitive(-1);
    std::tie(n_primitive, std::ignore) = _vhid_info::get_primitive_map_index_from_compositive_map_index(n_map_index);
    return _hid_api_briage::api_error(n_map_index);
}

std::tuple<bool, int, bool> _vhid_api_briage::_is_open(const char* path) const
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
        std::tie(b_open, n_primitive_map_index, std::ignore) = _hid_api_briage::is_open(s_primitive.c_str()); //check primitive type

        // the base type is opened ?
        if (!b_open) {
            continue;
        }

        auto it_info = m_map_ptr_hid_info_ptr_worker.find(n_primitive_map_index);
        if (it_info == std::end(m_map_ptr_hid_info_ptr_worker)) {
            b_open = false; // not opened compositive type
            continue;
        }

        //
        b_exclusive_open = !b_support_shared_open;

    } while (false);

    return std::make_tuple(b_open, n_primitive_map_index, b_exclusive_open);

}

/**
* ***************************************************
* _vhid_api_briage::_q_worker member function bodies
*/
_vhid_api_briage::_q_worker::_q_worker(int n_primitive_map_index, _vhid_api_briage* p_api_briage) :
    m_b_run_worker(false),
    m_n_primitive_map_index(n_primitive_map_index)
{
    m_b_run_worker = true;
    m_ptr_worker = std::shared_ptr<std::thread>(new std::thread(&_vhid_api_briage::_q_worker::_worker, this, p_api_briage));
}

_vhid_api_briage::_q_worker::~_q_worker()
{
    if (m_ptr_worker) {
        m_b_run_worker = false;

        if (m_ptr_worker->joinable()) {
            m_ptr_worker->join();
        }
    }
}

bool _vhid_api_briage::_q_worker::push(const _vhid_api_briage::_q_item::type_ptr& ptr_item)
{
    return m_q.push(ptr_item);
}

void _vhid_api_briage::_q_worker::_pop_all(int n_result, const _mp::type_v_buffer& v_rx)
{
    m_q.pop_all(n_result, v_rx);
}

std::pair<bool, bool> _vhid_api_briage::_q_worker::_try_pop(_q_container::type_ptr_list& ptr_list)
{
    return m_q.try_pop(ptr_list);
}

void _vhid_api_briage::_q_worker::_worker(_vhid_api_briage* p_api_briage)
{
#ifdef _WIN32
#ifdef _DEBUG
    ATLTRACE(L" ** Start _q_worker : %08x.\n", GetCurrentThreadId());
#endif
#endif

    bool b_pass_result(false);
    _q_container::type_ptr_list ptr_list;

    while (m_b_run_worker) {

        _mp::type_v_buffer v_rx(0);
        bool b_exsit_data(false);
        bool b_canceled(false);
        bool b_result(false);

        do {
            // main work here!
            // sw 작업자의 수신 요청만 있을때, pop 되고, 그 때, 마침 MSR 데이터가 도착하면, msr 은 데이터를 놓치는 일이 발생하므로. 주의 깊게 동작을 결정해야 한다.
            std::tie(b_exsit_data,b_canceled) = _try_pop(ptr_list);
            if (!b_exsit_data) {
                continue;
            }
/*
#ifdef _WIN32
#ifdef _DEBUG
            if (ptr_list) {
                std::wstring s;

                s = L"POP REQ = [";
                for (auto ptr_d : *ptr_list) {
                    if( !ptr_d){
                        continue;
                    }
                    auto ws_type = _vhid_info::get_type_wstring_from_compositive_map_index( ptr_d->get_map_index() );
                    if (ws_type.empty()) {
                        s += L"#hid";
                    }
                    else {
                        s += ws_type;
                    }
                    s += L".";
                    
                }//end for

                s += L"]\n";
                if(b_canceled)
                    ATLTRACE(L"CANCEL + %s",s.c_str());
                else
                    ATLTRACE(L"%s", s.c_str());
            }
#endif
#endif
*/
            if (b_canceled) {
                b_result = _process_cancel(ptr_list, p_api_briage);
            }
            else {
                std::tie(b_result, v_rx) = _process(ptr_list, p_api_briage);
            }
        } while (false);

        std::this_thread::sleep_for(std::chrono::milliseconds(_vhid_api_briage::_q_worker::_const_worker_interval_mmsec));

    }//end while

    // release all waiter
    _pop_all(0);
#ifdef _WIN32
#ifdef _DEBUG
    ATLTRACE(L"Exit _vhid_api_briage::_q_worker::_worker().\n");
#endif
#endif

}

std::tuple<bool,int> _vhid_api_briage::_q_worker::_process_req(
    const _vhid_api_briage::_q_item::type_ptr & ptr_item,
    _hid_api_briage* p_api_briage,
    _mp::type_ptr_v_buffer & v_ptr_rx,
    const std::wstring& s_debug_msg /*= std::wstring(L"")*/
    )
{
#ifdef _WIN32
#ifdef _DEBUG
    std::wstring s_d, ws_type;
#endif
#endif
    int n_result(-1);
    bool b_complete(true);
    _mp::type_v_buffer v_tx(0);

    v_ptr_rx = std::make_shared<_mp::type_v_buffer>(0);
    do {
        if (!ptr_item) {
            continue;
        }
        //
        size_t n_size_report_in(0);
        //
        _hid_api_briage::type_next_io next_io(_hid_api_briage::next_io_none);

        // setup parameter from request 
        if (ptr_item->get_user_rx_buffer() && ptr_item->get_user_rx_buffer_size() > 0) {
            (*v_ptr_rx).resize(ptr_item->get_user_rx_buffer_size(), 0);
        }

        v_tx = ptr_item->get_tx();
        next_io = ptr_item->get_next_io_type();

        //
        ptr_item->set_start_time();

        switch (ptr_item->get_cmd()) {
        case _vhid_api_briage::_q_item::cmd_set_nonblocking:
            n_result = p_api_briage->_hid_api_briage::api_set_nonblocking(m_n_primitive_map_index, ptr_item->get_param_nonblock());
            break;
        case _vhid_api_briage::_q_item::cmd_get_report_descriptor:
            n_result = p_api_briage->_hid_api_briage::api_get_report_descriptor(m_n_primitive_map_index, &(*v_ptr_rx)[0], (*v_ptr_rx).size());
            break;
        case _vhid_api_briage::_q_item::cmd_write:
            n_result = p_api_briage->_hid_api_briage::api_write(m_n_primitive_map_index, &v_tx[0], v_tx.size(), next_io);
            if (n_result == 0) {
                b_complete = false; // not yet tx
            }
            //tx is success and ready for rx or error
            break;
        case _vhid_api_briage::_q_item::cmd_read:
#ifdef _WIN32
#ifdef _DEBUG
            ws_type = _vhid_info::get_type_wstring_from_compositive_map_index(ptr_item->get_map_index());
            s_d = L"REQ-read : ";
            s_d += ws_type;

            //ATLTRACE(L"%s-%s\n", s_d.c_str(), s_debug_msg.c_str());
#endif
#endif

            n_size_report_in = ptr_item->get_size_report_in();

            (*v_ptr_rx).resize(n_size_report_in, 0);
            n_result = _rx((*v_ptr_rx), p_api_briage);
            if (n_result == 0) {
                // need more receving
                b_complete = false;
                (*v_ptr_rx).resize(0);
            }
            break;
        default:
            break;
        }

    } while (false);

    return std::make_pair(b_complete, n_result);
}

std::pair<bool, _mp::type_v_buffer> _vhid_api_briage::_q_worker::_process(
    _q_container::type_ptr_list& ptr_list,
    _hid_api_briage* p_api_briage
)
{
    bool b_result(false);
    _mp::type_ptr_v_buffer ptr_v_rx;
    int n_map_index(-1);

    do {
        if (!ptr_list) {
            continue;
        }
        if (ptr_list->empty()) {
            continue;
        }

        bool b_complete(true);
        int n_result(-1);
        _hid_api_briage::type_next_io next_io(_hid_api_briage::next_io_none);

        if (ptr_list->size() == 1) {
            //single request
            _vhid_api_briage::_q_item::type_ptr ptr_item(*ptr_list->begin());
            std::tie(std::ignore, n_result) = _process_req(ptr_item, p_api_briage, ptr_v_rx, L"Single");
            _save_rx_to_msr_or_ibutton_buffer_in_single_or_multi_rx_requests(n_result, ptr_v_rx, ptr_item);
            _notify_in_single_or_multi_rx_requests(ptr_item, n_result, ptr_v_rx);
            continue;
        }

        //
        _q_container::type_list::iterator it_list = ptr_list->begin();//get first request of list
        if ((*it_list)->get_next_io_type() == _hid_api_briage::next_io_none) {
            //multi rx request mode with the diffent map index. 
            std::tie(std::ignore, n_result) = _process_req((*it_list), p_api_briage, ptr_v_rx, L"Multi");
            _save_rx_to_msr_or_ibutton_buffer_in_single_or_multi_rx_requests(n_result, ptr_v_rx);

            // notify result to all
            for (_q_item::type_ptr& ptr_req : *ptr_list) {
                _notify_in_single_or_multi_rx_requests(ptr_req, n_result, ptr_v_rx);
            }//end for
            continue;
        }

        //tx_rx mode
        // 장비로 부터 받은 msr 이나 ibutton 데이터가 아직 rx 요청에 의해 전달되지 못해도. 
        // 모두 삭제해야. 엉뚱한 데이터가 어플에게 전송되는 것을 막을 수 있다.
        m_q_result_msr.clear();
        m_q_result_ibutton.clear();
#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"== CLR buffer.\n");
#endif
#endif

        for (_q_item::type_ptr &ptr_req : *ptr_list) {

            if (!ptr_req) {
                continue;
            }

            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] TXRX REQ : %ls.\n", ptr_req->get_cmd_by_string().c_str());

            next_io = ptr_req->get_next_io_type();
            ptr_v_rx.reset();

            do {
                std::tie(b_complete, n_result) = _process_req(ptr_req, p_api_briage, ptr_v_rx, L"TXRX");
                if(!m_b_run_worker ){
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] TXRX REQ : %ls - complete BY TH : %d.\n", ptr_req->get_cmd_by_string().c_str(), n_result);
                    break;//exit while
                }
                if (b_complete) {
                    _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D] TXRX REQ : %ls - complete : %d.\n", ptr_req->get_cmd_by_string().c_str(), n_result);
                    break;// continue while
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(_vhid_api_briage::_q_worker::_const_txrx_pair_tx_interval_mmsec));
            } while (!b_complete && m_b_run_worker);

            if (!m_b_run_worker) {
                n_result = -1;
                ptr_v_rx.reset();

                //notify the last request
                (*ptr_list->back()).set_result(n_result).set_rx(ptr_v_rx);
                (*ptr_list->back()).set_event();
                break; //worker termination 
            }

            if (n_result < 0) {
                ptr_v_rx.reset();

                //notify error
                (*ptr_list->back()).set_result(n_result).set_rx(ptr_v_rx);
                (*ptr_list->back()).set_event();
                break; // exit for
            }

            if (next_io == _hid_api_briage::next_io_none) {
                //notify the last request
                (*ptr_list->back()).set_result(n_result).set_rx(ptr_v_rx);
                (*ptr_list->back()).set_event();
            }
        }//end for

    } while (false);

    if(ptr_v_rx)
        return std::make_pair(b_result, *ptr_v_rx);
    else
        return std::make_pair(b_result, _mp::type_v_buffer());
}

bool _vhid_api_briage::_q_worker::_process_cancel(_q_container::type_ptr_list& ptr_list, _hid_api_briage* p_api_briage)
{
    bool b_result(false);
    _mp::type_v_buffer v_rx(0);
    int n_result(0);

    do {
        if (!ptr_list) {
            continue;
        }
        if (ptr_list->empty()) {
            continue;
        }
        //
        if (ptr_list->size() == 1) {
            // cancel request 하나로만 구성된 list
            (*ptr_list->begin())->set_result(n_result).set_rx(v_rx).set_event();
            b_result = true;
            continue;
        }

        //cancel request 는 항상 list 의 마지막을 있어야함.
        auto it = ptr_list->end();
        --it;
        _q_item::type_ptr ptr_item_cancel(*it);
        _q_item::type_cmd cmd_last(ptr_item_cancel->get_cmd());
        _mp::type_v_buffer v_tx_last(ptr_item_cancel->get_tx());
        int n_map_index_cancel(ptr_item_cancel->get_map_index());

        //마지막이 cancel 명령인지 검사
        if (cmd_last != _q_item::cmd_write) {
            continue; // 에러 cancel request 는 tx size가 0인 write 명령.
        }
        if (!v_tx_last.empty()) {
            continue; // 에러 cancel request 는 tx size가 0인 write 명령.
        }
        //

        // cancel 바로 앞 명령 얻기.
        --it;
        _q_item::type_ptr ptr_item_normal_last(*it);
        _hid_api_briage::type_next_io next_io_normal_last(ptr_item_normal_last->get_next_io_type());
        //
        _q_item::type_ptr ptr_item_first(*ptr_list->begin());
        _q_item::type_cmd cmd_first(ptr_item_first->get_cmd());

        b_result = true;

        if (next_io_normal_last == _hid_api_briage::next_io_none) {
            //complete transaction is canceled.
            if (cmd_first != _q_item::cmd_read) {
                ptr_item_normal_last->set_result(-1,L"complete transaction is canceled").set_rx(v_rx).set_event();
                ptr_item_cancel-> set_result(n_result).set_rx(v_rx).set_event();
                continue;
            }

            // 마지막 request 인 cancel 은 삭제.
            ptr_list->pop_back();
            _q_container::type_list::iterator it_found = std::find_if(std::begin(*ptr_list), std::end(*ptr_list), [&](_q_item::type_ptr & ptr_req)->bool {
                if (ptr_req->get_map_index() == n_map_index_cancel) {
                    return true;
                }
                else {
                    return false;
                }
            });

            if (it_found != ptr_list->end()) {
                (*it_found)->set_result(-1,L"remove cancel").set_rx(v_rx).set_event();
            }
            ptr_item_cancel->set_result(n_result).set_rx(v_rx).set_event();
            continue;
        }

        //net yet completeed transaction
        ptr_item_cancel->set_result(n_result).set_rx(v_rx).set_event();

    } while (false);

    return b_result;
}

bool _vhid_api_briage::_q_worker::_notify_in_single_or_multi_rx_requests(
    _q_item::type_ptr& ptr_req,
    int n_result,
    const _mp::type_ptr_v_buffer& ptr_v_rx
)
{
    // 무작정 기다리는 작업자가 있으므로, 통지를 하지 않으면, 무한 기다림이 발생한다.
    // 따라서 기본 값은 통지.
    bool b_notified(true);
    _mp::type_v_buffer v(0);

    if (ptr_v_rx) {
        v = *ptr_v_rx;
    }

    do {
        if (!ptr_req) {
            b_notified = false;//통지 할 정보가 없음으로 무시.
            continue;
        }
        if (ptr_req->get_cmd() != _vhid_api_briage::_q_item::cmd_read) {
            // read 인 것은 무조건 통지.
            continue;
        }

        int n_map_index = ptr_req->get_map_index();
        _mp::type_bm_dev t = _vhid_info::get_type_from_compositive_map_index(n_map_index);
        bool b_is_ibutton_format(false);
        std::tie(b_is_ibutton_format,std::ignore) = _vhid_info_lpu237::is_rx_ibutton(v);
        _q_worker::_type_pair_result_ptr_rx item(0, _mp::type_ptr_v_buffer());

#ifdef _WIN32
#ifdef _DEBUG
//        ATLTRACE(L"*_*-0x%08x(%s) NOTIFY.\n", n_map_index,_vhid_info::get_type_wstring_from_compositive_map_index(n_map_index).c_str());
#endif
#endif

        switch (t) {
        case _mp::type_bm_dev_hid:
        case _mp::type_bm_dev_lpu200_scr0:
            if (n_result <= 0) {
                //에러나 기다림 필요는 무조건 통지.
                break;
            }

            n_result = 0;
            v.resize(0);
            break; // 이 타입은 txrx 만 지원하므로, pump 지원을 위해 기다림 통지.
            // n_result = 1 ,에러로 통지하면, deep recover 시도해서 상위 작업자 죽이고, 다시 생성해서 문제 발생.
        case _mp::type_bm_dev_lpu200_msr:
            if (m_q_result_msr.empty()) {
                // msr 데이터가 필요한데, 없음.
                // 기다림 필요로 통지.
                n_result = 0;
                v.resize(0);
                break;
            }
            
#ifdef _WIN32
#ifdef _DEBUG
            ATLTRACE(L"-_- ;; NOTIFY - MSR.\n");
#endif
#endif
            item = m_q_result_msr.front();
            m_q_result_msr.pop_front();
            //
            n_result = item.first;
            if (item.second) {
                v = *item.second;
            }
            else {
                v.resize(0);
            }
            break;
        case _mp::type_bm_dev_lpu200_ibutton:
            if (m_q_result_ibutton.empty()) {
                // ibutton 데이터가 필요한데, 없음..
                // 기다림 필요로 통지.
                n_result = 0;
                v.resize(0);
                break;
            }

            item = m_q_result_ibutton.front();
            m_q_result_ibutton.pop_front();
            //
            n_result = item.first;
            if (item.second) {
                v = *item.second;
            }
            else {
                v.resize(0);
            }
            break;
        case _mp::type_bm_dev_lpu200_switch0:
            // 디버그를 위해 그냥 통지.
            break;
        default:
            // 알수 없는 디바이스 타입은 그냥 통지.
            break;
        }//emd switch

    } while (false);

    if (b_notified) {
        ptr_req->set_result(n_result).set_rx(v).set_event();
    }
    return b_notified;
}

void _vhid_api_briage::_q_worker::_save_rx_to_msr_or_ibutton_buffer_in_single_or_multi_rx_requests(
    int n_result,
    const _mp::type_ptr_v_buffer& ptr_v_rx,
    const _q_item::type_ptr& ptr_req /*= _q_item::type_ptr()*/
)
{
    int n_map_index(-1);
    _mp::type_bm_dev t(_mp::type_bm_dev_unknown);

    do {
        if (ptr_req) {
            // single request 의 경우.
            if (ptr_req->get_cmd() != _vhid_api_briage::_q_item::cmd_read) {
                continue;// read 요청에서만 고려할 사항
            }
        }

        if (n_result == 0) {
            // m_q_result_msr 나 m_q_result_ibutton 가 empty 이면, 자동으로 기다림 요청됨.
            continue;
        }

        //
        if (n_result < 0) {
            // read 에러.
            m_q_result_msr.push_back(std::make_pair(n_result, _mp::type_ptr_v_buffer()));
            m_q_result_ibutton.push_back(std::make_pair(n_result, _mp::type_ptr_v_buffer()));
            continue;
        }

        // n_result 가 영 보다 큰 경우, ptr_v_rx 는 무조건 할당되어 값을 가지고 있다.
        bool b_is_ibutton_format(false);
        std::tie(b_is_ibutton_format,std::ignore) = _vhid_info_lpu237::is_rx_ibutton(*ptr_v_rx);
        if (b_is_ibutton_format) {
            m_q_result_ibutton.push_back(std::make_pair(n_result, ptr_v_rx));
            continue;
        }

        m_q_result_msr.push_back(std::make_pair(n_result, ptr_v_rx));
#ifdef _WIN32
#ifdef _DEBUG
         ATLTRACE(L" MSR : pushed rx.(q size = %u)\n", m_q_result_msr.size());
#endif
#endif

    } while (false);
}

int _vhid_api_briage::_q_worker::_rx(_mp::type_v_buffer& v_rx, _hid_api_briage* p_api_briage)
{
    int n_result(0);
    int n_offset(0);
    int n_len = (int)v_rx.size();
    int n_loop = 0;
    int n_retry = 0;

    do {
        ++n_loop; //for debugging

        n_result = p_api_briage->_hid_api_briage::api_read(m_n_primitive_map_index, &v_rx[n_offset], n_len, v_rx.size());
        if (n_result < 0) {
            break;// error 
        }

        if (n_result == 0 ) {
            if (n_offset == 0) {
                break;// need read more.
            }
            else {
                //self loop
                ++n_retry;
                if (n_retry >= _q_worker::_const_one_packet_of_in_report_retry_counter) {
                    // fail rx over retry. 하나의 in-report 를 구성하는 packet 사이의 간격이 
                    //_const_one_packet_of_in_report_retry_counter*_const_txrx_pair_rx_interval_mmsec 를 초과.
                    n_result = -1;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(_vhid_api_briage::_q_worker::_const_txrx_pair_rx_interval_mmsec));
                continue;
            }
        }
        //
        n_retry = 0;

        if ((n_result + n_offset) >= v_rx.size()) {
            n_result = n_result + n_offset;
            _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D%d] RX-OK : (n_offset, n_read)=(%d,%d,%u).\n", n_loop, n_offset, n_result, v_rx.size());
            break; // read complete
        }
        //
        _mp::clog::get_instance().log_fmt_in_debug_mode(L"[D%d] RX : (n_offset, n_read)=(%d,%d,%u).\n", n_loop, n_offset, n_result, v_rx.size());

        n_offset = n_offset + n_result;
        n_len = n_len - n_result;

        std::this_thread::sleep_for(std::chrono::milliseconds(_vhid_api_briage::_q_worker::_const_txrx_pair_rx_interval_mmsec));
    } while (m_b_run_worker);
    //
    return n_result;
}


/**
* ***************************************************
* _vhid_api_briage::_q_item member function bodies
*/

_vhid_api_briage::_q_item::_q_item(int n_compositive_map_index) : m_n_compositive_map_index(n_compositive_map_index)
{
    m_next = _hid_api_briage::next_io_none;
    m_n_in_report = 0;
    m_n_rx_timeout_mmsec = 0;

    m_cmd = _q_item::cmd_undefined;
    m_n_nonblock = 0;
    m_n_milliseconds = -1;
    m_n_rx = 0;

    m_n_result = -1;
    m_n_event = -1;
    m_ps_rx = nullptr;
}


void _vhid_api_briage::_q_item::setup_for_set_nonblocking(int nonblock)
{
    m_v_rx.resize(0);
    m_ps_rx = nullptr;

    m_cmd = _q_item::cmd_set_nonblocking;
    m_v_tx.resize(0);
    //
    m_n_nonblock = nonblock;
    
    //
    m_n_result = -1;
    m_n_event = m_waiter.generate_new_event();
}
void _vhid_api_briage::_q_item::setup_for_get_report_descriptor(unsigned char* buf, size_t buf_size)
{
    m_n_rx = buf_size;
    m_v_rx.resize(buf_size,0);
    m_ps_rx = buf;

    m_cmd = _q_item::cmd_get_report_descriptor;
    m_v_tx.resize(0);
    //
    m_n_result = -1;
    m_n_event = m_waiter.generate_new_event();
}
void _vhid_api_briage::_q_item::setup_for_write(const unsigned char* data, size_t length, _hid_api_briage::type_next_io next)
{
    m_v_rx.resize(0);
    m_ps_rx = nullptr;
    m_next = next;

    m_cmd = _q_item::cmd_write;
    if (length > 0 && data != NULL) {
        m_v_tx.resize(length, 0);
        std::copy(&data[0], &data[length], m_v_tx.begin());
    }
    else {
        m_v_tx.resize(0);
    }
    //
    m_n_result = -1;
    m_n_event = m_waiter.generate_new_event();
}
void _vhid_api_briage::_q_item::setup_for_read(unsigned char* data, size_t length, size_t n_report)
{
    m_n_rx = length;
    m_v_rx.resize(length, 0);
    m_ps_rx = data;
    m_n_in_report = n_report;

    m_cmd = _q_item::cmd_read;
    m_v_tx.resize(0);
    //
    m_n_result = -1;
    m_n_event = m_waiter.generate_new_event();
}

_vhid_api_briage::_q_item::~_q_item()
{
}

_vhid_api_briage::_q_item::type_cmd _vhid_api_briage::_q_item::get_cmd() const
{
    return m_cmd;
}

std::wstring _vhid_api_briage::_q_item::get_cmd_by_string() const
{
    std::wstring s;

    switch (m_cmd) {
    case cmd_undefined:
        s = L"cmd_undefined";
        break;
    case cmd_set_nonblocking:
        s = L"cmd_set_nonblocking";
        break;
    case cmd_get_report_descriptor:
        s = L"cmd_get_report_descriptor";
        break;
    case cmd_write:
        s = L"cmd_write";
        break;
    case cmd_read:
        s = L"cmd_read";
        break;
    default:
        s = L"undefined";
        break;
    }
    return s;
}

_hid_api_briage::type_next_io _vhid_api_briage::_q_item::get_next_io_type() const
{
    return m_next;
}
_mp::type_v_buffer _vhid_api_briage::_q_item::get_tx() const
{
    return m_v_tx;
}

size_t _vhid_api_briage::_q_item::get_rx_size() const
{
    return m_v_rx.size();
}

_vhid_api_briage::_q_item& _vhid_api_briage::_q_item::set_rx(const _mp::type_ptr_v_buffer& ptr_v_rx)
{
    if (ptr_v_rx) {
        return set_rx(*ptr_v_rx);
    }
    else {
        return set_rx(_mp::type_v_buffer());
    }
}
_vhid_api_briage::_q_item& _vhid_api_briage::_q_item::set_rx(const _mp::type_v_buffer& v_rx)
{
    do {
        if (m_v_rx.empty()) {
            continue;
        }
        size_t n_rx = m_v_rx.size();
        if (n_rx >= v_rx.size()) {//overflower buffer
            m_v_rx = v_rx;
            if (m_ps_rx == nullptr) {
                continue;
            }
            std::copy(m_v_rx.begin(), m_v_rx.end(), &m_ps_rx[0]);
            continue;
        }
        //
        std::copy(v_rx.begin(), v_rx.begin()+ m_v_rx.size(), m_v_rx.begin());
        if (m_ps_rx == nullptr) {
            continue;
        }
        std::copy(m_v_rx.begin(), m_v_rx.end(), &m_ps_rx[0]);

    } while (false);
    return *this;
}


_vhid_api_briage::_q_item& _vhid_api_briage::_q_item::append_rx(const _mp::type_v_buffer& v_rx, bool b_add_to_head/* = false*/)
{
    if (b_add_to_head) {
        m_v_rx.insert(m_v_rx.begin(), v_rx.begin(), v_rx.end());
    }
    else {
        std::copy(v_rx.begin(), v_rx.end(), std::back_inserter(m_v_rx));
    }

    if (m_ps_rx != nullptr) {
        if (m_n_rx >= m_v_rx.size()) {
            std::copy(m_v_rx.begin(), m_v_rx.end(), &m_ps_rx[0]);
        }
        else {
            std::copy(m_v_rx.begin(), m_v_rx.begin()+ m_n_rx, &m_ps_rx[0]);
        }
    }

    return *this;
}

_vhid_api_briage::_q_item& _vhid_api_briage::_q_item::set_result(int n_result, const std::wstring& s_debug_msg/*=std::wstring()*/)
{
    m_n_result = n_result;

#ifdef _WIN32
#ifdef _DEBUG
    if (n_result <0 ) {
        ATLTRACE(L"error set_result - %s.\n", s_debug_msg.c_str());
        //std::wcout << L"error set_result - " << s_debug_msg << std::endl;
    }
#endif
#endif

    return *this;
}

size_t _vhid_api_briage::_q_item::get_user_rx_buffer_size() const
{
    return m_n_rx;
}

unsigned char* _vhid_api_briage::_q_item::get_user_rx_buffer() const
{
    return m_ps_rx;
}

int _vhid_api_briage::_q_item::get_param_nonblock() const
{
    return m_n_nonblock;
}

int _vhid_api_briage::_q_item::get_map_index() const
{
    return m_n_compositive_map_index;
}

size_t _vhid_api_briage::_q_item::get_size_report_in() const
{
    return m_n_in_report;
}

int _vhid_api_briage::_q_item::get_rx_timeout_mmsec() const
{
    return m_n_milliseconds;
}

void _vhid_api_briage::_q_item::set_start_time()
{
    m_start_time = std::chrono::high_resolution_clock::now();
}

double _vhid_api_briage::_q_item::get_elapsed_usec_time()
{
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - m_start_time).count();
    return duration / 1e3; //unit usec
}

std::pair<int, _mp::type_v_buffer> _vhid_api_briage::_q_item::get_result() const
{
    return std::make_pair(m_n_result,m_v_rx);
}

bool _vhid_api_briage::_q_item::waits()
{
    bool b_result(false);

    do {
        m_waiter.wait_for_at_once();
        b_result = true;
        //
    } while (false);
    return b_result;
}

void _vhid_api_briage::_q_item::set_event()
{
    m_waiter.set(m_n_event);
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// _q_container member
_vhid_api_briage::_q_container::_q_container()
{
}

_vhid_api_briage::_q_container::~_q_container()
{
}

bool _vhid_api_briage::_q_container::push( const _vhid_api_briage::_q_item::type_ptr& ptr_item)
{
    bool b_result(false);

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!ptr_item) {
            continue;
        }
        //
        b_result = true;
        //
        _q_container::type_req_list_status status_transaction(_q_container::st_transaction);
        _q_item::type_cmd cmd(ptr_item->get_cmd());
        _hid_api_briage::type_next_io next_type(ptr_item->get_next_io_type());
        int n_map_index(ptr_item->get_map_index());
        _mp::type_v_buffer v_tx(ptr_item->get_tx());

        if (next_type != _hid_api_briage::next_io_none) {
            //하나의 transaction 의 마지막 request 는 next_type 이 next_io_none 임.
            status_transaction = _q_container::st_not_yet_transaction;
        }
        else if ((cmd == _q_item::cmd_write) && (v_tx.empty())) {
            //보낼 데이터가 없는 comd_write 명령은 해당 map_index 의 명령을 취소 하는 요청.
            status_transaction = _q_container::st_cancel;
        }

        // 새로운 request 의 list 생성.
        _q_container::type_ptr_list ptr_list(new _q_container::type_list());
        ptr_list->push_back(ptr_item);

        // q 에 넣을 요소 생성.
        auto new_item = std::make_pair(status_transaction, ptr_list);
        //
        if (m_q_pair.empty()) {
            // q 가 빈 경우.
            m_q_pair.push_back(new_item);
            _dump_map();// for debuggig
            continue;
        }
        
        if (status_transaction == _q_container::st_cancel) {
            // cancel request mode.
            bool b_cancel_pushed(false);

            // 현재 저장된 request list 의 queue 에서 cancel 요청이 온 map_index 값을 갖는 request 가 있는 list 를 찾는다.
            for (_q_container::type_pair_st_ptr_list& item_q : m_q_pair) {
                if (item_q.first == _q_container::st_cancel) {
                    continue; //cancel 된 request list 에 또 다시 cancel request 를 추가 할수 없다.
                }

                // 현재 item_q 의 list 모든 요소에서 새로운 ptr_item 과 map index 가 동일한 요소를 찾는다.
                auto it_q_cancel = std::find_if(item_q.second->begin(), item_q.second->end(), [&](_q_item::type_ptr& item)->bool {
                    if (item->get_map_index() == n_map_index) {
                        return true;
                    }
                    else {
                        return false;
                    }
                });

                if (it_q_cancel == item_q.second->end()) {
                    continue; // 현재 item_q 의 list 모든 요소에 새로운 ptr_item 과 map index 가 동일한 요소가 없다.
                }
                
                // 현재 item_q 의 list 모든 요소에 새로운 ptr_item 과 map index 가 동일한 요소가
                // 있으면, item_q 상태를 cancel 로 바꾸고, 그 끝에 새로 받은 cancel request 를 추가.
                item_q.first = _q_container::st_cancel; // this transaction is canceled
                item_q.second->push_back(ptr_item); //push cancel request
                b_cancel_pushed = true;
                break; // exit for
            }//end for

            if (!b_cancel_pushed) {
                //cancel request 1개 만 있는 list 생성 필요.
                // cancel request 는  write 명령의 변경이고, write 명령은 push 하고, 결과를 wait 하기 때문에 실제 push 가 필요.
                m_q_pair.push_back(new_item);
                _dump_map();
            }

            continue;//end cancel mode
        }

        //
        //back search - 뒤로 부터 찾는 이유는 가장 최근에 push 된 request list 에 request 가 추가 될 확률이 높음.
        auto it_q = std::find_if(m_q_pair.rbegin(), m_q_pair.rend(), [&](_q_container::type_pair_st_ptr_list & item)->bool{
            // return 이 true 일 조건.
            // 1. item 이 complete 이고, list 의 모든 cmd 가 _q_item::cmd_read, 그리고 b_complete_transaction 도 true
            // 2. item 이 not complete 이고, list 의 모든 map index 가 n_map_index 와 동일.
            // check complete transaction.
            bool b_found(false);

            do {
                if (item.first == _q_container::st_cancel) {
                    continue;//cancel 된 request list 에 request 를 추가할 수 없음
                }
                if (item.first == _q_container::st_not_yet_transaction) {
                    if (item.second->back()->get_map_index() == n_map_index) {
                        b_found = true;
                    }
                    continue;
                }

                // item.first == _q_container::st_transaction
                bool b_all_cmd_read(true);
                for (auto req : *item.second) {
                    if (req->get_cmd() != _q_item::cmd_read) {
                        b_all_cmd_read = false;
                        break;// exit for
                    }
                }//end for list

                if (!b_all_cmd_read) {
                    continue;//pass complete transaction
                }
                if (status_transaction != _q_container::st_transaction) {
                    continue;
                }
                if (cmd != _q_item::cmd_read) {
                    continue;
                }

                b_found = true;
            } while (false);
            return b_found;

        });

        if (it_q == m_q_pair.rend()) {
            //add new transaction
            m_q_pair.push_back(new_item);
            _dump_map();
            continue;
        }

        it_q->first = status_transaction;
        it_q->second->push_back(ptr_item);
        _dump_map();
        //
    } while (false);

    return b_result;
}

void _vhid_api_briage::_q_container::_dump_map()
{
    // dump m_q_pair for debugging
    /*
#ifdef _WIN32
#ifdef _DEBUG
    ATLTRACE(L"==============\n");

    int i = 0;

    for (_q_container::type_pair_st_ptr_list& item_q : m_q_pair) {
        ATLTRACE(L"-%d------\n",++i);
        switch (item_q.first) {
        case st_transaction: 
            ATLTRACE(L"st : st_transaction\n");
            break;
        case st_not_yet_transaction:
            ATLTRACE(L"st : st_not_yet_transaction\n");
            break;
        case st_cancel:
            ATLTRACE(L"st : st_cancel\n");
            break;
        default:
            ATLTRACE(L"st : undefined\n");
            break;
        }//end switch 
        if (!item_q.second) {
            ATLTRACE(L"\t none list.\n");
            continue;
        }
        //
        for (auto item : *item_q.second) {
            ATLTRACE(L"\t (index,req)=(%x,%s).\n", item->get_map_index(),item->get_cmd_by_string().c_str());
        }//end for list

    }//end for
#endif
#endif
    */
}
void _vhid_api_briage::_q_container::pop_all(int n_result, const _mp::type_v_buffer& v_rx)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_q_pair.empty()) {
            continue;
        }

        //
        for (auto it = m_q_pair.begin(); it != m_q_pair.end(); ++it) {
            if (it->first == _q_container::st_not_yet_transaction) {
                continue;
            }
            //하나의 트랜젝션을 구성할 경우 또는 cancel 된 경우, result 를 알림.
            it->second->back()->set_result(n_result).set_rx(v_rx);
            it->second->back()->set_event();
        }//end for

        m_q_pair.clear();
    } while (false);
}

std::pair<bool, bool> _vhid_api_briage::_q_container::try_pop(_q_container::type_ptr_list& ptr_list)
{
    bool b_exsit_data(false);
    bool b_canceled(false);
    //clear output.
    if (ptr_list) {
        ptr_list.reset();
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    //
    for (auto it = m_q_pair.begin(); it != m_q_pair.end(); ++it) {
        if (it->first == _q_container::st_transaction || it->first == _q_container::st_cancel) {
            //complete transaction
            ptr_list = it->second;
            if (it->first == _q_container::st_cancel) {
                b_canceled = true;
            }
            it = m_q_pair.erase(it);
            b_exsit_data = true;
            break;// exit for
        }
    }//end for

    return std::make_pair(b_exsit_data, b_canceled);
}
