#include <websocket/mp_win_nt.h>
#include <mp_casync_result_manager.h>
#include <mp_clog.h>


#include "tg_lpu237_dll.h"
#include "lpu237_of_client.h"

lpu237_of_client::lpu237_of_client() :  i_device_of_client()
{
	m_device_function = cprotocol_lpu237::fun_none;
}

lpu237_of_client::lpu237_of_client(unsigned long n_client_index, const std::wstring& s_device_path)
    : i_device_of_client(n_client_index, s_device_path)
{
    m_device_function = cprotocol_lpu237::fun_none;
}

lpu237_of_client::~lpu237_of_client()
{
    //don't call get_class_name() : virtual function
}

lpu237_of_client::lpu237_of_client(const lpu237_of_client& src) : i_device_of_client(src)
{
}

_mp::type_v_buffer lpu237_of_client::get_name() const
{
    return m_v_name;
}

std::string lpu237_of_client::get_name_by_string() const
{
    std::string s;
    for (auto item : m_v_name) {
        if (item == ' ' || item == 0) {
            break;
        }
        s.push_back((char)item);
    }//end for
    return s;
}

std::wstring lpu237_of_client::get_name_by_wstring() const
{
    std::wstring s;
    for (auto item : m_v_name) {
        if (item == ' ' || item == 0) {
            break;
        }
        s.push_back((wchar_t)item);
    }//end for
    return s;
}

cprotocol_lpu237::type_version lpu237_of_client::get_system_version() const
{
    return m_system_version;
}

cprotocol_lpu237::type_function lpu237_of_client::get_device_function() const
{
    return m_device_function;
}

cprotocol_lpu237::type_system_interface lpu237_of_client::get_interface()
{
    std::lock_guard<std::mutex> lock(m_mutex);
	return m_protocol.get_interface();
}

void lpu237_of_client::set_interface(cprotocol_lpu237::type_system_interface inf)
{
    std::lock_guard<std::mutex> lock(m_mutex);
	m_protocol.set_interface(inf);
}

uint32_t lpu237_of_client::get_buzzer_frequency()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_protocol.get_buzzer_frequency();
}

void lpu237_of_client::set_buzzer_frequency(uint32_t n_frequency)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_protocol.set_buzzer_frequency(n_frequency);
}

cprotocol_lpu237::type_keyboard_language_index lpu237_of_client::get_language()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_protocol.get_language();
}

void lpu237_of_client::set_language(cprotocol_lpu237::type_keyboard_language_index language)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_protocol.set_language(language);
}

bool lpu237_of_client::get_enable_track(int n_track)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (n_track < (int)cprotocol_lpu237::iso1_track) {
        return false;
    }
    if (n_track > (int)cprotocol_lpu237::iso3_track) {
        return false;
    }

	return m_protocol.get_enable_iso(
        (cprotocol_lpu237::type_msr_track_Numer)n_track
    );
}

void lpu237_of_client::set_enable_track(int n_track, bool b_enable)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (n_track < (int)cprotocol_lpu237::iso1_track) {
			continue;
        }
        if (n_track > (int)cprotocol_lpu237::iso3_track) {
            continue;
        }

        m_protocol.set_enable_iso((cprotocol_lpu237::type_msr_track_Numer)n_track, b_enable);
    } while (false);
}

_mp::type_v_buffer lpu237_of_client::get_msr_private_tag(
    int n_track
    ,bool b_prefix
)
{
    _mp::type_v_buffer v;
    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (n_track < (int)cprotocol_lpu237::iso1_track) {
            continue;
        }
        if (n_track > (int)cprotocol_lpu237::iso3_track) {
            continue;
        }

        if(b_prefix)
		    v = m_protocol.get_private_prefix((cprotocol_lpu237::type_msr_track_Numer)n_track, 0);
        else
            v = m_protocol.get_private_postfix((cprotocol_lpu237::type_msr_track_Numer)n_track, 0);
        //
    } while (false);
    return v;
}

