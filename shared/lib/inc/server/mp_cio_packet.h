#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include <mp_cstring.h>
#include <mp_coperation.h>
#include <mp_cconvert.h>
#include <mp_vcpacket.h>

namespace _mp {
class cio_packet : public _mp::vcpacket
{
public:
	enum {
		const_min_number_of_field = 6	//the number of mandotory fields.
	};
	enum type_cmd : char{
		cmd_invalid = '0', //invalid format
		cmd_request = 'T',		//from client to server
		cmd_response = 'R',		//server to client
		cmd_system_request = 'S',// generatedby system, system event will be sent to client.
		cmd_self_request = 'I'	//internal request in server....... no need  response to client
	};

	enum type_act : char {
		//Don't use 'C' charactor. this charactor is used in JS library only.
		act_mgmt_unknown = 'U',
		act_mgmt_get_echo = 'E',
		act_mgmt_get_device_list = 'L',
		act_mgmt_ctl_show = 'S',
		act_mgmt_file_operation = 'F',
		act_mgmt_dev_plug_in = 'P',
		act_mgmt_advance_operation = 'A',//from v1.8
		act_mgmt_dev_kernel_operation = 'K',//from v1.8
		act_dev_independent_bootloader = 'b',//from v1.8
		act_dev_open = 'o',
		act_dev_close = 'c',
		act_dev_write = 's',
		act_dev_read = 'r',
		act_dev_transmit = 't', //write and read
		act_dev_cancel = 'x',	//cancel operation
		act_dev_sub_bootloader = '0' //bootloader.(sub system )
	};

	enum type_owner : char {
		owner_manager = 'M',	//packet is for manager.
		owner_device = 'D'		//packet is for a device.
	};

	enum type_data_field : char{
		data_field_binary = 'H',
		data_field_string_utf8 = 'S'
	};

#pragma pack(push, 1)
	typedef	struct {
		type_cmd c_cmd;
		unsigned long n_session;//session number
		type_owner c_owner;
		unsigned long dw_device_index;
		type_act c_act;//action code
		unsigned char c_in_id;//in report id or in pipe id
		unsigned char c_out_id;//out report id or out pipe id
		unsigned long n_data;//the size of s_data field
		unsigned char s_data[1];
	} type_packect;
#pragma pack(pop)

	typedef	std::shared_ptr< cio_packet >	type_ptr;

	typedef	std::tuple<unsigned long, unsigned long>		type_tuple_session_number_device_index;

private:
	enum _type_json_name{
		json_name_request_type,
		json_name_session_numer,
		json_name_packet_owner,
		json_name_device_index,
		json_name_action_code,
		json_name_in_id,
		json_name_out_id,
		json_name_data_field_type,
		json_name_data_field
	};

public:
	enum type_error_reason : int {
		error_reason_none = -1, //none reson string
		error_reason_json = 0,
		error_reason_session = 1,
		error_reason_request_type = 2,
		error_reason_device_path = 3,
		error_reason_device_object = 4,
		error_reason_action_code = 5,
		error_reason_device_open = 6,
		error_reason_device_not_open = 7,
		error_reason_device_close = 8,
		error_reason_device_operation = 9,
		error_reason_device_misformat = 10,
		error_reason_device_subsystem = 11,
		error_reason_device_busy_for_update = 12,
		error_reason_file_exsit_file = 13,
		error_reason_file_none_file = 14,
		error_reason_file_not_selected = 15,
		error_reason_file_operation = 16,
		error_reason_bootload_mismatch_condition = 17,
		error_reason_overflow_buffer = 18,	//from v1.8.1, tx rx overflow.
		error_reason_last_error_code = 19	//this constant must be defined in the last position.
											// only this is used for indicating the last of error code.
	};

	static std::wstring get_error_message(type_error_reason reason)
	{
		static std::wstring ss_message_map[] =
		{
			L"json",
			L"session",
			L"request_type",
			L"device_path",
			L"device_object",
			L"action_code",
			L"device_open",
			L"device_not_open",
			L"device_close",
			L"device_operation",
			L"device_misformat",
			L"device_subsystem",
			L"device_busy_for_update",
			L"file_exsit",
			L"file_none",
			L"file_not_selected",
			L"file_operation",
			L"bootload_mismatch_condition",
			L"overflow_buffer"
		};

		std::wstring s_message;

		if (reason < error_reason_last_error_code && reason >= error_reason_json) {
			s_message = ss_message_map[reason];
		}

		return s_message;
	}

	static cio_packet::type_ptr build_common_error_response(const cio_packet::type_ptr & ptr_request, const std::wstring& s_error_resason)
	{
		type_cmd c_cmd(cmd_response);
		unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);
		type_owner c_owner(owner_manager);
		unsigned long dw_device_index(0);
		cio_packet::type_act c_act(act_mgmt_unknown);
		unsigned char c_in_id(0), c_out_id(0);

		do {
			if (!ptr_request)
				continue;
			
			n_session = ptr_request->get_session_number();
			c_owner = ptr_request->get_owner();
			dw_device_index = ptr_request->get_device_index();
			c_act = ptr_request->get_action();
			c_in_id = ptr_request->get_in_id();
			c_out_id = ptr_request->get_out_id();

		} while (false);

		cio_packet::type_ptr ptr_response = cio_packet::build(
			c_cmd, n_session, c_owner, dw_device_index, c_act, c_in_id, c_out_id, cio_packet::data_field_string_utf8, 0, nullptr
		);

		//
		ptr_response->set_data_error(s_error_resason);
		return ptr_response;
	}
	
