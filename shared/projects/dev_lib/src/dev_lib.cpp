
#include <dev_lib.h>

#ifndef _WIN32
//linux only
static void _so_init(void) __attribute__((constructor));
static void _so_fini(void) __attribute__((destructor));
//when calls dlopen().
void _so_init(void)
{
	//printf("Shared library loaded\n");
	// NOT executed
}

//when calls dlclose().
void _so_fini(void)
{
	//printf("Shared library unloaded\n");
	// NOT executed
}
#endif // _WIN32


uint32_t _CALLTYPE_ dev_lib_on()
{
	uint32_t n_result(DEV_LIB_RESULT_ERROR);

	do {

	} while (false);
	return n_result;
}


uint32_t _CALLTYPE_ dev_lib_off()
{
	uint32_t n_result(DEV_LIB_RESULT_ERROR);

	do {

	} while (false);
	return n_result;

}
