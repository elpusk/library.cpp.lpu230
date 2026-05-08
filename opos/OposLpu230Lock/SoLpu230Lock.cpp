// SoLpu230Lock.cpp : CSoLpu230Lock의 구현입니다.

#include "stdafx.h"
#include "SoLpu230Lock.h"


// CSoLpu230Lock

STDMETHODIMP CSoLpu230Lock::OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::OpenService().\n") );
	return m_SoLock.OpenService( DeviceClass, DeviceName, pDispatch, pRC );
}


STDMETHODIMP CSoLpu230Lock::GetPropertyNumber(LONG PropIndex, LONG* pNumber)
{
	ATLTRACE(_T(" @CSoLpu230Lock::GetPropertyNumber().\n") );
	return m_SoLock.GetPropertyNumber( PropIndex, pNumber );
}


STDMETHODIMP CSoLpu230Lock::SetPropertyNumber(LONG PropIndex, LONG Number)
{
	ATLTRACE(_T(" @CSoLpu230Lock::SetPropertyNumber().\n") );
	return m_SoLock.SetPropertyNumber( PropIndex, Number );
}


STDMETHODIMP CSoLpu230Lock::GetPropertyString(LONG PropIndex, BSTR* pString)
{
	ATLTRACE(_T(" @CSoLpu230Lock::GetPropertyString().\n") );
	return m_SoLock.GetPropertyString( PropIndex, pString );
}


STDMETHODIMP CSoLpu230Lock::SetPropertyString(LONG PropIndex, BSTR bstrString)
{
	ATLTRACE(_T(" @CSoLpu230Lock::SetPropertyString().\n") );
	return m_SoLock.SetPropertyString( PropIndex, bstrString );
}


STDMETHODIMP CSoLpu230Lock::COFreezeEvents(VARIANT_BOOL Freeze, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::COFreezeEvents().\n") );
	return m_SoLock.COFreezeEvents( Freeze, pRC );
}


STDMETHODIMP CSoLpu230Lock::CheckHealth(LONG Level, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::CheckHealth().\n") );
	return m_SoLock.CheckHealth( Level, pRC );
}


STDMETHODIMP CSoLpu230Lock::ClaimDevice(LONG lTimeout, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::ClaimDevice().\n") );
	return m_SoLock.ClaimDevice( lTimeout, pRC );
}


STDMETHODIMP CSoLpu230Lock::Close(LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::Close().\n") );
	return m_SoLock.Close( pRC );
}


STDMETHODIMP CSoLpu230Lock::DirectIO(LONG Command, LONG* pData, BSTR* pString, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::DirectIO().\n") );
	return m_SoLock.DirectIO( Command, pData, pString, pRC );
}


STDMETHODIMP CSoLpu230Lock::ReleaseDevice(LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::ReleaseDevice().\n") );
	return m_SoLock.ReleaseDevice( pRC );
}


STDMETHODIMP CSoLpu230Lock::ResetStatistics(BSTR StatisticsBuffer, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::ResetStatistics().\n") );
	return m_SoLock.ResetStatistics( StatisticsBuffer, pRC );
}


STDMETHODIMP CSoLpu230Lock::RetrieveStatistics(BSTR* StatisticsBuffer, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::RetrieveStatistics().\n") );
	return m_SoLock.RetrieveStatistics( StatisticsBuffer, pRC );
}


STDMETHODIMP CSoLpu230Lock::UpdateStatistics(BSTR StatisticsBuffer, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::UpdateStatistics().\n") );
	return m_SoLock.UpdateStatistics( StatisticsBuffer, pRC );
}

STDMETHODIMP CSoLpu230Lock::WaitForKeylockChange(LONG nKeyPosition, LONG nTimeout, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::WaitForKeylockChange().\n") );
	return m_SoLock.WaitForKeylockChange( nKeyPosition, nTimeout , pRC );
}


STDMETHODIMP CSoLpu230Lock::CompareFirmwareVersion(BSTR FirmwareFileName, LONG* pResult, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::CompareFirmwareVersion().\n") );
	return m_SoLock.CompareFirmwareVersion( FirmwareFileName, pResult, pRC );
}


STDMETHODIMP CSoLpu230Lock::UpdateFirmware(BSTR FirmwareFileName, LONG* pRC)
{
	ATLTRACE(_T(" @CSoLpu230Lock::UpdateFirmware().\n") );
	return m_SoLock.UpdateFirmware( FirmwareFileName, pRC );
}
