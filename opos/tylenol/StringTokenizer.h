#pragma once

#include "TTR_tchar.h"
#include <vector>

using namespace std;


class CStringTokenizer
{
public:
	CStringTokenizer( const _tstring & sInput, const _tstring & sSeperator );
	virtual ~CStringTokenizer(void);

	size_t countTokens();		//the number of token
	bool hasMoreTokens();		//is exsit token
	_tstring nextToken();		//next token
	void split();					//string -> vector

private:
	CStringTokenizer(void);

private:
	_tstring m_sInput;
	_tstring m_sDelimiter;
	vector<_tstring> m_vToken;
	vector<_tstring>::iterator m_iterIndex;
};

