#include <chrono>
#include <algorithm>
#include <mp_elpusk.h>

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
            continue;
        }

        auto it = m_map_ptr_hid_info_ptr_worker.find(n_map_index);
        if (it == std::end(m_map_ptr_hid_info_ptr_worker)) {
            continue;
        }

        ptr_item = std::make_shared<_vhid_api_briage::_q_item>(n_map_index);
        ptr_item->setup_for_read(data, length,n_report);
#ifdef _WIN32
#ifdef _DEBUG
        //ATLTRACE(L"0x%08X-read(%u)\n", n_map_index, length);
#endif
#endif

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
        _mp::type_v_buffer v_rx(0);
        std::tie(n_result, v_rx) = ptr_item->get_result();

#ifdef _WIN32
#ifdef _DEBUG
        ATLTRACE(L"0x%08X-RX (n_result,size) = (%d,%d)\n", n_map_index, n_result, v_rx.size());
#endif
#endif
    } while (false);

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
    bool b_result(false);
    do {
        if (!ptr_item) {
            continue;
        }

        int n_composite_map_index = ptr_item->get_map_index();

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_map_ptr_q.find(n_composite_map_index);
        if (it == std::end(m_map_ptr_q)) {
            m_map_ptr_q[n_composite_map_index] = std::make_shared<_vhid_api_briage::_q_worker::_type_q>();
            m_map_ptr_q[n_composite_map_index]->push_back(ptr_item);
        }
        else {
            it->second->push_back(ptr_item);
        }

        auto it_index = std::find(m_q_map_index.begin(), m_q_map_index.end(), n_composite_map_index);
        if (it_index == std::end(m_q_map_index)) {
            m_q_map_index.push_back(n_composite_map_index);//add new request with lowest priority.
        }

        b_result = true;
    } while (false);

    return b_result;
}

void _vhid_api_briage::_q_worker::_pop_all(int n_result, const _mp::type_v_buffer& v_rx/*= _mp::type_v_buffer(0)*/)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_map_ptr_q.empty()) {
            continue;
        }
        //
        for (auto it = m_map_ptr_q.begin(); it != m_map_ptr_q.end(); ++it) {

            _vhid_api_briage::_q_worker::_type_ptr_q _ptr_q(it->second);
            if (!_ptr_q) {
                continue;
            }

            for (_vhid_api_briage::_q_item::type_ptr& _ptr_item : *_ptr_q) {
                if (!_ptr_item) {
                    continue;
                }

                _ptr_item->set_result(n_result).set_rx(v_rx);
                _ptr_item->set_event();
            }//end for


        }//end for
    } while (false);
}

bool _vhid_api_briage::_q_worker::_try_pop(_q_worker::_type_map_ptr_q& map_ptr_q)
{
    bool b_exsit_data(false);
    do {
        map_ptr_q.clear();

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_q_map_index.empty()) {
            continue;
        }
        //m_map_ptr_q 에 있는 q 중에 하나의 트랜잭션을 완성할수 있는q의 key값들을 찾는다.( q 의 마지작 item 의 get_next_io_type() 값이 _hid_api_briage::next_io_none 인 Q )

        for (auto it = m_map_ptr_q.begin(); it != m_map_ptr_q.end(); ++it) {

            _vhid_api_briage::_q_worker::_type_ptr_q _ptr_q(it->second);
            if (!_ptr_q) {
                continue;
            }

            bool b_found(false);
            _vhid_api_briage::_q_worker::_type_q q;
            for (_vhid_api_briage::_q_item::type_ptr& _ptr_item : *_ptr_q) {
                if (!_ptr_item) {
                    continue;
                }
                if (_ptr_item->get_next_io_type() == _hid_api_briage::next_io_none) {
                    q.push_back(_ptr_item);
                    b_found = true;
                    break;
                }
                else {
                    q.push_back(_ptr_item);
                    continue;//for debugging
                }
            }//end for

            if (b_found) {
                b_exsit_data = true;

                map_ptr_q.emplace(it->first, std::make_shared<_vhid_api_briage::_q_worker::_type_q>(q));
                //
                for (auto it_rm = _ptr_q->begin(); it_rm != _ptr_q->end();) {
                    if ((*it_rm)->get_next_io_type() == _hid_api_briage::next_io_none) {
                        it_rm = _ptr_q->erase(it_rm);
                        break;
                    }
                    else {
                        it_rm = _ptr_q->erase(it_rm);
                    }
                }//end for

                if (_ptr_q->empty()) {
                    it = m_map_ptr_q.erase(it);
                    if (it == m_map_ptr_q.end()) {
                        break;//exit for
                    }
                }
            }
        }//end for

        //
    } while (false);
    return b_exsit_data;
}

