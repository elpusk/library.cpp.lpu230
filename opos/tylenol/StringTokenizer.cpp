#include "StdAfx.h"
#include "StringTokenizer.h"


CStringTokenizer::CStringTokenizer( const _tstring & sInput, const _tstring & sSeperator ):
m_sInput(sInput), m_sDelimiter(sSeperator)
{
	split();
}


CStringTokenizer::~CStringTokenizer(void)
{
}

size_t CStringTokenizer::countTokens()		//the number of token
{
	return m_vToken.size();
}


bool CStringTokenizer::hasMoreTokens()		//is exsit token
{
	return (m_iterIndex != m_vToken.end() );
}

_tstring CStringTokenizer::nextToken()		//next token
{
	if( m_iterIndex != m_vToken.end() )
		return *(m_iterIndex++);
	else
		return _T("");
}

void CStringTokenizer::split()					//string -> vector
{
	_tstring::size_type lastPos = m_sInput.find_first_not_of( m_sDelimiter, 0 ); //구분자가 나타나지 않는 위치
	_tstring::size_type Pos = m_sInput.find_first_of( m_sDelimiter, lastPos ); //구분자가 나타나는 위치

	while(string::npos!=Pos || string::npos!=lastPos){

		m_vToken.push_back(m_sInput.substr( lastPos, Pos-lastPos ) );
		lastPos = m_sInput.find_first_not_of( m_sDelimiter, Pos ); //구분자가 나타나지 않는 위치
		Pos = m_sInput.find_first_of( m_sDelimiter, lastPos ); //구분자가 나타나는 위치
	}

	m_iterIndex = m_vToken.begin();
}
