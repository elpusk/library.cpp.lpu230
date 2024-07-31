#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <mp_cnamed_pipe_.h>
#include <mp_cstring.h>

// need link. boost_thread.lib, boost_system.lib
namespace _mp {

	/**
	* static member
	*/
	std::wstring cnamed_pipe::generated_pipe_name()
	{
		boost::uuids::uuid uuid = boost::uuids::random_generator()();
		return L"_20240725::_mp::cnamed_pipe::" + boost::uuids::to_wstring(uuid);
	}

	cnamed_pipe::~cnamed_pipe()
	{
#ifdef _WIN32
		std::wstring s(m_s_name);
#else
		std::string s(cstring::get_mcsc_from_unicode(m_s_name));
#endif
		if (m_b_i_am_creator) {
			if (m_ptr_mq) {
				boost::interprocess::message_queue::remove(
					s.c_str()
				);
				m_ptr_mq.reset();
			}
		}
	}

	cnamed_pipe::cnamed_pipe() :
		m_b_i_am_creator(true)
	{
		m_s_name = generated_pipe_name();

#ifdef _WIN32
		std::wstring s(m_s_name);
#else
		std::string s(cstring::get_mcsc_from_unicode(m_s_name));
#endif
		boost::interprocess::message_queue::remove(s.c_str());

		m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
			boost::interprocess::create_only,
			s.c_str(),
			cnamed_pipe::_max_number_of_message,//max message
			cnamed_pipe::_max_size_of_one_message_unit_byte//the max size of each message.
		);
	}

	cnamed_pipe::cnamed_pipe(const std::wstring& s_name, bool b_creator)
	{
		m_s_name = s_name;
		m_b_i_am_creator = b_creator;

#ifdef _WIN32
		std::wstring s(m_s_name);
#else
		std::string s(cstring::get_mcsc_from_unicode(m_s_name));
#endif
		try {
			if (m_b_i_am_creator) {
				boost::interprocess::message_queue::remove(s.c_str());
				m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
					boost::interprocess::create_only,
					s.c_str(),
					cnamed_pipe::_max_number_of_message,//max message
					cnamed_pipe::_max_size_of_one_message_unit_byte//the max size of each message.
				);
			}
			else {
				m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
					boost::interprocess::open_only,
					s.c_str()
				);
			}
		}
		catch (boost::interprocess::interprocess_exception& ex) {
			m_ptr_mq.reset();
		}
	}

	bool cnamed_pipe::is_ini() const
	{
		if (m_ptr_mq)
			return true;
		else
			return false;
	}
	bool cnamed_pipe::write(const type_v_buffer& v_data)
	{
		bool b_result(false);

		do {
			if (!m_ptr_mq) {
				continue;
			}
			if (v_data.empty()) {
				continue;
			}

			if (m_ptr_mq->get_max_msg() <= m_ptr_mq->get_num_msg()) {
				type_v_buffer v_rx;
				this->read(v_rx);//reduce q item.
			}

			try {
				if (m_ptr_mq->get_max_msg_size() < v_data.size()) {
					b_result = m_ptr_mq->try_send(&v_data[0], m_ptr_mq->get_max_msg_size(), cnamed_pipe::_def_priority);
				}
				else {
					b_result = m_ptr_mq->try_send(&v_data[0], v_data.size(), cnamed_pipe::_def_priority);
				}
			}
			catch (boost::interprocess::interprocess_exception& ex) {
				b_result = false;
			}
			
		} while (false);
		return b_result;
	}

	bool cnamed_pipe::write(const std::wstring& s_data)
	{
		type_v_buffer v_data((s_data.size() + 1) * sizeof(wchar_t));
		wchar_t c = 0;
		for (size_t i = 0; i < s_data.size(); i++) {
			c = s_data[i];
			memcpy(&v_data[sizeof(wchar_t) * i], &c, sizeof(c));
		}
		return this->write(v_data);
	}

	bool cnamed_pipe::read(type_v_buffer& v_data)
	{
		bool b_result(false);

		do {
			if (!m_ptr_mq) {
				continue;
			}

			type_v_buffer v(m_ptr_mq->get_max_msg_size(), 0);
			//boost::ulong_long_type n_rx(0);
			boost::interprocess::message_queue_t<boost::interprocess::offset_ptr<void> >::size_type n_rx(0);
			unsigned int n_prioroty(0);
			try {
				b_result = m_ptr_mq->try_receive(&v[0], v.size(), n_rx, n_prioroty);
			}
			catch (boost::interprocess::interprocess_exception& ex) {
				v_data.resize(0);
				b_result = false;
			}

			if (b_result) {
				v_data.resize(n_rx, 0);
				std::copy(v.begin(), v.begin() + n_rx, v_data.begin());
			}

		} while (false);
		return b_result;

	}

	bool cnamed_pipe::read(std::wstring& s_data)
	{
		type_v_buffer v_data;
		bool b_result = this->read(v_data);

		if (b_result) {
			if (v_data.size() % sizeof(wchar_t) != 0) {
				b_result = false;
			}
			else {
				wchar_t c;
				for (size_t i = 0; i < v_data.size() / sizeof(wchar_t); i++) {
					memcpy(&c, &v_data[i * sizeof(wchar_t)], sizeof(c));
					s_data.push_back(c);
				}//end for
			}
		}

		return b_result;
	}

}//the end of _mp namespace

