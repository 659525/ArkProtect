#pragma once

#include "Global.hpp"
#include "afxcmn.h"
#include "afxwin.h"

// CHookDlg �Ի���

class CHookDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHookDlg)

public:
	CHookDlg(CWnd* pParent = NULL, ArkProtect::CGlobal *GlobalObject = NULL);   // ��׼���캯��
	virtual ~CHookDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HOOK_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLbnSelchangeHookListbox();

	DECLARE_MESSAGE_MAP()
public:
	
	void APInitializeHookItemList();

	CListCtrl m_HookListCtrl;
	CListBox m_HookListBox;
	CListBox m_HookModuleListBox;

	ArkProtect::CGlobal  *m_Global;
	
	int                  m_iCurSel = 65535;

};
