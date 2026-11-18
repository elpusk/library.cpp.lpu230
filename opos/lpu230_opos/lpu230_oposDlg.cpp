
// lpu230_oposDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "lpu230_opos.h"
#include "lpu230_oposDlg.h"
#include "afxdialogex.h"

#include <algorithm>
#include <el_opos/el_opos.h>
#include <RegControl.h>

#pragma comment(lib,"Version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WIDE_HELPER(x) L##x
#define TO_WIDE(x) WIDE_HELPER(x)


// Clpu230oposDlg dialog

std::wstring Clpu230oposDlg::_get_version_from_resource_table()
{
	std::wstring s_ver;
	bool b_result(false);
	HMODULE h_module = ::GetModuleHandle(nullptr);
	do {
		if (!h_module) {
			continue;
		}
		HRSRC hResource = FindResource(h_module, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if (!hResource) {
			continue;
		}
		HGLOBAL hResourceData = LoadResource(h_module, hResource);
		if (!hResourceData) {
			continue;
		}
		LPVOID lpResource = LockResource(hResourceData);
		if (!lpResource) {
			continue;
		}

		VS_FIXEDFILEINFO* fileInfo(NULL);
		unsigned int n_fileInfo(0);
		if (!VerQueryValue(lpResource, L"\\", (LPVOID*)&fileInfo, &n_fileInfo)) {
			FreeResource(lpResource);
			continue;
		}
		unsigned short w_maj = HIWORD(fileInfo->dwFileVersionMS);
		unsigned short w_min = LOWORD(fileInfo->dwFileVersionMS);
		unsigned short w_fix = HIWORD(fileInfo->dwFileVersionLS);
		unsigned short w_build = LOWORD(fileInfo->dwFileVersionLS);

		s_ver = std::to_wstring((unsigned int)w_maj) + L'.'
			+ std::to_wstring((unsigned int)w_min) + L'.'
			+ std::to_wstring((unsigned int)w_fix) + L'.'
			+ std::to_wstring((unsigned int)w_build);

		FreeResource(lpResource);

	} while (false);

	return s_ver;
}

Clpu230oposDlg::Clpu230oposDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LPU230_OPOS_DIALOG, pParent)
	, m_b_msr_physical_device_name_changed(false)
	, m_b_keylock_physical_device_name_changed(false)
	, m_b_installed_so_msr(false)
	, m_b_installed_so_keylock(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Clpu230oposDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME_CUR_MSR, m_edit_msr_cur_physical_device_name);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME_NEW_MSR, m_edit_msr_new_physical_device_name);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME_CUR_KEYLOCK, m_edit_keylock_cur_physical_device_name);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME_NEW_KEYLOCK, m_edit_keylock_new_physical_device_name);
	DDX_Control(pDX, IDC_EDIT_LOGICAL_DEVICE_NAME_CUR_MSR, m_edit_msr_cur_logical_device_name);
	DDX_Control(pDX, IDC_EDIT_LOGICAL_DEVICE_NAME_NEW_MSR, m_edit_msr_new_logical_device_name);
	DDX_Control(pDX, IDC_EDIT_LOGICAL_DEVICE_NAME_CUR_KEYLOCK, m_edit_keylock_cur_logical_device_name);
	DDX_Control(pDX, IDC_EDIT_LOGICAL_DEVICE_NAME_NEW_KEYLOCK, m_edit_keylock_new_logical_device_name);
}

