

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <iostream>
#include <vector>

#include "tp_main.h"

static std::set<std::wstring> _get_command_line_parameters(int argc, char* argv[]);
static std::wstring _get_unicode_from_mcsc(const std::string& s_mcsc);

std::set<std::wstring> _get_command_line_parameters(int argc, char* argv[])
{
	std::set<std::wstring> set_parameters;

	do {
		for (int i = 0; i < argc; i++) {
			if (argv[i]) {
				set_parameters.insert(_get_unicode_from_mcsc(argv[i]));
			}
		}//end for
	} while (false);
	return set_parameters;
}

static std::wstring _get_unicode_from_mcsc(const std::string& s_mcsc)
{
	std::wstring s_unicode;

	do {
		if (s_mcsc.empty())
			continue;
		//
#ifdef _WIN32
		size_t size_needed = 0;
		mbstowcs_s(&size_needed, nullptr, 0, s_mcsc.c_str(), _TRUNCATE);
		if (size_needed > 0)
		{
			s_unicode.resize(size_needed);
			mbstowcs_s(&size_needed, &s_unicode[0], size_needed, s_mcsc.c_str(), _TRUNCATE);
			//mbstowcs_s function copy the last NULL. but std::wstring(or std::string) no need the last NULL
			//therefore .. need remove the last NULL from std::wstring(or std::string)
			if (!s_unicode.empty()) {
				s_unicode.resize(s_unicode.size() - 1);
			}
		}
#else
		size_t size_needed = std::mbstowcs(nullptr, s_mcsc.c_str(), 0);
		if (size_needed != (size_t)-1)
		{
			std::vector<wchar_t> v_unicode(size_needed + 1, 0);
			if (!v_unicode.empty()) {
				std::mbstowcs(&v_unicode[0], s_mcsc.c_str(), size_needed);
				s_unicode = &v_unicode[0];
			}
		}
#endif
		else
		{
			s_unicode.clear();//default for erro
		}

	} while (false);

	return s_unicode;
}


int main(int argc, char* argv[])
{
	int n_result(EXIT_FAILURE);
	bool b_help(false);

	do {
		//get command line parameters 
		std::set<std::wstring> set_parameters(_get_command_line_parameters(argc, argv));

		if (set_parameters.size() < 2) {
			b_help = true;
			continue;
		}

		if (set_parameters.find(L"/ibutton") != std::end(set_parameters)) {
			//ibutton test
			if (set_parameters.find(L"/getdataincallback") != std::end(set_parameters)) {
				n_result = main_lpu237_ibutton_getdataincallback(set_parameters);
				continue;
			}
			// default thread safty code style
			n_result = main_lpu237_ibutton_threadsafty(set_parameters);
			continue;
		}
		if (set_parameters.find(L"/msr") != std::end(set_parameters)) {
			//msr test
			if (set_parameters.find(L"/getdataincallback") != std::end(set_parameters)) {
				n_result = main_lpu237_dll_getdataincallback(set_parameters);
				continue;
			}
			// default thread safty code style
			n_result = main_lpu237_dll_threadsafty(set_parameters);
			continue;
		}

		b_help = true;
	} while (false);

	if(b_help) {
		std::wcout << L"Usage: [/ibutton or /msr]" << std::endl;
		std::wcout << L"  /ibutton : run ibutton test." << std::endl;
		std::wcout << L"  /msr : run msr test." << std::endl;
		std::wcout << L"  /threadsafty : use threadsafty-code-style.(default)" << std::endl;
		std::wcout << L"  /getdataincallback : get ms-data in callback function." << std::endl;
		n_result = EXIT_SUCCESS;
	}

	return n_result;
}