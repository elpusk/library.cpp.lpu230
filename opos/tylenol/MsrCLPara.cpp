#include "StdAfx.h"
#include "MsrCLPara.h"
#include <algorithm>

struct functor_argv{
	void operator () ( const _tstring & argv )
	{
		_tout << argv << endl;
	}
};

CMsrCLPara::CMsrCLPara( INT32 argc, _TCHAR* argv[] ) :
m_nLoop(1), m_bBoot(false), m_bIsSetReq(false)
{
	Load( argc, argv );
}


CMsrCLPara::~CMsrCLPara(void)
{
}

// parameter is 
// :boot - run bootload.
// :set - run on setting mode.
// digit - loop count
void CMsrCLPara::Load(  INT32 argc, _TCHAR* argv[] )
{
	CUserPara::Load( argc, argv );

	if( GetSize() > 1 ){

		for( INT32 i = 1; i<GetSize(); i++ ){

			if( m_vArg[i].compare( _T(":boot") ) == 0 ){
				m_bBoot = true;
			}
			else if( m_vArg[i].compare( _T(":set") ) == 0 ){
				m_bIsSetReq = true;
			}
			else{

				try{
					m_nLoop = stoi( m_vArg[i] );
				}
				catch( invalid_argument & ex ){
					_tstring sFile( m_vArg[i] );
					m_FileName = _tstring( _T(".\\") ) + sFile;
				}
				catch( out_of_range & ex ){
					m_nLoop = 1;
				}
			}

		}//end for

	}

	Display();
}

void CMsrCLPara::Display()
{
	_tout << _T( "* Command Line parameters") << endl << _T("================================") <<endl;

	//diaplay argvs
	//for_each( m_vArg.begin(), m_vArg.end(), functor_argv() );
	//_tout <<  _T("----------------------------------------") <<endl;

	if( m_bBoot ){
		_tout << _T(" - The current request is goto boot.!!!") <<endl;
	}

	if( m_bIsSetReq ){
		_tout << _T(" - The current request is set mode.!!!") <<endl;
	}
	else{
		_tout << _T(" - The current request is get mode.!!!") <<endl;
	}

	_tout << _T(" - Looping counter value is ") <<to_tstring( static_cast<unsigned long long>(m_nLoop) ) << endl;

	_tout << _T(" - APP NAME : ") << GetAppName() << endl;
	_tout << _T(" - INI FILE  NAME : ") << GetIniFileName() << endl;

}

const _tstring & CMsrCLPara::GetAppName()
{
	if( m_vArg.size() < 0 )
		return CUserPara::NullString;
	else
		return m_vArg[0];
}