BEGIN_MESSAGE_MAP(Clpu230oposDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &Clpu230oposDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_MSR, &Clpu230oposDlg::OnBnClickedButtonApplyPhysicalMsr)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_KEYLOCK, &Clpu230oposDlg::OnBnClickedButtonApplyPhysicalKeylock)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_DEVICE_NAME_NEW_MSR, &Clpu230oposDlg::OnEnChangeEditPhysicalDeviceNameNewMsr)
	ON_EN_CHANGE(IDC_EDIT_DEVICE_NAME_NEW_KEYLOCK, &Clpu230oposDlg::OnEnChangeEditPhysicalDeviceNameNewKeylock)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_LOGICAL_MSR, &Clpu230oposDlg::OnBnClickedButtonApplyLogicalMsr)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_LOGICAL_KEYLOCK, &Clpu230oposDlg::OnBnClickedButtonApplyLogicalKeylock)
	ON_EN_CHANGE(IDC_EDIT_LOGICAL_DEVICE_NAME_CUR_KEYLOCK, &Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameCurKeylock)
	ON_EN_CHANGE(IDC_EDIT_LOGICAL_DEVICE_NAME_CUR_MSR, &Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameCurMsr)
	ON_EN_CHANGE(IDC_EDIT_LOGICAL_DEVICE_NAME_NEW_KEYLOCK, &Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameNewKeylock)
	ON_EN_CHANGE(IDC_EDIT_LOGICAL_DEVICE_NAME_NEW_MSR, &Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameNewMsr)
END_MESSAGE_MAP()


// Clpu230oposDlg message handlers

