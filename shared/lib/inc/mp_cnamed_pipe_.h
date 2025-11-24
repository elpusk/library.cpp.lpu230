#pragma once

#include <string>
#include <memory>

#include <boost/interprocess/ipc/message_queue.hpp>

#include <mp_type.h>

namespace _mp {

	/**
	* Persistence - Kernel or Filesystem.
	*/
	class cnamed_pipe
	{
	public:
		typedef	std::shared_ptr< cnamed_pipe > type_ptr;

	private:
		enum {
			_max_number_of_message = 512
		};
		enum {
			_max_size_of_one_message_unit_byte = 512* sizeof(wchar_t)
		};
		enum :unsigned int{
			_def_priority = 1
		};

	private:
		typedef	std::shared_ptr<boost::interprocess::message_queue> _type_ptr_mq;

	public:
		static std::string generated_pipe_name();

		static bool remove_forcelly(const std::string& s_name);

	public:
		virtual ~cnamed_pipe();
		cnamed_pipe();
		cnamed_pipe(const std::string & s_name,bool b_creator);

		std::string get_name() const
		{
			return m_s_name;
		}

		/**
		* this function isn't blocked.
		*/
		bool write(const type_v_buffer & v_data);
		bool write(const std::wstring& s_data);

		bool read(type_v_buffer& v_data);
		bool read(std::wstring& s_data);

		bool is_ini() const;

		size_t get_max_size_of_one_message() const
		{
			if (m_ptr_mq) {
				return (size_t)(m_ptr_mq->get_max_msg_size());
			}
			else {
				return 0;
			}
		}

	private:
		std::string m_s_name;
		bool m_b_i_am_creator;
		cnamed_pipe::_type_ptr_mq m_ptr_mq;
	private:
		

	private://don't call these methods
		
		cnamed_pipe(const cnamed_pipe&);
		cnamed_pipe& operator=(const cnamed_pipe&);

	};

}//the end of _mp namespace

