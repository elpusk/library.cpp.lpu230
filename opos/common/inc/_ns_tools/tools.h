#pragma once

// all static method for tools function

#include <string>
#include <tchar.h>
#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include <fstream>

#include <Windows.h>


#include <ct_type.h>

using namespace std;

class ctools {

public:
	enum {
		SIZE_TIME_YYMMDD = 6
	};
	enum {
		SIZE_TIME_HHMMSS = 6
	};

public:
	~ctools() {}

private://don't call these files.
	ctools();
	ctools(const ctools &);
	ctools& operator=(const ctools &);
};