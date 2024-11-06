#include <main.h>

#include <iostream>

#include <mp_type.h>
#include <mp_clog.h>

/**
 * remove all installed file and paths.
 * TODO. not yet!
 */
int main_remove_all(const _mp::type_set_wstring& set_parameters)
{
	_mp::clog::get_instance().trace(L"[I] - %ls - %ls - remove all.\n", __WFILE__, __WFUNCTION__);

	return cdef_const::exit_error_not_supported;
}