void _vhid_api_briage::_q_worker::_worker(_vhid_api_briage* p_api_briage)
{
    int n_result(-1);
    bool b_pass_result(false);
    _vhid_api_briage::_q_worker::_type_map_ptr_q map_ptr_q;

    while (m_b_run_worker) {

        _vhid_api_briage::_q_item::type_ptr ptr_item_cur, ptr_item_new;
        _vhid_api_briage::_q_worker::_type_ptr_q ptr_dq_item_cur;
        
        map_ptr_q.clear();

        //int n_req(0);
        _mp::type_v_buffer v_rx(0);
        int n_result(-1);
        bool b_need_more_data_for_complete_transaction(false);
        bool b_exsit_data(false);

        do {
            // main work here!
            ptr_dq_item_cur.reset();
            b_exsit_data = _try_pop(map_ptr_q);
            if (!b_exsit_data) {
                continue;
            }

            do {
                //in or cmd_read command, b_pass_result can be enabled!
                std::tie(n_result, v_rx, b_pass_result) = _worker_on_idle_status(map_ptr_q, p_api_briage);
            } while (b_pass_result);

#ifdef _WIN32
#ifdef _DEBUG
            if (v_rx.size() > 0) {
                ATLTRACE(L"n_result = %d : rx = 0x%X\n", n_result, v_rx[0]);
            }
#endif
#endif
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

void _vhid_api_briage::_q_worker::_process_req_and_erase(
    _vhid_api_briage::_q_worker::_type_map_ptr_q& cur_map_ptr_q,
    _hid_api_briage* p_api_briage,
    _vhid_api_briage::_q_item::type_cmd cmd
)
{
    int n_result(-1);
    _mp::type_v_buffer v_rx(0), v_tx;
    unsigned char* ps_rx(nullptr);
    size_t n_rx(0);
    size_t n_size_report_in(0);
    size_t n_tx = 0;
    unsigned char* ps_tx = 0;
    const double d_min_process_usec_at_success = 10.0;
    bool b_already_received_in_rx_only(false);
    int n_result_for_rx_only(0);
    _mp::type_v_buffer v_rx_for_rx_only;
    int n_txrx_pair_index = _vhid_info::const_map_index_invalid;
	int n_cur_map_index = _vhid_info::const_map_index_invalid;


    for (auto it = cur_map_ptr_q.begin(); it != cur_map_ptr_q.end(); ) {

		n_cur_map_index = it->first;
        // _index_ptr_q_cur.second 에는 없던가, 하나의 트랜잭션을 완성하는 item 만 있음. 
        if (!it->second) {
            it = cur_map_ptr_q.erase(it);
            continue;
        }
        if (it->second->empty()) {
            it = cur_map_ptr_q.erase(it);
            continue;
        }
        //
        bool b_processed(false);
        bool b_tx_rx_mode(false);
        _hid_api_briage::type_next_io next_io(_hid_api_briage::next_io_none);

        for (auto _pptr_item_cur = it->second->begin(); _pptr_item_cur != it->second->end(); ++_pptr_item_cur) {
            if (cmd != (*_pptr_item_cur)->get_cmd()) {
                continue;
            }

            v_rx.resize(0);
            ps_rx = nullptr;
            n_rx = 0;

            if ((*_pptr_item_cur)->get_user_rx_buffer() && (*_pptr_item_cur)->get_user_rx_buffer_size() > 0) {
                v_rx.resize((*_pptr_item_cur)->get_user_rx_buffer_size(), 0);
                ps_rx = &v_rx[0];
                n_rx = v_rx.size();
            }
            v_tx = (*_pptr_item_cur)->get_tx();
            ps_tx = nullptr;
            n_tx = 0;
            if (!v_tx.empty()) {
                ps_tx = &v_tx[0];
                n_tx = v_tx.size();
            }

            next_io = (*_pptr_item_cur)->get_next_io_type();

            (*_pptr_item_cur)->set_start_time();

            //
            switch ((*_pptr_item_cur)->get_cmd()) {
            case _vhid_api_briage::_q_item::cmd_set_nonblocking:
                 n_result = p_api_briage->_hid_api_briage::api_set_nonblocking(m_n_primitive_map_index, (*_pptr_item_cur)->get_param_nonblock());
                 b_processed = true;
                break;
            case _vhid_api_briage::_q_item::cmd_get_report_descriptor:
                n_result = p_api_briage->_hid_api_briage::api_get_report_descriptor(m_n_primitive_map_index, ps_rx, n_rx);
                b_processed = true;
                break;
            case _vhid_api_briage::_q_item::cmd_write:
                n_result = p_api_briage->_hid_api_briage::api_write(m_n_primitive_map_index, ps_tx, n_tx, next_io);

                if (next_io == _hid_api_briage::next_io_read) {
                    if (n_result < 0) {
						b_processed = true;//tx error
                    }
					else {//tx is success nad ready for rx
                        b_tx_rx_mode = true;
                        cmd = _vhid_api_briage::_q_item::cmd_read; // setting for next loop
						n_txrx_pair_index = n_cur_map_index;//get compositive index of tx request.
                    }
                }
                else {
                    b_processed = true;//tx only.
                }
                break;
            case _vhid_api_briage::_q_item::cmd_read:
                n_size_report_in = (*_pptr_item_cur)->get_size_report_in();
                if (b_tx_rx_mode) {
                    n_result = _receive_for_txrx_pair(p_api_briage, ps_rx, n_rx, n_size_report_in);
                    b_processed = true;
                    break;
                }

                if (n_rx == n_size_report_in) {
                    //the first phase of rx transaction. 
                }

                if (!b_already_received_in_rx_only) {
                    v_rx_for_rx_only.resize(n_size_report_in, 0);
                    n_result_for_rx_only = p_api_briage->_hid_api_briage::api_read(m_n_primitive_map_index, &v_rx_for_rx_only[0], v_rx_for_rx_only.size(), v_rx_for_rx_only.size());
                    if (n_result_for_rx_only < 0) {
                        v_rx_for_rx_only.resize(0);
                    }
                    else {
                        v_rx_for_rx_only.resize(n_result_for_rx_only);
                    }

                    b_already_received_in_rx_only = true;//notify all rx requestes.
                    //
                    if ((*_pptr_item_cur)->get_elapsed_usec_time() < d_min_process_usec_at_success) {
                        //pass rx data
                    }
                }

                v_rx = v_rx_for_rx_only;
                n_result = n_result_for_rx_only;
                b_processed = true;
                break;
            default:
                break;
            }

            if (!b_processed) {
                if (!m_b_run_worker) {
                    // kill thread.......
                    (*_pptr_item_cur)->set_result(-1).set_rx(_mp::type_v_buffer(0));
                    (*_pptr_item_cur)->set_event();
                }
                continue;
            }

            //notifiy result
            (*_pptr_item_cur)->set_result(n_result).set_rx(v_rx);
            (*_pptr_item_cur)->set_event();

        }//end for item

        if (!b_processed) {
            ++it;
            continue;
        }

        it = cur_map_ptr_q.erase(it);//the processed request is removed from map.
    }//end for Q
}
int _vhid_api_briage::_q_worker::_receive_for_txrx_pair(_hid_api_briage* p_api_briage, unsigned char* ps_rx, size_t n_rx, size_t n_size_report_in)
{
    int n_result(0);
    int n_total(0);

    unsigned char* _ps_rx = ps_rx;
    size_t _n_rx = n_rx;

    do {
        n_result = p_api_briage->_hid_api_briage::api_read(m_n_primitive_map_index,_ps_rx, _n_rx, n_size_report_in);
        if (n_result < 0) {
            break;// error
        }
        if (n_result == 0) {
            if (m_b_run_worker)
                continue;
            else
                break;// worker must be terminated!
        }
        //
        n_total += n_result;
        if (n_result >= n_rx) {
            n_result = n_total;
            break; //success
        }

        _ps_rx = &_ps_rx[n_result];
        _n_rx = _n_rx - (size_t)n_result;

        std::this_thread::sleep_for(std::chrono::milliseconds(_vhid_api_briage::_q_worker::_const_txrx_pair_tx_interval_mmsec));
    } while (true);
    return n_result;
}

/**
* @brief this method will be called by _worker() on idle status
* @return first - int result value, second rx data, third - true(pass), false(use result)
*/
std::tuple<int, _mp::type_v_buffer, bool> _vhid_api_briage::_q_worker::_worker_on_idle_status
(
    const _vhid_api_briage::_q_worker::_type_map_ptr_q& cur_map_ptr_q, 
    _hid_api_briage* p_api_briage
)
{
    int n_result(-1);
    _mp::type_v_buffer v_rx(0), v_tx;
    unsigned char* ps_rx(nullptr);
    size_t n_rx(0);
    size_t n_tx = 0;
    unsigned char* ps_tx = 0;
    const double d_min_process_usec_at_success = 1000.0;
    bool b_need_check_min_processing_time(false);
    double d_elapsed_usec(0.0);
    bool b_pass_result(false);
    _vhid_api_briage::_q_worker::_type_map_ptr_q _cur_map_ptr_q(cur_map_ptr_q);

    do {
        if (_cur_map_ptr_q.empty()) {
            continue;
        }
        // 아래 순서는 처리 우선순위에 따라 조절됨.
        // cmd_set_nonblocking 는 디바이스가 아니라 lib 에 저장된 설정을 변경하는 것으로 최 우선.
        // cmd_get_report_descriptor 는 디바이스로 부터 report descriptor 를 받아오는 ctl transfer 을 통한 write/read pair로. request 에 대한 response 가 맞게 처리해야함.
        // cmd_write 는 디바이스로 전송만 하는 것과, write/read pair로 되어 있는 것이 있어서, cmd_read 보다 먼저 처리해야 한다.
        // cmd_read 는 디바이스로 부터 수신하는 것만.  엄청 위험 & 복잡.
        _process_req_and_erase(_cur_map_ptr_q, p_api_briage, _vhid_api_briage::_q_item::cmd_set_nonblocking);
        _process_req_and_erase(_cur_map_ptr_q, p_api_briage, _vhid_api_briage::_q_item::cmd_get_report_descriptor);
        _process_req_and_erase(_cur_map_ptr_q, p_api_briage, _vhid_api_briage::_q_item::cmd_write);
        _process_req_and_erase(_cur_map_ptr_q, p_api_briage, _vhid_api_briage::_q_item::cmd_read);
        //
    } while (false);

    if (b_need_check_min_processing_time && n_result > 0) {
        if (d_elapsed_usec < d_min_process_usec_at_success) {
            //n_result = 0; // pass this response. this may be response of prevous dummy response! 
            //v_rx.resize(0);
            //b_pass_result = true;
        }

    }

    return std::make_tuple(n_result, v_rx, b_pass_result);
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
    m_v_tx.resize(length,0);
    std::copy(&data[0], &data[length], m_v_tx.begin());
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

_vhid_api_briage::_q_item& _vhid_api_briage::_q_item::set_result(int n_result)
{
    m_n_result = n_result;
    return *this;
}

size_t _vhid_api_briage::_q_item::get_user_rx_buffer_size() const
{
    return m_n_rx;
}

unsigned char* _vhid_api_briage::_q_item::get_user_rx_buffer()
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