	static bool adjust_device_filter(type_set_wstring& in_out_set_wstring_data_field)
	{
		/**
		* this function device filter string adjust to platform.
		* 
		*/
		bool b_result(false);

		do {
			// here windows system only. lower
			type_set_wstring out_set;
			std::wstring s_out_filter;

			for (std::wstring s_in_filter : in_out_set_wstring_data_field) {
				if (!s_in_filter.empty()) {
					s_out_filter.resize(s_in_filter.size());
					std::transform(std::begin(s_in_filter), std::end(s_in_filter), std::begin(s_out_filter), towlower);
					out_set.insert(s_out_filter);
				}

			}//end for

			in_out_set_wstring_data_field.swap(out_set);
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	static cio_packet::type_ptr build_close(unsigned long n_session, unsigned long dw_device_index)
	{
		return cio_packet::build(
			cio_packet::cmd_self_request
			, n_session
			, cio_packet::owner_device
			, dw_device_index
			, cio_packet::act_dev_close
			, 0
			, 0
			, cio_packet::data_field_binary
			, 0
			, nullptr
		);
	}

	static cio_packet::type_ptr build_cancel(unsigned long n_session, unsigned long dw_device_index)
	{
		return cio_packet::build(
			cio_packet::cmd_self_request
			, n_session
			, cio_packet::owner_device
			, dw_device_index
			, cio_packet::act_dev_cancel
			, 0
			, 0
			, cio_packet::data_field_binary
			, 0
			, nullptr
		);
	}

	static cio_packet::type_ptr build(
		type_cmd c_cmd, unsigned long n_session
		, type_owner c_owner, unsigned long dw_device_index, type_act c_act
		, unsigned char c_in_id, unsigned char c_out_id
		, cio_packet::type_data_field data_field_type,unsigned long n_data, const unsigned char* ps_data)
	{
		 return std::make_shared<cio_packet>(data_field_type,cio_packet::_build(c_cmd, n_session,c_owner, dw_device_index, c_act, c_in_id,c_out_id, n_data, ps_data));
	}

	static cio_packet::type_ptr build_from_json(const type_v_buffer& v_json_string_format, std::string & s_json_error)
	{
		std::string s_asc_rx;
		s_json_error.clear();

		for (char c : v_json_string_format) {
			s_asc_rx.push_back(c);
		}//end for

		try {
			nlohmann::json json_obj = nlohmann::json::parse(s_asc_rx);
			return cio_packet::_build(json_obj);
		}
		catch (nlohmann::json::parse_error& e) {
			s_json_error = "msg : " + std::string(e.what());
			s_json_error += (" :: id : " + std::to_string(e.id));
			s_json_error += (" :: byte position of error: " + std::to_string(e.byte));

			return cio_packet::type_ptr();
		}
	}
public:
	virtual ~cio_packet()
	{
	}

	virtual bool is_valid()
	{
		return cio_packet::_is_valid(m_v_packet);
	}

	cio_packet() : _mp::vcpacket(),m_type_data_field(cio_packet::data_field_binary)
	{
		m_v_packet = cio_packet::_build(cio_packet::cmd_request, _MP_TOOLS_INVALID_SESSION_NUMBER, cio_packet::owner_manager, 0, act_mgmt_get_echo, 0, 0,  0, NULL);
	}

	cio_packet(const cio_packet & src)
	{
		*this = src;
	}

	cio_packet(cio_packet::type_data_field data_field_type,const type_v_buffer & v_packet) 
		: _mp::vcpacket(), m_type_data_field(data_field_type)
	{
		if (cio_packet::_is_valid(v_packet)) {
			m_v_packet = v_packet;
		}
		else{//error
			m_v_packet = cio_packet::_build(cio_packet::cmd_request, _MP_TOOLS_INVALID_SESSION_NUMBER,cio_packet::owner_manager, 0, act_mgmt_get_echo, 0, 0,0, NULL);
		}
	}

	cio_packet(
		type_cmd c_cmd, unsigned long n_session
		,type_owner c_owner,unsigned long dw_device_index, type_act c_act
		, unsigned char c_in_id, unsigned char c_out_id
		, cio_packet::type_data_field data_field_type,unsigned long n_data,const unsigned char *ps_data)
		: m_type_data_field(data_field_type)
	{
		m_v_packet = cio_packet::_build(c_cmd, n_session, c_owner, dw_device_index, c_act, c_in_id,c_out_id, n_data, ps_data);
	}

	cio_packet(
		type_cmd c_cmd, unsigned long n_session
		, type_owner c_owner, unsigned long dw_device_index, type_act c_act
		, unsigned char c_in_id, unsigned char c_out_id
		, cio_packet::type_data_field data_field_type, const type_v_buffer & v_data)
		: m_type_data_field(data_field_type)
	{
		if (v_data.empty()) {
			m_v_packet = cio_packet::_build(c_cmd, n_session, c_owner, dw_device_index, c_act, c_in_id,c_out_id, (unsigned long)v_data.size(), NULL);
		}
		else {
			m_v_packet = cio_packet::_build(c_cmd, n_session, c_owner, dw_device_index, c_act, c_in_id, c_out_id, (unsigned long)v_data.size(), &v_data[0]);
		}
	}

	cio_packet & operator=(const cio_packet & src)
	{
		m_v_packet = src.m_v_packet;
		m_type_data_field = src.m_type_data_field;
		return *this;
	}

	//getter
	bool is_kernel_device_open() const
	{
		bool b_kernel_device_open(false);
		do {
			type_list_wstring list_wstring;
			if (get_data_field(list_wstring) < 2)
				continue;

			type_list_wstring::iterator it(std::begin(list_wstring));
			if (it->compare(L"success") != 0)
				continue;
			++it;

			if (it->compare(L"this_packet_is_kernel_device_open") != 0)
				continue;
			//
			b_kernel_device_open = true;
		} while (false);
		return b_kernel_device_open;
	}
	bool is_kernel_device_close()
	{
		bool b_kernel_device_close(false);
		do {
			type_list_wstring list_wstring;
			if (get_data_field(list_wstring) < 2)
				continue;

			type_list_wstring::iterator it(std::begin(list_wstring));
			if (it->compare(L"success") != 0)
				continue;
			++it;

			if (it->compare(L"this_packet_is_kernel_device_close") != 0)
				continue;
			//
			b_kernel_device_close = true;
		} while (false);
		return b_kernel_device_close;
	}

	cio_packet::type_data_field get_data_field_type() const
	{
		return m_type_data_field;
	}
	bool is_request()
	{
		type_packect *p_packet = (type_packect *)&m_v_packet[0];

		if (p_packet->c_cmd == cio_packet::cmd_request )
			return true;
		else
			return false;
	}
	bool is_response()
	{
		type_packect* p_packet = (type_packect*)&m_v_packet[0];

		if (p_packet->c_cmd == cio_packet::cmd_response)
			return true;
		else
			return false;
	}
	bool is_self()
	{
		type_packect* p_packet = (type_packect*)& m_v_packet[0];

		if (p_packet->c_cmd == cio_packet::cmd_self_request)
			return true;
		else
			return false;
	}
	bool is_system()
	{
		type_packect* p_packet = (type_packect*)&m_v_packet[0];

		if (p_packet->c_cmd == cio_packet::cmd_system_request)
			return true;
		else
			return false;
	}

	bool is_owner_manager()
	{
		type_packect *p_packet = (type_packect *)&m_v_packet[0];

		if (p_packet->c_owner == cio_packet::owner_manager)
			return true;
		else
			return false;
	}

	type_owner get_owner() const
	{
		const type_packect* p_packet = (type_packect*)&m_v_packet[0];
		return p_packet->c_owner;
	}

	unsigned long get_session_number() const
	{
		const type_packect* p_packet = (type_packect*)& m_v_packet[0];
		return p_packet->n_session;
	}
	unsigned long get_device_index() const
	{
		const type_packect *p_packet = (type_packect *)&m_v_packet[0];
		return p_packet->dw_device_index;
	}
	type_act get_action() const
	{
		const type_packect* p_packet = (type_packect*)& m_v_packet[0];
		return p_packet->c_act;
	}
	unsigned char get_in_id() const
	{
		const type_packect* p_packet = (type_packect*)&m_v_packet[0];
		return p_packet->c_in_id;
	}
	unsigned char get_out_id() const
	{
		const type_packect* p_packet = (type_packect*)&m_v_packet[0];
		return p_packet->c_out_id;
	}

	bool empty_data_field() const
	{
		const type_packect* p_packet = (type_packect*)&m_v_packet[0];
		if (p_packet->n_data == 0)
			return true;
		else
			return false;
	}
	size_t get_data_field_raw_size() const
	{
		const type_packect* p_packet = (type_packect*)&m_v_packet[0];
		return (size_t)p_packet->n_data;
	}
	size_t get_data_field_size() const
	{
		size_t n_size(0);
		n_size = get_data_field_raw_size();
		return n_size;
		/*
		do {

			if (get_data_field_type() != cio_packet::data_field_string_utf8) {
				//binary
				n_size = get_data_field_raw_size();
				continue;
			}

			type_set_wstring set_wstring;
			size_t n_total_characters(0);
			if (get_data_field(set_wstring, n_total_characters) == 0)
				continue;
			n_size = n_total_characters;
		} while (false);

		return n_size;
		*/
	}

	size_t get_data_field(type_v_buffer & v_data) const
	{
		const type_packect *p_packet = (type_packect *)&m_v_packet[0];
		v_data.resize(p_packet->n_data);
		if (p_packet->n_data > 0)
			std::copy(p_packet->s_data, &p_packet->s_data[p_packet->n_data], std::begin(v_data));
		//
		return v_data.size();
	}
	size_t get_data_field(std::wstring & sw_data) const
	{
		sw_data.clear();
		type_set_wstring set_wstring;
		if (get_data_field(set_wstring) > 0) {
			sw_data = *(set_wstring.begin());
		}
		return sw_data.size();
	}
	size_t get_data_field(std::string& s_data) const
	{
		s_data.clear();
		std::wstring sw_data;
		if (get_data_field(sw_data) > 0) {
			s_data = cstring::get_mcsc_from_unicode(sw_data);
		}
		return s_data.size();
	}

	/**
	 * geet data field by wstring set.
	 * 
	 * \param set_wstring
	 * \param n_total_characters : set by the numnber of all character on set_wstring.
	 * ex) set_wstring = {"01","2","467"}, then n_total_characters is 6.
	 * \return the number of string
	 */
	size_t get_data_field(type_set_wstring& set_wstring,size_t & n_total_characters) const
	{
		n_total_characters = 0;
		set_wstring.clear();

		do {
			if (get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;
			//
			const type_packect* p_packet = (type_packect*)&m_v_packet[0];
			if (p_packet->n_data == 0)
				continue;
			//
			size_t n_total(0);
			cconvert::change_from_utf8_multistring(set_wstring, n_total_characters, (const char*)p_packet->s_data, p_packet->n_data);

		} while (false);

		return set_wstring.size();
	}

	size_t get_data_field(type_set_wstring& set_wstring) const
	{
		set_wstring.clear();
		do {
			if (get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;
			//
			const type_packect* p_packet = (type_packect*)&m_v_packet[0];
			if (p_packet->n_data == 0)
				continue;
			//
			size_t n_total(0);
			cconvert::change_from_utf8_multistring(set_wstring, n_total, (const char*)p_packet->s_data, p_packet->n_data);
		} while (false);

		return set_wstring.size();
	}
	size_t get_data_field(type_list_wstring& list_wstring) const
	{
		list_wstring.clear();
		do {
			if (get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;
			//
			const type_packect* p_packet = (type_packect*)&m_v_packet[0];
			if (p_packet->n_data == 0)
				continue;
			//
			size_t n_total(0);
			cconvert::change_from_utf8_multistring(list_wstring, n_total, (const char*)p_packet->s_data, p_packet->n_data);
		} while (false);

		return list_wstring.size();
	}
	size_t get_data_field(type_deque_wstring & deque_wstring) const
	{
		deque_wstring.clear();
		std::wstring s_data;
		do {
			if (get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;
			//
			const type_packect* p_packet = (type_packect*)&m_v_packet[0];
			if (p_packet->n_data == 0)
				continue;
			//
			size_t n_total(0);
			cconvert::change_from_utf8_multistring(deque_wstring, n_total,(const char*)p_packet->s_data, p_packet->n_data);
		} while (false);
		return deque_wstring.size();
	}
	size_t get_packet_by_json_format(type_v_buffer& v_data)
	{
		cconvert::get_data(v_data, _to_json(), false);
		return v_data.size();
	}
	//setter
	void set_kernel_device_open(bool b_this_packet_is_kernel_device_open)
	{
		set_data_by_utf8(L"this_packet_is_kernel_device_open", true);
	}
	void set_kernel_device_close(bool b_this_packet_is_kernel_device_close)
	{
		set_data_by_utf8(L"this_packet_is_kernel_device_close", true);
	}

	cio_packet& set_cmd(cio_packet::type_cmd c_cmd)
	{
		type_packect *p_packet = (type_packect *)&m_v_packet[0];
		p_packet->c_cmd = c_cmd;
		return *this;
	}
	cio_packet& set_session_number(unsigned long n_session)
	{
		type_packect* p_packet = (type_packect*)& m_v_packet[0];
		p_packet->n_session = n_session;
		return *this;
	}
	cio_packet& set_owner(cio_packet::type_owner c_owner, unsigned long dw_device_index = 0)
	{
		type_packect *p_packet = (type_packect *)&m_v_packet[0];
		p_packet->c_owner = c_owner;
		p_packet->dw_device_index = dw_device_index;
		return *this;
	}
	cio_packet& set_device_index(unsigned long dw_device_index)
	{
		type_packect* p_packet = (type_packect*)&m_v_packet[0];
		p_packet->dw_device_index = dw_device_index;
		return *this;
	}
	cio_packet& set_action(cio_packet::type_act c_act)
	{
		type_packect* p_packet = (type_packect*)& m_v_packet[0];
		p_packet->c_act = c_act;
		return *this;
	}
	cio_packet& set_in_id(unsigned char c_id)
	{
		type_packect* p_packet = (type_packect*)&m_v_packet[0];
		p_packet->c_in_id = c_id;
		return *this;
	}
	cio_packet& set_out_id(unsigned char c_id)
	{
		type_packect* p_packet = (type_packect*)&m_v_packet[0];
		p_packet->c_out_id = c_id;
		return *this;
	}

	cio_packet& set_data_by_utf8(const type_set_wstring& set_wstring)
	{
		do {
			size_t i = 0;
			if (set_wstring.empty()) {
				set_data_by_utf8(std::wstring());//reset data field
				continue;
			}

			for (auto str : set_wstring) {
				if (i == 0) {
					set_data_by_utf8(str, false);
				}
				else {
					set_data_by_utf8(str, true);
				}
				//
				++i;
			}//end for
		} while (false);

		return *this;
	}
	cio_packet& set_data_by_utf8(const type_list_wstring& list_wstring)
	{
		do {
			size_t i = 0;
			if (list_wstring.empty()) {
				set_data_by_utf8(std::wstring());//reset data field
				continue;
			}

			for (auto str : list_wstring) {
				if (i == 0) {
					set_data_by_utf8(str, false);
				}
				else {
					set_data_by_utf8(str, true);
				}
				//
				++i;
			}//end for
		} while (false);

		return *this;
	}

	/**
	* NOTICE : inner character ':' will be changed to "::".
	*/
	cio_packet& set_data_by_utf8(const std::wstring& s_data, bool b_append = false)
	{
		do {
			if (m_type_data_field == cio_packet::data_field_binary && b_append)
				continue;//impossible operation

			m_type_data_field = cio_packet::data_field_string_utf8;

			if (s_data.empty()) {
				_set_data(type_v_buffer(0), b_append);
				continue;
			}

			//replace :  -> ::  ~~~  warning
			size_t n_pos(0);

			std::shared_ptr<std::wstring> ptr_s_data_bk(std::make_shared<std::wstring>(s_data));
			do {
				n_pos = ptr_s_data_bk->find(L':', n_pos);
				if (n_pos == std::wstring::npos)
					continue;
				//
				ptr_s_data_bk->replace(n_pos, 1, 2, L':');
				if ((n_pos + 2) < ptr_s_data_bk->size())
					n_pos += 2;
				else
					break;//exit while
			} while (n_pos != std::wstring::npos);

			std::string s;
			if (ptr_s_data_bk) {
				s = cstring::get_mcsc_from_unicode(*ptr_s_data_bk);
			}

			type_v_buffer v_data_utf8(s.size(), 0);
			std::transform(s.begin(), s.end(), v_data_utf8.begin(), [](char c)->unsigned char {
				return (unsigned char)c;
				});

			_set_data(v_data_utf8, b_append);
			
		} while (false);
		return *this;
	}

	cio_packet& set_data_by_utf8_without_colron_doubling(const std::wstring& s_data, bool b_append = false)
	{
		do {
			if (m_type_data_field == cio_packet::data_field_binary && b_append)
				continue;//impossible operation

			m_type_data_field = cio_packet::data_field_string_utf8;

			if (s_data.empty()) {
				_set_data(type_v_buffer(0), b_append);
				continue;
			}

			std::string s;
			s = cstring::get_mcsc_from_unicode(s_data);

			type_v_buffer v_data_utf8(s.size(), 0);
			std::transform(s.begin(), s.end(), v_data_utf8.begin(), [](char c)->unsigned char {
				return (unsigned char)c;
				});

			_set_data(v_data_utf8, b_append);

		} while (false);
		return *this;
	}

	cio_packet& set_data(const type_v_buffer& v_data, bool b_append = false)
	{
		do {
			if (m_type_data_field == cio_packet::data_field_string_utf8) {
				if (b_append && !empty_data_field())
					continue;//impossible operation.
			}
			m_type_data_field = cio_packet::data_field_binary;
			_set_data(v_data, b_append);
		} while (false);
		return *this;
	}
	cio_packet& set_data(const unsigned char *s_data, unsigned long n_data, bool b_append = false)
	{
		do {
			type_v_buffer v_data(0);

			if (s_data != nullptr && n_data > 0) {
				v_data.assign(&s_data[0], &s_data[n_data]);
			}
			set_data(v_data, b_append);
		} while (false);
		return *this;
	}

	cio_packet& set_data_sucesss()
	{
		set_data_by_utf8(L"success");
		return *this;
	}
	cio_packet& set_data_error(const std::wstring & s_resason=std::wstring() )
	{
		set_data_by_utf8(L"error");
		if (!s_resason.empty()) {
			set_data_by_utf8(s_resason, true);
		}
		return *this;
	}
	cio_packet& set_data_error_for_update(const std::wstring& s_resason = std::wstring())
	{
		set_data_by_utf8(L"error");
		set_data_by_utf8(L"0", true);//current step
		set_data_by_utf8(L"0", true);//totoal step
		set_data_by_utf8(s_resason, true);
		return *this;
	}

	/**
	 * set_data_from_send_data_to_session_request.
	 * generate response of "send_data_to_session"
	 * 
	 * \param send_data_to_session_request -[in]"send_data_to_session" request
	 * \return reference of this object.
	 */
	cio_packet& set_data_from_send_data_to_session_request(
		const cio_packet& send_data_to_session_request
		, const std::wstring & s_session_dst_name
		, unsigned long n_session_src =_MP_TOOLS_INVALID_SESSION_NUMBER)
	{
		do {
			*this = send_data_to_session_request;
			//
			type_deque_wstring deque_wstring_data_field;
			if (send_data_to_session_request.get_data_field(deque_wstring_data_field) == 0)
				continue;
			//
			this->set_cmd(cio_packet::cmd_system_request)
				.set_session_number(n_session_src)
				.set_data_by_utf8(deque_wstring_data_field[0]);
			this->set_data_by_utf8(s_session_dst_name, true);

			type_deque_wstring::iterator it = std::begin(deque_wstring_data_field);
			if (n_session_src != _MP_TOOLS_INVALID_SESSION_NUMBER)
				it += 2;
			else
				++it;
			//
			for (; it != std::end(deque_wstring_data_field); ++it) {
				this->set_data_by_utf8(*it, true);
			}//end for

		} while (false);
		//
		return *this;
	}
	cio_packet& set_data_cancel(const std::wstring& s_resason = std::wstring())
	{
		set_data_by_utf8(L"cancel");
		if (!s_resason.empty()) {
			set_data_by_utf8(s_resason, true);
		}
		return *this;
	}
	bool is_success()
	{
		bool b_result(false);

		do {
			type_packect* p_packet = (type_packect*)& m_v_packet[0];

			if (!coperation::is_equal(p_packet->s_data, p_packet->n_data, std::string("success")))
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}
	bool is_error()
	{
		bool b_result(false);

		do {
			type_packect* p_packet = (type_packect*)& m_v_packet[0];

			if (!coperation::is_equal(p_packet->s_data, p_packet->n_data, std::string("error")))
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}
	bool is_cancel()
	{
		bool b_result(false);

		do {
			type_packect* p_packet = (type_packect*)&m_v_packet[0];

			if (!coperation::is_equal(p_packet->s_data, p_packet->n_data, std::string("cancel")))
				continue;
			b_result = true;
		} while (false);
		return b_result;
	}

private:
	void _set_data(const type_v_buffer& v_data, bool b_append = false)
	{
		do {
			type_packect* p_packet = (type_packect*)& m_v_packet[0];
			if (v_data.empty()) {
				if (b_append)
					continue;
				m_v_packet.resize(sizeof(type_packect) - 1);
				p_packet->n_data = 0;
				continue;
			}

			if (b_append && p_packet->n_data > 0) {
				//append
				if (m_type_data_field == cio_packet::data_field_binary) {
					m_v_packet.resize(m_v_packet.size() + v_data.size());
					p_packet = (type_packect*)& m_v_packet[0];
					coperation::copy_btb(&(p_packet->s_data[p_packet->n_data]),(unsigned int)(v_data.size()), &v_data[0], (unsigned int)(v_data.size()));
					p_packet->n_data += (unsigned long)(v_data.size());
					continue;
				}
				//utf8 case
				m_v_packet.resize(m_v_packet.size() + 1 + v_data.size());
				p_packet = (type_packect*)& m_v_packet[0];
				coperation::copy_btb(&(p_packet->s_data[p_packet->n_data - 1]), (unsigned int)(v_data.size()), &v_data[0], (unsigned int)(v_data.size()));
				p_packet->n_data += (unsigned long)(v_data.size() + 1);
				//set double null
				p_packet->s_data[p_packet->n_data-1] = 0;
				p_packet->s_data[p_packet->n_data-2] = 0;
				continue;
			}

			// new set
			if (m_type_data_field == cio_packet::data_field_binary) {
				m_v_packet.resize(sizeof(type_packect) - 1 + v_data.size());
				p_packet = (type_packect*)&m_v_packet[0];
				p_packet->n_data = (unsigned long)v_data.size();
				coperation::copy_btb(p_packet->s_data, p_packet->n_data, &v_data[0], (unsigned int)(v_data.size()));
				continue;
			}
			//utf8 case
			m_v_packet.resize(sizeof(type_packect) - 1 + v_data.size() + 2); //2 is double null termination
			p_packet = (type_packect*)&m_v_packet[0];
			p_packet->n_data = (unsigned long)v_data.size() + 2;
			coperation::copy_btb(p_packet->s_data, p_packet->n_data, &v_data[0], (unsigned int)(v_data.size()));
			//set double null
			p_packet->s_data[v_data.size()] = 0;
			p_packet->s_data[v_data.size()+1] = 0;
		} while (false);
	}

	std::string _to_json()
	{
		std::string s_format;
		nlohmann::json json_obj;
		std::string s_data;

		do {
			if (m_v_packet.empty())
				continue;
			//
			if (is_request()) {
				s_data = (char)cio_packet::cmd_request;
			}
			else if (is_self()) {
				s_data = (char)cio_packet::cmd_self_request;
			}
			else if (is_system()) {
				s_data = (char)cio_packet::cmd_system_request;
			}
			else {
				s_data = (char)cio_packet::cmd_response;
			}
			json_obj[cio_packet::_get_json_name(json_name_request_type)] = s_data;
			//
			json_obj[cio_packet::_get_json_name(json_name_session_numer)] = get_session_number();
			//
			if (is_owner_manager()) {
				s_data = (char)cio_packet::owner_manager;
			}
			else {
				s_data = (char)cio_packet::owner_device;
			}
			json_obj[cio_packet::_get_json_name(json_name_packet_owner)] = s_data;
			//
			json_obj[cio_packet::_get_json_name(json_name_device_index)] = get_device_index();
			//
			s_data = (char)get_action();
			json_obj[cio_packet::_get_json_name(json_name_action_code)] = s_data;
			//
			json_obj[cio_packet::_get_json_name(json_name_in_id)] = get_in_id();
			//
			json_obj[cio_packet::_get_json_name(json_name_out_id)] = get_out_id();


			s_data = (char)m_type_data_field;
			json_obj[cio_packet::_get_json_name(json_name_data_field_type)] = s_data;

			type_v_buffer v_data;
			get_data_field(v_data);
			//
			if (v_data.empty()) {
				json_obj[cio_packet::_get_json_name(json_name_data_field)] = nullptr;
				continue;
			}

			if (m_type_data_field == cio_packet::data_field_binary) {
				//change binary to hex string without delimiter.
				cconvert::hex_string_from_binary(s_data, v_data);
				json_obj[cio_packet::_get_json_name(json_name_data_field)] = s_data;
				continue;
			}
			//utf8 case
			// multi string to string vector
			std::vector<std::string> v_string_data;

			s_data.clear();
			type_v_buffer::iterator it = std::begin(v_data);
			for (; it != end(v_data); ++it) {
				if (*it == 0x00) {
					if (!s_data.empty()) {
						v_string_data.push_back(s_data);
						s_data.clear();
					}
				}
				else {
					s_data += (char)*it;
				}
			}//end for

			if(!s_data.empty())
				v_string_data.push_back(s_data);
			//string vector to json string array.
			json_obj[cio_packet::_get_json_name(json_name_data_field)] = v_string_data;

		} while (false);

		if (!json_obj.empty()) {
			try{
				s_format = json_obj.dump();
			}
			catch (nlohmann::json::type_error& e){
				s_format.clear();
				std::wstring ws_error;
				std::string s_error(e.what());
				ws_error.assign(std::begin(s_error), std::end(s_error));
				//ATLTRACE(L" = ERROR : cio_packet::_to_json : %ls.\n", ws_error.c_str());
			}
		}
		return s_format;
	}

	static const std::string _get_json_name(const _type_json_name json_name)
	{
		static const std::string s_json_request_type("request_type");
		static const std::string s_json_session_numer("session_number");
		static const std::string s_json_packet_owner("packet_owner");
		static const std::string s_json_device_index("device_index");
		static const std::string s_json_action_code("action_code");
		static const std::string s_json_name_in_id("in_id");
		static const std::string s_json_name_out_id("out_id");
		static const std::string s_json_data_field_type("data_field_type");
		static const std::string s_json_data_field("data_field");
		std::string s_name;
		switch (json_name) {
		case cio_packet::json_name_request_type:
			s_name = s_json_request_type; break;
		case cio_packet::json_name_session_numer:
			s_name = s_json_session_numer; break;
		case cio_packet::json_name_packet_owner:
			s_name = s_json_packet_owner; break;
		case cio_packet::json_name_device_index:
			s_name = s_json_device_index; break;
		case cio_packet::json_name_action_code:
			s_name = s_json_action_code; break;
		case cio_packet::json_name_in_id:
			s_name = s_json_name_in_id; break;
		case cio_packet::json_name_out_id:
			s_name = s_json_name_out_id; break;
		case cio_packet::json_name_data_field_type:
			s_name = s_json_data_field_type; break;
		case cio_packet::json_name_data_field:
			s_name = s_json_data_field; break;
		default:
			break;
		}//end switch
		return s_name;
	}
	static bool _is_valid(const type_v_buffer & v_packet)
	{
		bool b_result(false);

		do {
			if (v_packet.size() < (sizeof(cio_packet::type_packect)-1))
				continue;
			type_packect *p_packet = (type_packect *)&v_packet[0];
			switch (p_packet->c_cmd) {
			case cio_packet::cmd_request:
			case cio_packet::cmd_response:
			case cio_packet::cmd_self_request:
			case cio_packet::cmd_system_request:
				break;
			default:
				continue;
			}//end switch
			switch (p_packet->c_owner) {
			case cio_packet::owner_manager:
			case cio_packet::owner_device:
				break;
			default:
				continue;
			}//end switch

			switch (p_packet->c_act) {
			case cio_packet::act_mgmt_unknown:
			case cio_packet::act_mgmt_get_device_list:
			case cio_packet::act_mgmt_get_echo:
			case cio_packet::act_mgmt_ctl_show:
			case cio_packet::act_mgmt_file_operation:
			case cio_packet::act_mgmt_advance_operation:
			case cio_packet::act_mgmt_dev_plug_in:
				if (p_packet->c_owner == cio_packet::owner_manager)
					break;
				else
					continue;
			case cio_packet::act_dev_open:
			case cio_packet::act_dev_close:
			case cio_packet::act_dev_write:
			case cio_packet::act_dev_read:
			case cio_packet::act_dev_transmit:
			case cio_packet::act_dev_cancel:
			case cio_packet::act_dev_sub_bootloader:
				if (p_packet->c_owner == cio_packet::owner_device)
					break;
				else
					continue;
			case cio_packet::act_mgmt_dev_kernel_operation:
				break;
			default:
				continue;
			}//end switch
			if (v_packet.size() != (size_t)(sizeof(cio_packet::type_packect)-1 + p_packet->n_data))
				continue;
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	static type_v_buffer _build(
		type_cmd c_cmd, unsigned long n_session
		, type_owner c_owner, unsigned long dw_device_index, type_act c_act
		,unsigned char c_in_id,unsigned char c_out_id
		,unsigned long n_data, const unsigned char* ps_data)
	{
		type_v_buffer v_packet(0);

		do {
			if (n_data > 0 && ps_data == NULL)
				continue;
			v_packet.resize(sizeof(type_packect) - 1 + n_data, 0);

			type_packect* p_packet = (type_packect*)& v_packet[0];
			p_packet->c_cmd = c_cmd;
			p_packet->n_session = n_session;
			p_packet->c_owner = c_owner;
			p_packet->dw_device_index = dw_device_index;
			p_packet->c_act = c_act;
			p_packet->c_in_id = c_in_id;
			p_packet->c_out_id = c_out_id;
			p_packet->n_data = n_data;
			if (n_data > 0)
				memcpy(p_packet->s_data, ps_data, n_data);
			//
			if (!cio_packet::_is_valid(v_packet)) {
				v_packet.resize(0);
			}

		} while (false);

		return v_packet;
	}

	/**
	* NOTICE : if the type of data is string in json_obj, the substring "::" of data will be changed to ":".
	*/
	static cio_packet::type_ptr _build(const nlohmann::json & json_obj)
	{
		cio_packet::type_ptr ptr_cio_packet;
		type_v_buffer binary_format(0);
		cio_packet::type_data_field data_field_type = cio_packet::data_field_binary;
		bool b_result(false);
		std::string s_data;

		do {
			type_v_buffer v_long(sizeof(unsigned long), 0);
			unsigned long* p_long = (unsigned long*)& v_long[0];

			type_v_buffer v_uchar(sizeof(unsigned char), 0);
			unsigned char* p_uchar = (unsigned char*)&v_uchar[0];

			type_v_buffer v_hex(0);

			nlohmann::json::const_iterator cit = begin(json_obj);
			{
				//request type
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_request_type));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_string())
					continue;
				s_data = cit->get<std::string>();
				if (!cio_packet::_is_valid_cmd(s_data))
					continue;
				binary_format.push_back((unsigned char)s_data[0]);
			}
			{
				//session
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_session_numer));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_number())
					continue;
				*p_long = cit->get<unsigned long>();
				coperation::append_btb(binary_format, v_long);
			}
			{
				//owner
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_packet_owner));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_string())
					continue;
				s_data = cit->get<std::string>();
				if (!cio_packet::_is_valid_owner(s_data))
					continue;
				binary_format.push_back((unsigned char)s_data[0]);
			}
			{
				//device index
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_device_index));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_number())
					continue;
				*p_long = cit->get<unsigned long>();
				coperation::append_btb(binary_format, v_long);
			}
			{
				//action code
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_action_code));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_string())
					continue;
				s_data = cit->get<std::string>();
				if (!cio_packet::_is_valid_action_code(s_data))
					continue;
				binary_format.push_back((unsigned char)s_data[0]);
			}
			{
				//in id
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_in_id));
				if (cit != end(json_obj)) {
					if (!cit->is_number())
						continue;
					*p_uchar = cit->get<unsigned char>();
				}
				else
					*p_uchar = 0;//using default id
				coperation::append_btb(binary_format, v_uchar);
			}
			{
				//out id
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_out_id));
				if (cit != end(json_obj)) {
					if (!cit->is_number())
						continue;
					*p_uchar = cit->get<unsigned char>();
				}
				else
					*p_uchar = 0;//using default id
				coperation::append_btb(binary_format, v_uchar);
			}
			{
				//data field type
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_data_field_type));
				if (cit == end(json_obj))
					continue;
				if (!cit->is_string())
					continue;
				s_data = cit->get<std::string>();
				if (!cio_packet::_is_valid_data_field_type(s_data))
					continue;
				data_field_type = (cio_packet::type_data_field)s_data[0];
			}
			{
				//data field length
				*p_long = 0;
				coperation::append_btb(binary_format, v_long);
			}
			{
				//data field
				bool b_add_double_null(false);
				std::vector<std::string>	v_string_data;
				b_result = true;
				cit = json_obj.find(cio_packet::_get_json_name(cio_packet::json_name_data_field));
				if (cit == end(json_obj))
					continue;//no data field
				if (cit->is_array()) {
					try {
						cit->get_to(v_string_data);

						for (size_t i(0); i < v_string_data.size(); i++) {
							if (v_string_data[i].empty())
								continue;
							//
							for (auto c_item : v_string_data[i]) {
								binary_format.push_back((unsigned char)c_item);
							}//end for
							binary_format.push_back((unsigned char)0x00);//for string null termination
							b_add_double_null = true;
						}//end for

						if (b_add_double_null)
							binary_format.push_back((unsigned char)0x00);//for double  null termination
					}
					catch (nlohmann::json::exception& e) {
						b_result = false;
						std::wstring ws_error;
						std::string s_error(e.what());
						ws_error.assign(std::begin(s_error), std::end(s_error));
						//ATLTRACE(L" = ERROR : cio_packet::_build : %ls.\n", ws_error.c_str());
					}

				}
				else if (cit->is_string()){
					s_data = cit->get<std::string>();
					if (data_field_type == cio_packet::data_field_binary) {
						cstring::remove_white_space_of_prepostfix(s_data);
						if (s_data.empty())
							continue;//no data field

						std::string s_seperator;//supported seperator is none or space.

						if (s_data.find(' ') != std::string::npos) {
							s_seperator = " ";
						}
						else {
							//no seperator
							if (s_data.size() % 2 != 0) {
								b_result = false;//mismatching format.
								continue;
							}

							s_seperator = "";
						}
						//
						cconvert::binary_from_hex_string(v_hex, s_data, s_seperator);
						if (v_hex.empty()) {
							b_result = false;//mismatching format.
							continue;
						}
						coperation::append_btb(binary_format, v_hex);
					}
					else {
						// replace "::" -> ":" ~~~  warning
						int n_col = 0;//colron counter
						for (auto c_item : s_data) {
							if (c_item == ':') {
								if (n_col != 0) {
									n_col = 0;
									continue;//bypass ':'
								}
								++n_col;
							}
							else {
								n_col = 0;
							}
							binary_format.push_back((unsigned char)c_item);
						}//end for
						if (!s_data.empty()) {
							binary_format.push_back((unsigned char)0x00);//for string null termination
							binary_format.push_back((unsigned char)0x00);//for double  null termination
						}
					}
				}
				else if (cit->is_object()) {
					//not yet supported!
					b_result = false;
				}
			}
		} while (false);

		if (b_result) {
			//adjust data fields length
			cio_packet::type_packect* p_packet = (cio_packet::type_packect*)(&binary_format[0]);
			p_packet->n_data = (unsigned long)(binary_format.size() - (sizeof(cio_packet::type_packect) - 1));
			auto ptr_n = std::make_shared<cio_packet>(data_field_type, binary_format);
			ptr_cio_packet.swap(ptr_n);
		}
		return ptr_cio_packet;
	}
	//
	static bool _is_valid_data_field_type(cio_packet::type_data_field c_data_field)
	{
		bool b_result(false);

		switch (c_data_field)
		{
		case cio_packet::data_field_binary:
		case cio_packet::data_field_string_utf8:
			b_result = true;
			break;
		default:
			break;
		}
		return b_result;
	}
	static bool _is_valid_data_field_type(const std::string& s_data_field)
	{
		bool b_result(false);

		do {
			if (s_data_field.size() != 1)
				continue;
			cio_packet::type_data_field c_data_field = (cio_packet::type_data_field)s_data_field[0];
			b_result = _is_valid_data_field_type(c_data_field);
		} while (false);
		return b_result;
	}
	static bool _is_valid_owner(cio_packet::type_owner c_owner)
	{
		bool b_result(false);

		switch (c_owner)
		{
		case cio_packet::owner_manager:
		case cio_packet::owner_device:
			b_result = true;
			break;
		default:
			break;
		}
		return b_result;
	}
	static bool _is_valid_owner(const std::string& s_owner)
	{
		bool b_result(false);

		do {
			if (s_owner.size() != 1)
				continue;
			cio_packet::type_owner c_owner = (cio_packet::type_owner)s_owner[0];
			b_result = _is_valid_owner(c_owner);
		} while (false);
		return b_result;
	}
	static bool _is_valid_action_code(cio_packet::type_act c_act)
	{
		bool b_result(false);

		switch (c_act)
		{
		case cio_packet::act_mgmt_unknown:
		case cio_packet::act_mgmt_get_echo:
		case cio_packet::act_mgmt_get_device_list:
		case cio_packet::act_mgmt_ctl_show:
		case cio_packet::act_mgmt_file_operation:
		case cio_packet::act_mgmt_advance_operation:
		case cio_packet::act_mgmt_dev_kernel_operation:
		case cio_packet::act_mgmt_dev_plug_in:
		case cio_packet::act_dev_open:
		case cio_packet::act_dev_close:
		case cio_packet::act_dev_write:
		case cio_packet::act_dev_read:
		case cio_packet::act_dev_transmit:
		case cio_packet::act_dev_cancel:
		case cio_packet::act_dev_sub_bootloader:
			b_result = true;
			break;
		default:
			break;
		}
		return b_result;
	}
	static bool _is_valid_action_code(const std::string& s_act)
	{
		bool b_result(false);

		do {
			if (s_act.size() != 1)
				continue;
			cio_packet::type_act c_act = (cio_packet::type_act)s_act[0];
			b_result = _is_valid_action_code(c_act);
		} while (false);
		return b_result;
	}
	static bool _is_valid_cmd(cio_packet::type_cmd c_cmd)
	{
		bool b_result(false);

		switch (c_cmd)
		{
		case cio_packet::cmd_request:
		case cio_packet::cmd_response:
		case cio_packet::cmd_self_request:
		case cio_packet::cmd_system_request:
			b_result = true;
			break;
		default:
			break;
		}
		return b_result;
	}
	static bool _is_valid_cmd(const std::string & s_cmd)
	{
		bool b_result(false);

		do {
			if (s_cmd.size() != 1)
				continue;
			cio_packet::type_cmd c_cmd = (cio_packet::type_cmd)s_cmd[0];
			b_result = _is_valid_cmd(c_cmd);
		} while (false);
		return b_result;
	}
protected:
	cio_packet::type_data_field m_type_data_field;
	
};

}//the end of _mp namespace