#pragma once
#include "Global.hpp"
#include "afxcmn.h"

// CDriverDlg �Ի���

class CDriverDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDriverDlg)

public:
	CDriverDlg(CWnd* pParent = NULL, ArkProtect::CGlobal *GlobalObject = NULL);   // ��׼���캯��
	virtual ~CDriverDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DRIVER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	

	DECLARE_MESSAGE_MAP()
public:
	
	void APInitializeDriverList();

	void APLoadDriverList();


	CImageList m_DriverIconList;   // ����ͼ��
	CListCtrl  m_DriverListCtrl;

	ArkProtect::CGlobal      *m_Global;
	
	
};
