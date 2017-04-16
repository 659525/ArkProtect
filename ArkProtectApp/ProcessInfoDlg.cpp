// ProcessInfoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ArkProtectApp.h"
#include "ProcessInfoDlg.h"
#include "afxdialogex.h"


// CProcessInfoDlg �Ի���

IMPLEMENT_DYNAMIC(CProcessInfoDlg, CDialogEx)

CProcessInfoDlg::CProcessInfoDlg(CWnd* pParent, ArkProtect::eProcessInfoKind ProcessInfoKind,
	ArkProtect::CGlobal *GlobalObject, ArkProtect::PPROCESS_ENTRY_INFORMATION ProcessEntry)
	: CDialogEx(IDD_PROCESS_INFO_DIALOG, pParent)
	, m_WantedInfoKind(ProcessInfoKind)
	, m_Global(GlobalObject)
	, m_ProcessModule(GlobalObject, ProcessEntry)
	, m_ProcessEntry(ProcessEntry)
{
}

CProcessInfoDlg::~CProcessInfoDlg()
{
}

void CProcessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_INFO_LIST, m_ProcessInfoListCtrl);
}


BEGIN_MESSAGE_MAP(CProcessInfoDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CProcessInfoDlg ��Ϣ�������


BOOL CProcessInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	// ���öԻ����ͼ��
	SHFILEINFO shFileInfo = { 0 };
	SHGetFileInfo(m_ProcessEntry->wzFilePath, FILE_ATTRIBUTE_NORMAL,
		&shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	m_hIcon = shFileInfo.hIcon;

	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// ��ʼ��ListControl
	APInitializeProcessInfoList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CProcessInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
}




/************************************************************************
*  Name : APInitializeProcessList
*  Param: void
*  Ret  : void
*  ��ʼ��ListControl
************************************************************************/
void CProcessInfoDlg::APInitializeProcessInfoList()
{
	// �жϵ�ǰ��Ҫ�ĶԻ���
	if (m_WantedInfoKind == m_CurrentInfoKind)
	{
		return;
	}

	CString strWindowText = L"";

	switch (m_WantedInfoKind)
	{
	case ArkProtect::pik_Module:
		
		m_CurrentInfoKind = m_WantedInfoKind;

		strWindowText.Format(L"Process Module - %s", m_ProcessEntry->wzImageName);

		SetWindowText(strWindowText.GetBuffer());

		APInitializeProcessModuleList();

		APLoadProcessModuleList();

		break;
	case ArkProtect::pik_Thread:
		break;
	case ArkProtect::pik_Handle:
		break;
	case ArkProtect::pik_Window:
		break;
	case ArkProtect::pik_Memory:
		break;
	default:
		break;
	}



}


/************************************************************************
*  Name : APInitializeProcessList
*  Param: void
*  Ret  : void
*  ��ʼ��ListControl
************************************************************************/
void CProcessInfoDlg::APInitializeProcessModuleList()
{
	m_ProcessModule.InitializeProcessModuleList(&m_ProcessInfoListCtrl);
}


/************************************************************************
*  Name : APLoadProcessModuleList
*  Param: void
*  Ret  : void
*  ���ؽ�����Ϣ��ListControl
************************************************************************/
void CProcessInfoDlg::APLoadProcessModuleList()
{
	if (m_Global->m_bIsRequestNow == TRUE)
	{
		return;
	}

	m_ProcessInfoListCtrl.DeleteAllItems();

	m_ProcessInfoListCtrl.SetSelectedColumn(-1);

	// ���ؽ�����Ϣ�б�
	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ArkProtect::CProcessModule::QueryProcessModuleCallback, &m_ProcessInfoListCtrl, 0, NULL)
	);
}