void lpu237_of_client::set_msr_private_tag(
    int n_track
    , bool b_prefix
    , const _mp::type_v_buffer& v_tag
)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (n_track < (int)cprotocol_lpu237::iso1_track) {
            continue;
        }
        if (n_track > (int)cprotocol_lpu237::iso3_track) {
            continue;
        }

        if (b_prefix)
			m_protocol.set_private_prefix((cprotocol_lpu237::type_msr_track_Numer)n_track, 0, v_tag);
        else
            m_protocol.set_private_postfix((cprotocol_lpu237::type_msr_track_Numer)n_track, 0, v_tag);

    } while (false);
}

cprotocol_lpu237::type_ibutton_mode lpu237_of_client::get_ibutton_mode()
{
    cprotocol_lpu237::type_ibutton_mode m(cprotocol_lpu237::ibutton_zeros);
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_protocol.get_enable_zeros_ibutton()) {
            m = cprotocol_lpu237::ibutton_zeros;
			continue;
        }
        if (m_protocol.get_enable_f12_ibutton()) {
            m = cprotocol_lpu237::ibutton_f12;
            continue;
        }
        if (m_protocol.get_enable_zeros_7times_ibutton()) {
            m = cprotocol_lpu237::ibutton_zeros7;
            continue;
        }
        if (m_protocol.get_enable_addmit_code_stick_ibutton()) {
            m = cprotocol_lpu237::ibutton_addmit;
            continue;
        }
        m = cprotocol_lpu237::ibutton_none;
	} while (false);
    return m;
}

void lpu237_of_client::set_ibutton_mode(cprotocol_lpu237::type_ibutton_mode mode)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        switch (mode) {
        case cprotocol_lpu237::ibutton_zeros:
			m_protocol.set_enable_zeros_ibutton(true);
            break;
        case cprotocol_lpu237::ibutton_f12:
            m_protocol.set_enable_f12_ibutton(true);
			break;
        case cprotocol_lpu237::ibutton_zeros7:
            m_protocol.set_enable_zeros_7times_ibutton(true);
			break;
        case cprotocol_lpu237::ibutton_addmit:
            m_protocol.set_enable_addmit_Code_stick_ibutton(true);
			break;
        case cprotocol_lpu237::ibutton_none:
            m_protocol.set_enable_zeros_ibutton(false);
            m_protocol.set_enable_f12_ibutton(false);
            m_protocol.set_enable_zeros_7times_ibutton(false);
			m_protocol.set_enable_addmit_Code_stick_ibutton(false);
        default:
            continue;
        }// end switch

    } while (false);
}

_mp::type_v_buffer lpu237_of_client::get_ibutton_tag(bool b_remove, bool b_prefix)
{
    _mp::type_v_buffer v;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (b_remove) {
            if (b_prefix)
                v = m_protocol.get_prefix_ibutton_remove();
            else
                v = m_protocol.get_postfix_ibutton_remove();
        }
        else {
            if (b_prefix)
                v = m_protocol.get_prefix_ibutton();
            else
                v = m_protocol.get_postfix_ibutton();
        }
    } while (false);
	return v;

}

void lpu237_of_client::set_ibutton_tag(bool b_remove, bool b_prefix, const _mp::type_v_buffer& v_tag)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (b_remove) {
            if (b_prefix)
                m_protocol.set_prefix_ibutton_remove(v_tag);
            else
                m_protocol.set_postfix_ibutton_remove(v_tag);
        }
        else {
            if (b_prefix)
                m_protocol.set_prefix_ibutton(v_tag);
            else
                m_protocol.set_postfix_ibutton(v_tag);
        }
    } while (false);

}

_mp::type_v_buffer lpu237_of_client::get_ibutton_remove_indicate_tag()
{
	_mp::type_v_buffer v;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        v = m_protocol.get_ibutton_remove();
    } while (false);
    return v;
}

void lpu237_of_client::set_ibutton_remove_indicate_tag(const _mp::type_v_buffer& v_tag)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_protocol.set_ibutton_remove(v_tag);
    } while (false);
}

