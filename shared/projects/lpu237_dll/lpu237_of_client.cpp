#include <websocket/mp_win_nt.h>
#include <mp_casync_result_manager.h>
#include <mp_clog.h>


#include "tg_lpu237_dll.h"
#include "lpu237_of_client.h"

lpu237_of_client::lpu237_of_client() :  i_device_of_client()
{
}

lpu237_of_client::lpu237_of_client(unsigned long n_client_index, const std::wstring& s_device_path)
    : i_device_of_client(n_client_index, s_device_path)
{
}

lpu237_of_client::~lpu237_of_client()
{
    //don't call get_class_name() : virtual function
}

lpu237_of_client::lpu237_of_client(const lpu237_of_client& src) : i_device_of_client(src)
{
}

bool lpu237_of_client::cmd_enter_config()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get(cprotocol_lpu237::cmd_enter_cs);
}

bool lpu237_of_client::cmd_leave_config()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get(cprotocol_lpu237::cmd_leave_cs);
}

bool lpu237_of_client::cmd_enter_opos()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get( cprotocol_lpu237::cmd_enter_opos);
}

bool lpu237_of_client::cmd_leave_opos()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get( cprotocol_lpu237::cmd_leave_opos);
}

bool lpu237_of_client::cmd_bypass(const _mp::type_v_buffer& v_tx, _mp::type_v_buffer& v_rx)
{
    bool b_result(false);
    int n_result_index(-1);
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (is_null_device())
            continue;
        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        //
        v_rx.resize(0);

        if (!m_protocol.generate_bypass_uart(v_tx))
            continue;

        unsigned long n_result_code(0);

        _mp::type_v_buffer v_out_packet(0);
        _mp::type_v_buffer v_in_packet(0);
        size_t n_remainder_transaction(0);
        bool b_last = false;
        b_result = true;
        do {
            v_out_packet.resize(0);
            n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);
            if (n_remainder_transaction == 0) {
                //b_last = true;
            }
            if (v_out_packet.size() == 0)
                break;//write complete
            n_result_index = _create_async_result_for_transaction();
            if (n_result_index < 0) {
                b_result = false;
                break;
            }

            if (b_last) {
                if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0,0, v_out_packet)) {
                    b_result = false;
                    break;
                }
            }
            else {
                if (!capi_client::get_instance().write(m_n_client_index, m_n_device_index, 0, v_out_packet)) {
                    b_result = false;
                    break;
                }
            }
            _mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_transaction(n_result_index);
            if (!ptr_async_parameter_result->waits(lpu237_of_client::_const_default_mmsec_timeout_of_response)) {
                b_result = false;
                break;
            }

            if (!ptr_async_parameter_result->get_result(v_in_packet)) {
                b_result = false;
                break;
            }
            remove_async_result_for_transaction(n_result_index);
        } while (true);

        m_protocol.clear_transaction();
        remove_async_result_for_transaction(n_result_index);
        if (!b_result)
            continue;

        //write complete.

        // starts reading
        b_result = true;
        unsigned c_chain(0);
        unsigned long dw_rx_transaction_counter(0);
        unsigned long dw_rx_total(0);
        _mp::type_v_buffer rxbuffer(0);
        unsigned long n_remainder(0);
        unsigned long n_rx(0);
        unsigned char c_rx(0);
        unsigned long dw_rx_offset(0);
        unsigned long n_result_scr(0);
        bool b_re_receive(false);
        do {
            b_re_receive = false;
            v_in_packet.resize(0);
            n_result_index = _create_async_result_for_transaction();
            if (n_result_index < 0) {
                b_result = false;
                break;
            }
            if (!capi_client::get_instance().read(m_n_client_index, m_n_device_index, 0)) {
                b_result = false;
                break;
            }
            _mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_transaction(n_result_index);
            if (!ptr_async_parameter_result->waits(lpu237_of_client::_const_default_mmsec_timeout_of_response)) {
                b_result = false;
                break;
            }
            if (!ptr_async_parameter_result->get_result(n_result_scr)) {
                b_result = false;
                break;
            }
            if (n_result_scr == LPU237_DLL_RESULT_ICC_INSERTED || n_result_scr == LPU237_DLL_RESULT_ICC_REMOVED) {
                b_re_receive = true;
                continue;//re-receiving
            }
            if (!ptr_async_parameter_result->get_result(v_in_packet)) {
                b_result = false;
                break;
            }
            if (v_in_packet.empty()) {
                b_result = false;
                break;
            }

            if (rxbuffer.empty()) {
                if (v_in_packet[0] == 'R'&& v_in_packet[1] ==0xFF && v_in_packet[2] == 0) {
                    //good response of tx
                    b_re_receive = true;
                    continue;
                }
                //the first header
                c_rx = v_in_packet[2];
                memcpy(&dw_rx_transaction_counter, &v_in_packet[3], sizeof(dw_rx_transaction_counter));
                c_chain = v_in_packet[3 + sizeof(unsigned long)];

                memcpy(&dw_rx_total, &v_in_packet[3 + sizeof(unsigned long) + 1], sizeof(unsigned long));
                if (dw_rx_total == 0) {
                    v_rx.resize(0);
                    b_result = false;
                    break;
                }
                rxbuffer.resize(dw_rx_total, 0);
                n_rx = c_rx - (sizeof(unsigned long) + 1 + sizeof(unsigned long));
                memcpy(&rxbuffer[0], &v_in_packet[3 + sizeof(unsigned long) + 1 + sizeof(unsigned long)], n_rx);

                n_remainder = dw_rx_total - n_rx;
                dw_rx_offset = n_rx;
            }
            else {
                c_rx = v_in_packet[2];
                memcpy(&dw_rx_transaction_counter, &v_in_packet[3], sizeof(dw_rx_transaction_counter));

                if ((c_chain + 1) != v_in_packet[3 + sizeof(unsigned long)]) {
                    v_rx.resize(0);
                    b_result = false;
                    break;
                }
                c_chain = v_in_packet[3 + sizeof(unsigned long)];
                n_rx = c_rx - (sizeof(unsigned long) + 1);
                memcpy(&rxbuffer[dw_rx_offset], &v_in_packet[3 + sizeof(unsigned long) + 1], n_rx);
                n_remainder -= n_rx;
                dw_rx_offset += n_rx;
            }
            remove_async_result_for_transaction(n_result_index);
        } while (n_remainder > 0 || b_re_receive);

        if (!b_result)
            continue;

        v_rx.resize(rxbuffer.size());
        std::copy(std::begin(rxbuffer), std::end(rxbuffer), std::begin(v_rx));
    } while (false);

    m_protocol.clear_transaction();
    remove_async_result_for_transaction(n_result_index);

    return b_result;
}

