#pragma once

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/exceptions.hpp>

#include <mp_type.h>
#include <mp_cstring.h>

namespace _mp {

	class csystem
	{
	public:

		/**
		* good in Windows.
		* none operation in debian12. 
		*/
		static std::shared_ptr<boost::interprocess::file_lock> get_file_lock_for_single_instance(const std::wstring& s_name);

		/**
		* in this funcation, parent process will be terminated with _exit(0)
		*/
		static bool daemonize_on_linux(const std::wstring  & s_daemon_name,void(*signal_handler)(int));

	public:
		virtual ~csystem();

	private:

	private://don't call these methods
		csystem();
		csystem(const csystem&);
		csystem& operator=(const csystem&);

	};

}//the end of _mp namespace