std::pair<int, int> lpu237_of_client::get_ibutton_range()
{
    int n_start(-1), n_stop(-1);
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        n_start = m_protocol.get_ibutton_start_code_zero_base_index();
        n_stop = m_protocol.get_ibutton_stop_code_zero_base_index();

        if (n_start == 0 && n_stop == 0) {
            n_stop = 15; // (0,0) -> 은 실제 (0,15) 를 의미한다.
        }
        else if (n_start == 0 && n_stop == 15) {
            n_stop = 0; // (0,15) -> 은 실제 (0,0) 를 의미한다.
        }

    } while (false);
    return std::make_pair(n_start, n_stop);
}

void lpu237_of_client::set_ibutton_range(int n_zero_base_offset_start, int n_zero_base_offset_stop)
{
    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        int n_start(n_zero_base_offset_start),n_stop(n_zero_base_offset_stop);

        if (n_start < 0) {
            n_start = (int)m_protocol.get_ibutton_start_code_zero_base_index();
        }
        if (n_stop < 0) {
            n_stop = (int)m_protocol.get_ibutton_stop_code_zero_base_index();
        }

        if (n_start == 0 && n_stop == 0) {
            n_stop = 15; // (0,0) -> 은 실제 (0,15) 를 의미한다.
        }
        else if (n_start == 0 && n_stop == 15) {
            n_stop = 0; // (0,15) -> 은 실제 (0,0) 를 의미한다.
        }

        if (n_zero_base_offset_start >= 0) {
            m_protocol.set_ibutton_start_code_zero_base_index(n_start);
        }
        if (n_zero_base_offset_stop >= 0) {
            m_protocol.set_ibutton_stop_code_zero_base_index(n_stop);
        }

    } while (false);
}

