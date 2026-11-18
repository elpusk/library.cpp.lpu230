
// lpu230_oposDlg.h : header file
//

#pragma once

#include <string>


// Clpu230oposDlg dialog
class Clpu230oposDlg : public CDialogEx
{
private:
	enum {
		CONST_MAX_DEVICE_NAME_LENGTH = 30
	};
// Construction
public:
	Clpu230oposDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LPU230_OPOS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonApplyPhysicalMsr();
	afx_msg void OnBnClickedButtonApplyPhysicalKeylock();
	CEdit m_edit_msr_cur_physical_device_name;
	CEdit m_edit_msr_new_physical_device_name;
	CEdit m_edit_keylock_cur_physical_device_name;
	CEdit m_edit_keylock_new_physical_device_name;

private:
	bool _do_anything_before_close();
	bool _apply_msr_physical_device_name_change();
	bool _apply_keylock_physical_device_name_change();

	bool _apply_msr_logical_device_name_change();
	bool _apply_keylock_logical_device_name_change();

	static bool _is_valied_device_name(const std::wstring& s_device_name);
	static bool _on_enchange_edit_devicename(CEdit &edit, CButton*p_bt);
	static std::wstring Clpu230oposDlg::_get_version_from_resource_table();

private:
	CButton* m_p_btn_apply_physical_msr;
	CButton* m_p_btn_apply_physical_keylock;
	CButton* m_p_btn_apply_logical_msr;
	CButton* m_p_btn_apply_logical_keylock;

	bool m_b_msr_physical_device_name_changed;
	bool m_b_keylock_physical_device_name_changed;
	bool m_b_msr_logical_device_name_changed;
	bool m_b_keylock_logical_device_name_changed;

	bool m_b_installed_so_msr;
	bool m_b_installed_so_keylock;

public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEditPhysicalDeviceNameNewMsr();
	afx_msg void OnEnChangeEditPhysicalDeviceNameNewKeylock();
	afx_msg void OnBnClickedButtonApplyLogicalMsr();
	afx_msg void OnBnClickedButtonApplyLogicalKeylock();
	CEdit m_edit_msr_cur_logical_device_name;
	CEdit m_edit_msr_new_logical_device_name;
	CEdit m_edit_keylock_cur_logical_device_name;
	CEdit m_edit_keylock_new_logical_device_name;
	afx_msg void OnEnChangeEditLogicalDeviceNameCurKeylock();
	afx_msg void OnEnChangeEditLogicalDeviceNameCurMsr();
	afx_msg void OnEnChangeEditLogicalDeviceNameNewKeylock();
	afx_msg void OnEnChangeEditLogicalDeviceNameNewMsr();
};
