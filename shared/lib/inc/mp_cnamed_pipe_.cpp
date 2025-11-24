#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/interprocess/permissions.hpp>

#include <mp_cnamed_pipe_.h>
#include <mp_cstring.h>

#ifdef _WIN32
#include <windows.h>
#include <sddl.h>
// D:(A;;GA;;;WD) -> DACL: Allow Generic All to World (Everyone)
const char* SDDL_EVERYONE_FULL_ACCESS = "D:(A;;GA;;;WD)";

static PSECURITY_ATTRIBUTES _create_sa_from_sddl();

PSECURITY_ATTRIBUTES _create_sa_from_sddl() {
	PSECURITY_DESCRIPTOR pSecDesc = nullptr;
	PSECURITY_ATTRIBUTES pSecAttr = new SECURITY_ATTRIBUTES;

	// SDDL 문자열을 실제 보안 디스크립터로 변환
	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
		SDDL_EVERYONE_FULL_ACCESS,
		SDDL_REVISION_1,
		&pSecDesc,
		NULL))
	{
		delete pSecAttr;
		return nullptr;
	}

	pSecAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
	pSecAttr->lpSecurityDescriptor = pSecDesc;
	pSecAttr->bInheritHandle = FALSE;

	return pSecAttr;
}
#endif

// need link. boost_thread.lib, boost_system.lib
namespace _mp {

	/**
	* static member
	*/
	std::string cnamed_pipe::generated_pipe_name()
	{
		boost::uuids::uuid uuid = boost::uuids::random_generator()();
		return "_20240725::_mp::cnamed_pipe::" + boost::uuids::to_string(uuid);
	}

	bool cnamed_pipe::remove_forcelly(const std::string& s_name)
	{
		return boost::interprocess::message_queue::remove(s_name.c_str());
	}

	cnamed_pipe::~cnamed_pipe()
	{
		if (m_b_i_am_creator) {
			if (m_ptr_mq) {
				boost::interprocess::message_queue::remove(
					m_s_name.c_str()
				);
				m_ptr_mq.reset();
			}
		}
	}

	cnamed_pipe::cnamed_pipe() :
		m_b_i_am_creator(true)
	{
		m_s_name = generated_pipe_name();

		boost::interprocess::message_queue::remove(m_s_name.c_str());

		m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
			boost::interprocess::create_only,
			m_s_name.c_str(),
			cnamed_pipe::_max_number_of_message,//max message
			cnamed_pipe::_max_size_of_one_message_unit_byte//the max size of each message.
		);
	}

	cnamed_pipe::cnamed_pipe(const std::string& s_name, bool b_creator)
	{
#ifdef _WIN32
		PSECURITY_ATTRIBUTES pSA = NULL;
#endif //_WIN32

		m_s_name = s_name;
		m_b_i_am_creator = b_creator;

		try {
			if (m_b_i_am_creator) {
				boost::interprocess::message_queue::remove(m_s_name.c_str());
#ifdef _WIN32
				boost::interprocess::permissions perm;
				perm.set_unrestricted();//perm.set_permissions(pSA);

				m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
					boost::interprocess::create_only,
					m_s_name.c_str(),
					cnamed_pipe::_max_number_of_message,//max message
					cnamed_pipe::_max_size_of_one_message_unit_byte,//the max size of each message.
					perm
				);
#else
				m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
					boost::interprocess::create_only,
					m_s_name.c_str(),
					cnamed_pipe::_max_number_of_message,//max message
					cnamed_pipe::_max_size_of_one_message_unit_byte//the max size of each message.
				);

#endif // _WIN32

			}
			else {
				m_ptr_mq = std::make_shared<boost::interprocess::message_queue>(
					boost::interprocess::open_only,
					m_s_name.c_str()
				);
			}
		}
		catch (boost::interprocess::interprocess_exception& ex) {
			m_ptr_mq.reset();
		}
		catch (...) {
			m_ptr_mq.reset();
		}

#ifdef _WIN32
		if (pSA) {
			LocalFree(pSA->lpSecurityDescriptor);
			delete pSA;
		}
#endif //_WIN32

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

		do {
			s_data.clear();
			if(!b_result) {
				continue;
			}

			b_result = false;

			if (v_data.empty()) {
				continue;
			}
			if (v_data.size() % sizeof(wchar_t) != 0) {
				continue;
			}

			size_t ns_data = v_data.size() / sizeof(wchar_t);
			wchar_t* ps_data = reinterpret_cast<wchar_t*>(&v_data[0]);

			if( *ps_data == 0) {
				//if the first wchar_t is NULL, then it is empty string.
				continue;
			}

			for(size_t i = 0; i < ns_data; i++) {
				if (ps_data[i] == 0) {
					//if the wchar_t is NULL, then it is end of string.
					break; //exit for
				}
				s_data.push_back(ps_data[i]);
			}//end for

			b_result = true;

		} while (false);

		return b_result;
	}

}//the end of _mp namespace