void lpu237_of_client::set_default()
{
    std::lock_guard<std::mutex> lock(m_mutex);
	m_protocol.set_global_pre_postfix_send_condition(true);
	m_protocol.set_interface(cprotocol_lpu237::system_interface_usb_keyboard);
	m_protocol.set_language(cprotocol_lpu237::language_map_index_english);
	m_protocol.set_buzzer_frequency(cprotocol_lpu237::the_frequency_of_on_buzzer);

    _mp::type_v_buffer v_tag_enter_only{ 0xff,0x0d };
    _mp::type_v_buffer v_tag_empty(0);

    unsigned char s_max_size[] = { 76,37 + 1,37 };
    unsigned char s_bit_size[] = { 7,5,5 };
    unsigned char s_data_mask[] = { 0xfe,0xf8,0xf8 };
    bool b_use_parity[] = { true,true,true };
    unsigned char s_parity_type[] = { 1,1,1 };
    unsigned char s_stxl[] = { 0x8a,0x58,0x58 };
    unsigned char s_etxl[] = { 0x3e,0xf8,0xf8 };
    bool b_use_error_correct[] = { true,true,true };
    unsigned char s_emc_type[] = { 0,0,0 };
    unsigned char s_add_value[] = { 0x20,0x30,0x30 };

    _mp::type_v_buffer v_tag_pre_msr[] = {
        { 0x02,0x22 },
        { 0x00,0x33 },
        { 0x00,0x33 }
    };
    _mp::type_v_buffer v_tag_post_msr[] = {
        { 0x02,0x38,0xff,0x0d },
        { 0x02,0x38,0xff,0x0d },
        { 0x02,0x38,0xff,0x0d }
    };

    for (int i = 0; i < 3; i++) {
        m_protocol.set_enable_iso((cprotocol_lpu237::type_msr_track_Numer)i, true);
		m_protocol.set_private_prefix((cprotocol_lpu237::type_msr_track_Numer)i, 0, v_tag_pre_msr[i]);
		m_protocol.set_private_postfix((cprotocol_lpu237::type_msr_track_Numer)i, 0, v_tag_post_msr[i]);

		m_protocol.set_combination((cprotocol_lpu237::type_msr_track_Numer)i, 1);
		m_protocol.set_msr_max_size((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_max_size[i]);
		m_protocol.set_msr_bit_size((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_bit_size[i]);
		m_protocol.set_msr_data_mask((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_data_mask[i]);
		m_protocol.set_msr_use_parity((cprotocol_lpu237::type_msr_track_Numer)i, 0, b_use_parity[i]);
		m_protocol.set_msr_parity_type((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_parity_type[i]);
		m_protocol.set_msr_stxl((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_stxl[i]);
		m_protocol.set_msr_etxl((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_etxl[i]);
		m_protocol.set_msr_use_error_correct((cprotocol_lpu237::type_msr_track_Numer)i, 0, b_use_error_correct[i]);
		m_protocol.set_msr_ecm_type((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_emc_type[i]);
		m_protocol.set_msr_add_value((cprotocol_lpu237::type_msr_track_Numer)i, 0, s_add_value[i]);
    }//end for

    m_protocol.set_direction(cprotocol_lpu237::dir_bidectional);

    m_protocol.set_global_prefix(v_tag_empty);
	m_protocol.set_global_postfix(v_tag_empty);

	m_protocol.set_prefix_ibutton(v_tag_empty);
	m_protocol.set_postfix_ibutton(v_tag_empty);

	m_protocol.set_ibutton_remove(v_tag_empty);
	m_protocol.set_prefix_ibutton_remove(v_tag_empty);
    m_protocol.set_postfix_ibutton_remove(v_tag_enter_only);

	m_protocol.set_prefix_uart(v_tag_empty);
	m_protocol.set_postfix_uart(v_tag_empty);

    m_protocol.set_order_of_track(cprotocol_lpu237::iso1_track, cprotocol_lpu237::iso2_track, cprotocol_lpu237::iso3_track);

    // blanks arrary setting
    m_protocol.set_enable_zeros_ibutton(true);
    m_protocol.set_indicate_success_if_any_trace_ok(false);
    m_protocol.set_ignore_1track_if_12_is_equal(false);
    m_protocol.set_ignore_3track_if_12_is_equal(false);
    m_protocol.set_ignore_colron(false);
	m_protocol.set_mmd1100_reset_interval(0);
	m_protocol.set_ibutton_start_code_zero_base_index(0);// default : (0,0) 이어야 fw 에서 (0,15) 로 인식함.
    m_protocol.set_ibutton_stop_code_zero_base_index(0);
}

void lpu237_of_client::set_default_with_inf_is_vcom()
{
    set_default();
    set_interface(cprotocol_lpu237::System_interface_usb_vcom);
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

bool lpu237_of_client::cmd_changed_interface_apply()
{
    std::lock_guard<std::mutex> lock(m_mutex);

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

        m_protocol.clear_transaction();

        b_result = m_protocol.generate_set_interface();//enter config, set interface, apply and leave config.
        if (!b_result) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : generate_x().\n", __WFUNCTION__);
            continue;
        }

        size_t n_remainder_transaction(0);
        do {
            b_result = false;
            //
            _mp::type_v_buffer v_tx(0);
            _mp::type_v_buffer v_rx(0);

            n_remainder_transaction = m_protocol.get_tx_transaction(v_tx);
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
                _mp::clog::get_instance().log_data_in_debug_mode(v_rx, L"v_rx = ", L"\n");
                continue;
            }
            if (!m_protocol.set_from_rx()) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : set_from_rx().\n", __WFUNCTION__);
                continue;
            }
            //
            _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index, n_result_index);
            n_result_index = -1;
            b_result = true;
        } while (n_remainder_transaction > 0 && b_result);

    } while (false);

    m_protocol.clear_transaction();
    _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index, n_result_index);

    return b_result;
}

bool lpu237_of_client::cmd_ibutton_enable()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get(cprotocol_lpu237::cmd_start_ibutton);
}

bool lpu237_of_client::cmd_ibutton_disble()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_get(cprotocol_lpu237::cmd_stop_ibutton);
}

int lpu237_of_client::cmd_async_waits_data()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_async_waits_rx(nullptr, nullptr, NULL, 0);
}

int lpu237_of_client::cmd_async_waits_data(_mp::casync_parameter_result::type_callback p_fun, void* p_para)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_async_waits_rx(p_fun, p_para, NULL, 0);
}