BOOL Clpu230oposDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	std::wstring s_ver = Clpu230oposDlg::_get_version_from_resource_table();
	if (!s_ver.empty()) {
		CString sTitle;
		GetWindowText(sTitle);

		sTitle += L" - V";
		sTitle += s_ver.c_str();
		SetWindowText(sTitle);
	}

	m_p_btn_apply_physical_msr = (CButton*)GetDlgItem(IDC_BUTTON_APPLY_MSR);
	m_p_btn_apply_physical_keylock = (CButton*)GetDlgItem(IDC_BUTTON_APPLY_KEYLOCK);

	m_p_btn_apply_logical_msr = (CButton*)GetDlgItem(IDC_BUTTON_APPLY_LOGICAL_MSR);
	m_p_btn_apply_logical_keylock = (CButton*)GetDlgItem(IDC_BUTTON_APPLY_LOGICAL_KEYLOCK);

	ASSERT(m_p_btn_apply_physical_msr != nullptr);
	ASSERT(m_p_btn_apply_physical_keylock != nullptr);
	ASSERT(m_p_btn_apply_logical_msr != nullptr);
	ASSERT(m_p_btn_apply_logical_keylock != nullptr);

	m_p_btn_apply_physical_msr->EnableWindow(FALSE);
	m_p_btn_apply_physical_keylock->EnableWindow(FALSE);
	m_p_btn_apply_logical_msr->EnableWindow(FALSE);
	m_p_btn_apply_logical_keylock->EnableWindow(FALSE);
	//
	do {
		// PhysicalDeviceName is mandotory
		std::list<std::wstring> list_msr = CRegControl::GetOposPhysicalDeviceName(
			std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
		);
		std::list<std::wstring> list_keylock = CRegControl::GetOposPhysicalDeviceName(
			std::wstring(TO_WIDE(OPOS_CLASSKEY_LOCK))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_KEYLOCK))
		);
		if (list_msr.empty() && list_keylock.empty()) {
			AfxMessageBox(_T("Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
			PostMessage(WM_CLOSE);//terminate program
			continue;
		}

		if (list_msr.empty()) {
			//ONLY list_keylock installed
			if (list_keylock.size() != 1) {
				AfxMessageBox(_T("(ONLY one KEYLOCK physical devie name)Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
				PostMessage(WM_CLOSE);//terminate program
				continue;
			}

			m_b_installed_so_keylock = true;
		}
		else {
			if (list_keylock.empty()) {
				//ONLY msr installed
				if (list_msr.size() != 1) {
					AfxMessageBox(_T("(ONLY one MSR physical devie name)Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
					PostMessage(WM_CLOSE);//terminate program
					continue;
				}
				m_b_installed_so_msr = true;
			}
			else {//msr AND keylock installed
				if (list_keylock.size() != 1) {
					AfxMessageBox(_T("(ONLY one KEYLOCK physical devie name)Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
					PostMessage(WM_CLOSE);//terminate program
					continue;
				}
				if (list_msr.size() != 1) {
					AfxMessageBox(_T("(ONLY one MSR physical devie name)Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
					PostMessage(WM_CLOSE);//terminate program
					continue;
				}
				m_b_installed_so_keylock = true;
				m_b_installed_so_msr = true;
			}
		}


		if (m_b_installed_so_msr) {
			std::wstring sRegMsrCurDevName = *list_msr.begin();
			if (sRegMsrCurDevName.empty())
			{
				AfxMessageBox(_T("Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
				PostMessage(WM_CLOSE);//terminate program
				continue;
			}
			m_edit_msr_cur_physical_device_name.SetWindowText(sRegMsrCurDevName.c_str());
		}
		else {
			m_edit_msr_cur_physical_device_name.SetWindowText(L"NoneServiceObject");
			m_edit_msr_new_physical_device_name.EnableWindow(FALSE);
		}
		
		if (m_b_installed_so_keylock) {
			std::wstring sRegKeylockCurDevName = *list_keylock.begin();
			if (sRegKeylockCurDevName.empty())
			{
				AfxMessageBox(_T("Failed to get OPOS device name from registry. Please check if the device is installed and registered correctly."));
				PostMessage(WM_CLOSE);//terminate program
				continue;
			}
			m_edit_keylock_cur_physical_device_name.SetWindowText(sRegKeylockCurDevName.c_str());
		}
		else {
			m_edit_keylock_cur_physical_device_name.SetWindowText(L"NoneServiceObject");
			m_edit_keylock_new_physical_device_name.EnableWindow(FALSE);
		}

		/////////////////////////////////////////////////////////////////////////
		//logical is option.
		// 하나의 logical device 만 지원
		m_edit_msr_new_logical_device_name.EnableWindow(FALSE);
		if (m_b_installed_so_msr) {
			std::list<std::wstring> list_logical_msr = CRegControl::GetOposLogicalDeviceName(
				std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
				, std::wstring(TO_WIDE(EL_APPUID_KEY))
				, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
			);
			if (!list_logical_msr.empty()) {
				if (!list_logical_msr.begin()->empty()) {
					m_edit_msr_cur_logical_device_name.SetWindowText(list_logical_msr.begin()->c_str());
					m_edit_msr_new_logical_device_name.EnableWindow(TRUE);
				}
			}
		}
		else {
			m_edit_msr_cur_logical_device_name.SetWindowText(L"NoneServiceObject");
		}

		// 하나의 logical device 만 지원
		m_edit_keylock_new_logical_device_name.EnableWindow(FALSE);
		if (m_b_installed_so_keylock) {
			std::list<std::wstring> list_logical_keylock = CRegControl::GetOposLogicalDeviceName(
				std::wstring(TO_WIDE(OPOS_CLASSKEY_LOCK))
				, std::wstring(TO_WIDE(EL_APPUID_KEY))
				, std::wstring(TO_WIDE(EL_APPUID_SO_KEYLOCK))
			);
			if (!list_logical_keylock.empty()) {
				if (!list_logical_keylock.begin()->empty()) {
					m_edit_keylock_cur_logical_device_name.SetWindowTextW(list_logical_keylock.begin()->c_str());
					m_edit_keylock_new_logical_device_name.EnableWindow(TRUE);
				}
			}
		}
		else {
			m_edit_keylock_cur_logical_device_name.SetWindowText(L"NoneServiceObject");
		}

	} while (false);

	//

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Clpu230oposDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		// don;t show about dialog
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Clpu230oposDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Clpu230oposDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Clpu230oposDlg::OnBnClickedButtonApplyPhysicalMsr()
{
	// TODO: Add your control notification handler code here
	if(_apply_msr_physical_device_name_change()) {
		AfxMessageBox(_T("MSR device name change applied successfully."), MB_ICONINFORMATION);
		CString sName;
		m_edit_msr_new_physical_device_name.GetWindowText(sName);
		m_edit_msr_cur_physical_device_name.SetWindowText(sName);
		m_edit_msr_new_physical_device_name.SetWindowText(L"");
	}
	else {
		AfxMessageBox(_T("Failed to apply MSR device name change."));
	}
}

void Clpu230oposDlg::OnBnClickedButtonApplyPhysicalKeylock()
{
	// TODO: Add your control notification handler code here
	if(_apply_keylock_physical_device_name_change()) {
		AfxMessageBox(_T("Keylock device name change applied successfully."), MB_ICONINFORMATION);
		CString sName;
		m_edit_keylock_new_physical_device_name.GetWindowText(sName);
		m_edit_keylock_cur_physical_device_name.SetWindowText(sName);
		m_edit_keylock_new_physical_device_name.SetWindowText(L"");
	}
	else {
		AfxMessageBox(_T("Failed to apply Keylock device name change."));
	}
}

void Clpu230oposDlg::OnBnClickedButtonApplyLogicalMsr()
{
	if (_apply_msr_logical_device_name_change()) {
		AfxMessageBox(_T("MSR logical device name change applied successfully."), MB_ICONINFORMATION);
		CString sName;
		m_edit_msr_new_logical_device_name.GetWindowText(sName);
		m_edit_msr_cur_logical_device_name.SetWindowText(sName);
		m_edit_msr_new_logical_device_name.SetWindowText(L"");
	}
	else {
		AfxMessageBox(_T("Failed to apply MSR logical device name change."));
	}
}

void Clpu230oposDlg::OnBnClickedButtonApplyLogicalKeylock()
{
	if (_apply_keylock_logical_device_name_change()) {
		AfxMessageBox(_T("Keylock logical device name change applied successfully."), MB_ICONINFORMATION);
		CString sName;
		m_edit_keylock_new_logical_device_name.GetWindowText(sName);
		m_edit_keylock_cur_logical_device_name.SetWindowText(sName);
		m_edit_keylock_new_logical_device_name.SetWindowText(L"");
	}
	else {
		AfxMessageBox(_T("Failed to apply Keylock logical device name change."));
	}
}

void Clpu230oposDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
}

bool Clpu230oposDlg::_apply_msr_physical_device_name_change()
{
	bool b_result(false);

	do {
		if (!m_b_installed_so_msr) {
			continue;
		}
		if (!m_b_msr_physical_device_name_changed) {
			continue;
		}

		CString sNewDevName;
		m_edit_msr_new_physical_device_name.GetWindowText(sNewDevName);
		if (sNewDevName.IsEmpty()) {
			continue;
		}
		CString sCurDevName;
		m_edit_msr_cur_physical_device_name.GetWindowText(sCurDevName);
		if (sCurDevName.IsEmpty()) {
			continue;
		}

		std::wstring s_new_dev_name(sNewDevName.GetString());
		std::wstring s_cur_dev_name(sCurDevName.GetString());
		if( !CRegControl::SetOposPhysicalDeviceName(
			s_cur_dev_name
			, s_new_dev_name
			, std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
		)) {
			continue;
		}

		b_result = true;
	} while (false);
	return b_result;
}
bool Clpu230oposDlg::_apply_keylock_physical_device_name_change()
{
	bool b_result(false);

	do {
		if (!m_b_installed_so_keylock) {
			continue;
		}
		if (!m_b_keylock_physical_device_name_changed) {
			continue;
		}
		CString sNewDevName;
		m_edit_keylock_new_physical_device_name.GetWindowText(sNewDevName);
		if (sNewDevName.IsEmpty()) {
			continue;
		}
		CString sCurDevName;
		m_edit_keylock_cur_physical_device_name.GetWindowText(sCurDevName);
		if (sCurDevName.IsEmpty()) {
			continue;
		}

		std::wstring s_new_dev_name(sNewDevName.GetString());
		std::wstring s_cur_dev_name(sCurDevName.GetString());
		if (!CRegControl::SetOposPhysicalDeviceName(
			s_cur_dev_name
			, s_new_dev_name
			, std::wstring(TO_WIDE(OPOS_CLASSKEY_LOCK))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_KEYLOCK))
		)) {
			continue;
		}
		b_result = true;

	} while (false);
	return b_result;
}

bool Clpu230oposDlg::_apply_msr_logical_device_name_change()
{
	bool b_result(false);

	do {
		if (!m_b_installed_so_msr) {
			continue;
		}
		if (!m_b_msr_logical_device_name_changed) {
			continue;
		}

		CString sNewDevName;
		m_edit_msr_new_logical_device_name.GetWindowText(sNewDevName);
		if (sNewDevName.IsEmpty()) {
			continue;
		}
		CString sCurDevName;
		m_edit_msr_cur_logical_device_name.GetWindowText(sCurDevName);
		if (sCurDevName.IsEmpty()) {
			continue;
		}

		std::wstring s_new_dev_name(sNewDevName.GetString());
		std::wstring s_cur_dev_name(sCurDevName.GetString());
		if (!CRegControl::SetOposLogicalDeviceName(
			s_cur_dev_name
			, s_new_dev_name
			, std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
		)) {
			continue;
		}

		b_result = true;
	} while (false);
	return b_result;
}
bool Clpu230oposDlg::_apply_keylock_logical_device_name_change()
{
	bool b_result(false);

	do {
		if (!m_b_installed_so_keylock) {
			continue;
		}
		if (!m_b_keylock_logical_device_name_changed) {
			continue;
		}
		CString sNewDevName;
		m_edit_keylock_new_logical_device_name.GetWindowText(sNewDevName);
		if (sNewDevName.IsEmpty()) {
			continue;
		}
		CString sCurDevName;
		m_edit_keylock_cur_logical_device_name.GetWindowText(sCurDevName);
		if (sCurDevName.IsEmpty()) {
			continue;
		}

		std::wstring s_new_dev_name(sNewDevName.GetString());
		std::wstring s_cur_dev_name(sCurDevName.GetString());
		if (!CRegControl::SetOposLogicalDeviceName(
			s_cur_dev_name
			, s_new_dev_name
			, std::wstring(TO_WIDE(OPOS_CLASSKEY_LOCK))
			, std::wstring(TO_WIDE(EL_APPUID_KEY))
			, std::wstring(TO_WIDE(EL_APPUID_SO_KEYLOCK))
		)) {
			continue;
		}
		b_result = true;

	} while (false);
	return b_result;
}

/**
* @description: This function is called before the dialog is closed.
*   It can be used to perform any necessary cleanup or save state before the application exits.
* @return: true if the function executed successfully or none processing, false otherwise.
*/
bool Clpu230oposDlg::_do_anything_before_close()
{
	bool b_result(false);

	do {
		if (
			!m_b_msr_physical_device_name_changed && 
			!m_b_keylock_physical_device_name_changed && 
			!m_b_msr_logical_device_name_changed &&
			!m_b_keylock_logical_device_name_changed
			) {
			b_result = true; //none processing
			continue;
		}

		if(
			m_b_msr_physical_device_name_changed || 
			m_b_keylock_physical_device_name_changed ||
			m_b_msr_logical_device_name_changed ||
			m_b_keylock_logical_device_name_changed
			) {
			// 사용자에게	변경 사항을 저장할지 묻는 메시지 박스를 표시합니다.
			int nResponse = AfxMessageBox(_T("Do you want to save the changes?"), MB_YESNOCANCEL | MB_ICONQUESTION);
			switch (nResponse)
			{
			case IDYES:
				break;
			case IDCANCEL:
				// 사용자가 "Cancel"을 선택한 경우, 다이얼로그를 닫지 않고 그대로 유지합니다.
				b_result = false;
				continue;
			case IDNO:
				// 사용자가 "No"를 선택한 경우, 변경 사항을 저장하지 않고 다이얼로그를 닫습니다.
				b_result = true;
				continue;
			default:
				b_result = true; // 기본적으로 다이얼로그를 닫도록 설정합니다.
				continue;
			}//end switch

			// 사용자가 "Yes"를 선택한 경우, 변경 사항을 저장하는 코드를 여기에 작성합니다.
			// 예: SaveChanges();
		}

		b_result = true;

		_apply_msr_physical_device_name_change();
		_apply_keylock_physical_device_name_change();

		_apply_msr_logical_device_name_change();
		_apply_keylock_logical_device_name_change();

	} while (false);
	return b_result;
}
void Clpu230oposDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if( _do_anything_before_close() ){
		CDialogEx::OnOK();
	}
}

void Clpu230oposDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (_do_anything_before_close()) {
		CDialogEx::OnClose();
	}
}

BOOL Clpu230oposDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

bool Clpu230oposDlg::_is_valied_device_name(const std::wstring& s_device_name)
{
	if(s_device_name.empty()) {
		return false;
	}
	if(s_device_name.size() > CONST_MAX_DEVICE_NAME_LENGTH) {
		return false;
	}
	// Check if the device name contains only printable ASCII characters and does not contain backslashes
	// ASCII 기준 Printable character 범위: 0x20(공백) ~ 0x7E(~)
	// 공백을 허용하지 않으려면 0x20 대신 0x21('!')을 사용
	bool b_result = std::all_of(s_device_name.cbegin(), s_device_name.cend(), [](std::wstring::value_type ch) {
		return (ch >= 0x21 && ch <= 0x7E) && (ch != L'\\');
		});
	return b_result;
}

bool Clpu230oposDlg::_on_enchange_edit_devicename(CEdit& edit, CButton* p_bt)
{
	bool b_changed(false);

	do {
		if(!p_bt) {
			continue;
		}
		CString sNewDevName;
		edit.GetWindowText(sNewDevName);

		if (sNewDevName.IsEmpty()) {
			p_bt->EnableWindow(FALSE);
			continue;
		}

		std::wstring s_new_dev_name(sNewDevName.GetString());

		if (!Clpu230oposDlg::_is_valied_device_name(s_new_dev_name)) {
			p_bt->EnableWindow(FALSE);
			continue;
		}

		p_bt->EnableWindow(TRUE);
		b_changed = true;

	} while (false);
	return b_changed;
}

void Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameCurKeylock()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameCurMsr()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void Clpu230oposDlg::OnEnChangeEditPhysicalDeviceNameNewMsr()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_b_msr_physical_device_name_changed = Clpu230oposDlg::_on_enchange_edit_devicename(m_edit_msr_new_physical_device_name, m_p_btn_apply_physical_msr);
}

void Clpu230oposDlg::OnEnChangeEditPhysicalDeviceNameNewKeylock()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_b_keylock_physical_device_name_changed = Clpu230oposDlg::_on_enchange_edit_devicename(m_edit_keylock_new_physical_device_name, m_p_btn_apply_physical_keylock);
}

void Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameNewKeylock()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_b_keylock_logical_device_name_changed = Clpu230oposDlg::_on_enchange_edit_devicename(m_edit_keylock_new_logical_device_name, m_p_btn_apply_logical_keylock);
}

void Clpu230oposDlg::OnEnChangeEditLogicalDeviceNameNewMsr()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_b_msr_logical_device_name_changed = Clpu230oposDlg::_on_enchange_edit_devicename(m_edit_msr_new_logical_device_name, m_p_btn_apply_logical_msr);
}
