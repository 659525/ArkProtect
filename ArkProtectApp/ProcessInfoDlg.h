#pragma once
#include "afxcmn.h"
#include "Define.h"
#include "ProcessCore.h"
#include "ProcessModule.h"
#include "ProcessThread.h"

// CProcessInfoDlg �Ի���

class CProcessInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessInfoDlg)

public:
	CProcessInfoDlg(CWnd* pParent, ArkProtect::eProcessInfoKind ProcessInfoKind,
		ArkProtect::CGlobal *GlobalObject, ArkProtect::PPROCESS_ENTRY_INFORMATION ProcessEntry);   // ��׼���캯��
	virtual ~CProcessInfoDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS_INFO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	

	DECLARE_MESSAGE_MAP()
public:
	
	void APInitializeProcessInfoList();

	void APInitializeProcessModuleList();

	void APLoadProcessModuleList();

	void APInitializeProcessThreadList();

	void APLoadProcessThreadList();



	CListCtrl m_ProcessInfoListCtrl;
	HICON     m_hIcon;

	UINT32    m_ProcessId = 0;

	ArkProtect::CGlobal                     *m_Global;
	ArkProtect::eProcessInfoKind            m_WantedInfoKind;
	ArkProtect::eProcessInfoKind            m_CurrentInfoKind = (ArkProtect::eProcessInfoKind)(-1);
	ArkProtect::PPROCESS_ENTRY_INFORMATION  m_ProcessEntry;             // Process�ṹ��
	ArkProtect::CProcessModule              m_ProcessModule;            // ����ģ�������
	ArkProtect::CProcessThread              m_ProcessThread;            // �����߳������ 
};