int lpu237_of_client::cmd_async_waits_data(HWND h_wnd, UINT n_msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return _cmd_async_waits_rx(nullptr, nullptr, h_wnd, n_msg);
}

size_t lpu237_of_client::get_remainder_phase_number()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_protocol.size_tx_transaction();
}

bool lpu237_of_client::process_async_result(const _mp::type_v_buffer& v_result)
{
    bool b_result(false);

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_protocol.set_rx_transaction(v_result)) {
            continue;
        }
        if (!m_protocol.set_from_rx()) {
            continue;
        }

		b_result = true;
    }while(false);

	return b_result;
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

int lpu237_of_client::_cmd_async_waits_rx(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
    , HWND h_wnd
    , UINT n_msg
)
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

void lpu237_of_client::_set_name(const _mp::type_v_buffer& v_name)
{
    m_v_name = v_name;
}

void lpu237_of_client::_set_system_version(const cprotocol_lpu237::type_version& version)
{
	m_system_version = version;
}

void lpu237_of_client::_set_device_function(cprotocol_lpu237::type_function device_function)
{
    m_device_function = device_function;
}

bool lpu237_of_client::cmd_get_system_information_with_name()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    bool b_result(false);

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
        if (!m_protocol.generate_get_system_information_with_name()) {
            _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : generate_x().\n", __WFUNCTION__);
            continue;
        }

        _mp::type_v_buffer v_tx(0);
        _mp::type_v_buffer v_rx(0);
        size_t n_remainder_transaction(0);
        int n_result_index(-1);

        do {
			v_tx.resize(0); v_rx.resize(0);
            n_remainder_transaction = m_protocol.get_tx_transaction(v_tx);
            if (n_remainder_transaction == 0) {
                b_result = true;
				continue;//complete all transaction.
            }
            if (v_tx.size() == 0) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : v_tx is empty().\n", __WFUNCTION__);
                break;
            }
            n_result_index = _create_async_result_for_transaction(nullptr, nullptr, NULL, 0);
            if (n_result_index < 0) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : _create_async_result_for_transaction().\n", __WFUNCTION__);
                break;
            }
            if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_tx)) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : transmit().\n", __WFUNCTION__);
                break;
            }
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
                _mp::clog::get_instance().log_data_in_debug_mode(v_rx, L"v_rx = ", L"\n");
                continue;
            }
            if (!m_protocol.set_from_rx()) {
                _mp::clog::get_instance().log_fmt_in_debug_mode(L" : DEB : %ls : error : set_from_rx().\n", __WFUNCTION__);
                continue;
            }

        } while (n_remainder_transaction > 0);

        if (b_result) {
            // get version, get device type, get version structure, get name 
            _set_name(m_protocol.get_name());
            _set_system_version(m_protocol.get_system_version());

            cprotocol_lpu237::type_function dev_type = m_protocol.get_device_function();
            _set_device_function(dev_type);


        }

        m_protocol.clear_transaction();
        _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index, n_result_index);

    }while(false);


    return b_result;
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

std::tuple<bool, int, size_t,bool> lpu237_of_client::cmd_start_async_next_phase(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
    , int n_result_index
)
{
    bool b_result(false);
    unsigned long n_device_index(const_invalied_device_index);
    bool b_remove_async_result_for_transaction(true);
    size_t n_remainder_transaction(0);
	bool b_complete_transaction(true);

    if (n_result_index == _mp::casync_result_manager::const_invalied_result_index) {
        return std::make_tuple(false, n_result_index, 0, true);
    }

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
            continue;
        if (m_n_device_index == const_invalied_device_index)
            continue;
        if (is_null_device())
            continue;
        //
        _mp::type_v_buffer v_out_packet(0);
        n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);

        if (n_remainder_transaction >0) {
			// 현재 시작하려는 phase 가 transaction 의 마지막 phase 가 아닐 때, transaction 이 아직 완료되지 않았다고 판단한다.
            b_complete_transaction = false;
        }

		// n_remainder_transaction == 0 일 때,
        // 현재 시작하려는 phase 가 transaction 의 마지막 phase 또는 
        // transaction 의 모든 phase 가 완료된 경우.
        // 
        if (!v_out_packet.empty()) {
            //현재 시작하려는 phase 가 transaction 의 마지막 phase
            b_complete_transaction = false;

            if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_out_packet)) {
                continue; //transmit failed.
            }

            b_remove_async_result_for_transaction = false;
        }
        else {
            //transaction 의 모든 phase 가 완료된 경우.

        }
        //
        b_result = true;
    } while (false);

    if (b_remove_async_result_for_transaction) {
        remove_async_result_for_transaction(n_result_index);
        n_result_index = _mp::casync_result_manager::const_invalied_result_index;
    }
    return std::make_tuple(b_result, n_result_index, n_remainder_transaction, b_complete_transaction);
}

