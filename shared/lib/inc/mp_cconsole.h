#pragma once

#include <memory>
#include <iostream>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <mp_type.h>
#include <mp_cstring.h>

namespace _mp {

	class cconsole
	{

	public:
		typedef	std::weak_ptr< cconsole >		type_wptr;
		typedef	std::shared_ptr< cconsole >	type_ptr;

	public:
		static cconsole& get_instance()
		{
			static cconsole obj;
			return obj;
		}
		virtual ~cconsole()
		{
		}

	private:

		cconsole()
		{

		}
#ifdef _WIN32
#else
#endif

	private:
#ifdef _WIN32
#else
#endif

	private://don't call these methods
		
		cconsole(const cconsole&);
		cconsole& operator=(const cconsole&);

	};

}//the end of _mp namespace

