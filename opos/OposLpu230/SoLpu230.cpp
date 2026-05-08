// SoLpu230.cpp : CSoLpu230의 구현입니다.

#include "stdafx.h"
#include "SoLpu230.h"

using namespace std;

STDMETHODIMP CSoLpu230::OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::OpenService().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->OpenService( DeviceClass, DeviceName, pDispatch, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::GetPropertyNumber(long PropIndex, long* pNumber)
{
	ATLTRACE(_T(" @CSoLpu230::GetPropertyNumber().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->GetPropertyNumber( PropIndex, pNumber );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::SetPropertyNumber(long PropIndex, long Number)
{
	ATLTRACE(_T(" @CSoLpu230::SetPropertyNumber().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->SetPropertyNumber( PropIndex, Number );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::GetPropertyString(long PropIndex, BSTR* pString)
{
	ATLTRACE(_T(" @CSoLpu230::GetPropertyString().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->GetPropertyString( PropIndex, pString );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::SetPropertyString(long PropIndex, BSTR bstrString)
{
	ATLTRACE(_T(" @CSoLpu230::SetPropertyString().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->SetPropertyString( PropIndex, bstrString );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::GetOpenResult(long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::GetOpenResult().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->GetOpenResult( pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::COFreezeEvents(VARIANT_BOOL Freeze, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::COFreezeEvents().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->COFreezeEvents( Freeze, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::CheckHealth(long Level, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::CheckHealth().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->CheckHealth( Level, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::ClaimDevice(long lTimeout, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::ClaimDevice().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->ClaimDevice( lTimeout, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::ClearInput(long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::ClearInput().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->ClearInput( pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::Close(long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::Close().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->Close( pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::DirectIO(long Command, long* pData, BSTR* pString, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::DirectIO().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->DirectIO( Command, pData, pString, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::ReleaseDevice(long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::ReleaseDevice().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->ReleaseDevice( pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::ResetStatistics(BSTR StatisticsBuffer, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::ResetStatistics().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->ResetStatistics( StatisticsBuffer, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::RetrieveStatistics(BSTR* StatisticsBuffer, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::RetrieveStatistics().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->RetrieveStatistics( StatisticsBuffer, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::UpdateStatistics(BSTR StatisticsBuffer, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::UpdateStatistics().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->UpdateStatistics( StatisticsBuffer, pRC );
	else
		return S_FALSE;
}


STDMETHODIMP CSoLpu230::UpdateKey(BSTR Key, BSTR KeyName, long* pRC)
{
	ATLTRACE(_T(" @CSoLpu230::UpdateKey().\n") );
	if( m_pSoMsr )
		return m_pSoMsr->UpdateKey( Key, KeyName, pRC );
	else
		return S_FALSE;
}