std::tuple<bool, int, size_t> lpu237_of_client::cmd_start_async_get_parameters(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
)
{
    bool b_result(false);
    unsigned long n_device_index(const_invalied_device_index);
    int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
    bool b_remove_async_result_for_transaction(false);
	size_t n_total_phase_in_this_transaction(0);
    std::wstring s_option_reuse;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
            continue;
        if (m_n_device_index == const_invalied_device_index)
            continue;
        if (is_null_device())
            continue;
        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        m_protocol.clear_transaction();

        if (!m_protocol.generate_get_parameters())
            continue;

        n_total_phase_in_this_transaction = m_protocol.size_tx_transaction();

        _mp::type_v_buffer v_out_packet(0);
        size_t n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);
        if (n_remainder_transaction > 0) {
            s_option_reuse = L"reuse";
        }
        if (n_remainder_transaction == 0) {
			continue;//error.
        }
        if (v_out_packet.size() == 0) {
			continue;//error.
        }
        //
        n_result_index = _create_async_result_for_transaction(p_fun, p_para, 0, 0, L"reuse");
        if (n_result_index < 0)
            continue;
        if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_out_packet, s_option_reuse)) {
            b_remove_async_result_for_transaction = true;
			continue; //transmit failed.
        }
        b_result = true;
    } while (false);

    if (b_remove_async_result_for_transaction) {
        remove_async_result_for_transaction(n_result_index);
        n_result_index = _mp::casync_result_manager::const_invalied_result_index;
    }
    return std::make_tuple(b_result, n_result_index, n_total_phase_in_this_transaction);
}

std::tuple<bool, int, size_t> lpu237_of_client::cmd_start_async_set_parameters(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
)
{
    bool b_result(false);
    unsigned long n_device_index(const_invalied_device_index);
    int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
    bool b_remove_async_result_for_transaction(false);
    size_t n_total_phase_in_this_transaction(0);
    std::wstring s_option_reuse;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
            continue;
        if (m_n_device_index == const_invalied_device_index)
            continue;
        if (is_null_device())
            continue;
        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        m_protocol.clear_transaction();

        if (!m_protocol.generate_set_parameters())
            continue;
        
        n_total_phase_in_this_transaction = m_protocol.size_tx_transaction();

        _mp::type_v_buffer v_out_packet(0);
        size_t n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);
        if (n_remainder_transaction > 0) {
            s_option_reuse = L"reuse";
        }
        if (n_remainder_transaction == 0) {
            continue;//error.
        }
        if (v_out_packet.size() == 0) {
            continue;//error.
        }
        //
        n_result_index = _create_async_result_for_transaction(p_fun, p_para, 0, 0, L"reuse");
        if (n_result_index < 0)
            continue;
        if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_out_packet, s_option_reuse)) {
            b_remove_async_result_for_transaction = true;
            continue; //transmit failed.
        }
        b_result = true;
    } while (false);

    if (b_remove_async_result_for_transaction) {
        remove_async_result_for_transaction(n_result_index);
        n_result_index = _mp::casync_result_manager::const_invalied_result_index;
    }
    return std::make_tuple(b_result, n_result_index, n_total_phase_in_this_transaction);
}

