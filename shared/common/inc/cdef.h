#pragma once
#include <string>

class cdef
{

public:
	~cdef()
	{
	}
private:
	cdef();
	cdef(const cdef&);
	cdef& operator=(const cdef&);
};