int lpu237_of_client::_cmd_async_waits_rx(_mp::casync_parameter_result::type_callback p_fun, void* p_para, HWND h_wnd, UINT n_msg)
{
    int n_result_index(-1);

    do {
        if (is_null_device())
            continue;

        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        //
        n_result_index = _create_async_result_for_transaction(p_fun, p_para, h_wnd, n_msg);
        if (n_result_index < 0)
            continue;

        if (!capi_client::get_instance().read(m_n_client_index, m_n_device_index, 0)) {
            remove_async_result_for_transaction(n_result_index);
            continue;
        }
    } while (false);
    return n_result_index;
}

bool lpu237_of_client::cmd_get_id()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    bool b_result =  _cmd_get(cprotocol_lpu237::cmd_read_uid);
    if (b_result) {
        _set_device_id(m_protocol.get_uid());
    }
    return b_result;
}

bool lpu237_of_client::_cmd_get( cprotocol_lpu237::type_cmd c_cmd)
{
    bool b_result(false);
    int n_result_index(-1);

    do {
        if (is_null_device()) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : is_null_device().\n", __WFUNCTION__);
            continue;
        }

        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : _reset().\n", __WFUNCTION__);
                continue;
            }
        }
        //
        switch (c_cmd) {
        case cprotocol_lpu237::cmd_read_uid:
            b_result = m_protocol.generate_get_uid();
            break;
        case cprotocol_lpu237::cmd_enter_cs:
            b_result = m_protocol.generate_enter_config_mode();
            break;
        case cprotocol_lpu237::cmd_leave_cs:
            b_result = m_protocol.generate_leave_config_mode();
            break;
        case cprotocol_lpu237::cmd_enter_opos:
            b_result = m_protocol.generate_enter_opos_mode();
            break;
        case cprotocol_lpu237::cmd_leave_opos:
            b_result = m_protocol.generate_leave_opos_mode();
            break;
        default:
            break;
        }//end switch

        if (!b_result) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : generate_x().\n", __WFUNCTION__);
            continue;
        }

        b_result = false;
        //
        _mp::type_v_buffer v_tx(0);
        _mp::type_v_buffer v_rx(0);

        size_t n_remainder_transaction = m_protocol.get_tx_transaction(v_tx);
        if (v_tx.size() == 0) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : v_tx is empty().\n", __WFUNCTION__);
            continue;
        }
        //
        n_result_index = _create_async_result_for_transaction(nullptr, nullptr, NULL, 0);
        if (n_result_index < 0) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : _create_async_result_for_transaction().\n", __WFUNCTION__);
            continue;
        }
        if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_tx)) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : transmit().\n", __WFUNCTION__);
            continue;
        }
        _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : n_result_index = %d.\n", __WFUNCTION__, n_result_index);
        _mp::clog::get_instance().log_data_in_debug_mode(v_tx, L"v_tx = ", L"\n");
        //
        _mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = _mp::casync_result_manager::get_instance(get_class_name()).get_async_parameter_result(m_n_device_index, n_result_index);
        if (!ptr_async_parameter_result->waits(lpu237_of_client::_const_default_mmsec_timeout_of_response)) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : waits().\n", __WFUNCTION__);
            continue;
        }
        if (!ptr_async_parameter_result->get_result(v_rx)) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : get_result().\n", __WFUNCTION__);
            continue;
        }
        if (!m_protocol.set_rx_transaction(v_rx)) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : set_rx_transaction().\n", __WFUNCTION__);
            continue;
        }
        if (!m_protocol.set_from_rx()) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : set_from_rx().\n", __WFUNCTION__);
            continue;
        }
        //        
        b_result = true;
    } while (false);

    m_protocol.clear_transaction();
    _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index,n_result_index);

    return b_result;
}