std::tuple<bool, int, size_t> lpu237_of_client::cmd_start_async_get_parameters_except_combination(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
)
{
    bool b_result(false);
    unsigned long n_device_index(const_invalied_device_index);
    int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
    bool b_remove_async_result_for_transaction(false);
    size_t n_total_phase_in_this_transaction(0);
    std::wstring s_option_reuse;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
            continue;
        if (m_n_device_index == const_invalied_device_index)
            continue;
        if (is_null_device())
            continue;
        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        m_protocol.clear_transaction();

        if (!m_protocol.generate_get_parameters())
            continue;
        
        n_total_phase_in_this_transaction = m_protocol.size_tx_transaction();

        _mp::type_v_buffer v_out_packet(0);
        size_t n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);
        if (n_remainder_transaction > 0) {
            s_option_reuse = L"reuse";
        }
        if (n_remainder_transaction == 0) {
            continue;//error.
        }
        if (v_out_packet.size() == 0) {
            continue;//error.
        }
        //
        n_result_index = _create_async_result_for_transaction(p_fun, p_para, 0, 0, L"reuse");
        if (n_result_index < 0)
            continue;
        if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_out_packet, s_option_reuse)) {
            b_remove_async_result_for_transaction = true;
            continue; //transmit failed.
        }
        b_result = true;
    } while (false);

    if (b_remove_async_result_for_transaction) {
        remove_async_result_for_transaction(n_result_index);
        n_result_index = _mp::casync_result_manager::const_invalied_result_index;
    }
    return std::make_tuple(b_result, n_result_index, n_total_phase_in_this_transaction);
}

std::tuple<bool, int, size_t> lpu237_of_client::cmd_start_async_set_parameters_except_combination(
    _mp::casync_parameter_result::type_callback p_fun
    , void* p_para
)
{
    bool b_result(false);
    unsigned long n_device_index(const_invalied_device_index);
    int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
    bool b_remove_async_result_for_transaction(false);
    size_t n_total_phase_in_this_transaction(0);
    std::wstring s_option_reuse;

    do {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
            continue;
        if (m_n_device_index == const_invalied_device_index)
            continue;
        if (is_null_device())
            continue;
        if (!_mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index)) {
            //cancel operation.
            if (!_reset()) {
                continue;
            }
        }
        m_protocol.clear_transaction();

        if (!m_protocol.generate_set_parameters())
            continue;

        n_total_phase_in_this_transaction = m_protocol.size_tx_transaction();

        _mp::type_v_buffer v_out_packet(0);
        size_t n_remainder_transaction = m_protocol.get_tx_transaction(v_out_packet);
        if (n_remainder_transaction > 0) {
            s_option_reuse = L"reuse";
        }
        if (n_remainder_transaction == 0) {
            continue;//error.
        }
        if (v_out_packet.size() == 0) {
            continue;//error.
        }
        //
        n_result_index = _create_async_result_for_transaction(p_fun, p_para, 0, 0, L"reuse");
        if (n_result_index < 0)
            continue;
        if (!capi_client::get_instance().transmit(m_n_client_index, m_n_device_index, 0, 0, v_out_packet, s_option_reuse)) {
            b_remove_async_result_for_transaction = true;
            continue; //transmit failed.
        }
        b_result = true;
    } while (false);

    if (b_remove_async_result_for_transaction) {
        remove_async_result_for_transaction(n_result_index);
        n_result_index = _mp::casync_result_manager::const_invalied_result_index;
    }
    return std::make_tuple(b_result, n_result_index, n_total_phase_in_this_transaction);
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
		case cprotocol_lpu237::cmd_start_ibutton:
            b_result = m_protocol.generate_start_ibutton();
			break;
        case cprotocol_lpu237::cmd_stop_ibutton:
			b_result = m_protocol.generate_stop_ibutton();
			break;
		case cprotocol_lpu237::cmd_apply:
            b_result = m_protocol.generate_apply_config_mode();
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
            _mp::clog::get_instance().log_data_in_debug_mode(v_rx, L"v_rx = ", L"\n");
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

