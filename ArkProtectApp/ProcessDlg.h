#pragma once
#include "afxcmn.h"
#include "Global.hpp"
#include "ProcessCore.h"


// CProcessDlg �Ի���

class CProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessDlg)

public:
	CProcessDlg(CWnd* pParent = NULL, ArkProtect::CGlobal *GlobalObject = NULL);   // ��׼���캯��
	virtual ~CProcessDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	

	DECLARE_MESSAGE_MAP()
public:
	
	void APInitializeProcessList();


	CImageList m_ProcessIconList;   // ����ͼ��
	CListCtrl  m_ProcessListCtrl;   // ListControl

	ArkProtect::CGlobal      *m_Global;
	ArkProtect::CProcessCore m_Process;

	
};
