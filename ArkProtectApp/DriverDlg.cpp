// DriverDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ArkProtectApp.h"
#include "DriverDlg.h"
#include "afxdialogex.h"

#include "ArkProtectAppDlg.h"

// CDriverDlg �Ի���

IMPLEMENT_DYNAMIC(CDriverDlg, CDialogEx)

CDriverDlg::CDriverDlg(CWnd* pParent /*=NULL*/, ArkProtect::CGlobal *GlobalObject)
	: CDialogEx(IDD_DRIVER_DIALOG, pParent)
	, m_Global(GlobalObject)
{
	// ����Ի���ָ��
	m_Global->m_DriverDlg = this;
}

CDriverDlg::~CDriverDlg()
{
}

void CDriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DRIVER_LIST, m_DriverListCtrl);
}


BEGIN_MESSAGE_MAP(CDriverDlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DRIVER_LIST, &CDriverDlg::OnNMCustomdrawDriverList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DRIVER_LIST, &CDriverDlg::OnLvnItemchangedDriverList)
	ON_NOTIFY(NM_RCLICK, IDC_DRIVER_LIST, &CDriverDlg::OnNMRClickDriverList)
	ON_COMMAND(ID_DRIVER_FRESHEN, &CDriverDlg::OnDriverFreshen)
	ON_COMMAND(ID_DRIVER_DELETE, &CDriverDlg::OnDriverDelete)
	ON_COMMAND(ID_DRIVER_UNLOAD, &CDriverDlg::OnDriverUnload)
END_MESSAGE_MAP()


// CDriverDlg ��Ϣ�������


BOOL CDriverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
		
	// ��ʼ�������б�
	APInitializeDriverList();

	// ����Iconͼ���б�
	UINT nIconSize = 20 * (UINT)(m_Global->iDpix / 96.0);
	m_DriverIconList.Create(nIconSize, nIconSize, ILC_COLOR32 | ILC_MASK, 2, 2);
	ListView_SetImageList(m_DriverListCtrl.m_hWnd, m_DriverIconList.GetSafeHandle(), LVSIL_SMALL);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDriverDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	m_Global->iResizeX = cx;
	m_Global->iResizeY = cy;
}


void CDriverDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	if (bShow == TRUE)
	{
		m_DriverListCtrl.MoveWindow(0, 0, m_Global->iResizeX, m_Global->iResizeY);

		// ���¸�������Ϣ CurrentChildDlg �� ���õ�ǰ�Ӵ��ڵ�button
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_CurrentChildDlg = ArkProtect::cd_DriverDialog;
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_DriverrButton.EnableWindow(FALSE);

		// ���ؽ�����Ϣ�б�
		APLoadDriverList();

		m_DriverListCtrl.SetFocus();
	}
}


void CDriverDlg::OnNMCustomdrawDriverList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������



	*pResult = 0;
}


void CDriverDlg::OnLvnItemchangedDriverList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������



	*pResult = 0;
}


void CDriverDlg::OnNMRClickDriverList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu Menu;
	Menu.LoadMenuW(IDR_DRIVER_MENU);
	CMenu* SubMenu = Menu.GetSubMenu(0);	// �Ӳ˵�

	CPoint Pt;
	GetCursorPos(&Pt);         // �õ����λ��

	int	iCount = SubMenu->GetMenuItemCount();

	// ���û��ѡ��,����ˢ�� ����ȫ��Disable
	if (m_DriverListCtrl.GetSelectedCount() == 0)
	{
		for (int i = 0; i < iCount; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
		}

		SubMenu->EnableMenuItem(ID_DRIVER_FRESHEN, MF_BYCOMMAND | MF_ENABLED);
	}

	int iIndex = m_DriverListCtrl.GetSelectionMark();
	if (iIndex >= 0)
	{
		if (_wcsicmp(L"-", m_DriverListCtrl.GetItemText(iIndex, ArkProtect::dc_Object)) == 0)
		{
			SubMenu->EnableMenuItem(ID_DRIVER_UNLOAD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);  // û��Object, UnloadҲҪ���
		}
	}

	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Pt.x, Pt.y, this);

	*pResult = 0;
}



void CDriverDlg::OnDriverFreshen()
{
	// TODO: �ڴ���������������
	// ��������ģ����Ϣ�б�
	APLoadDriverList();

}


void CDriverDlg::OnDriverDelete()
{
	// TODO: �ڴ���������������
	// ɾ���ļ�

}


void CDriverDlg::OnDriverUnload()
{
	// TODO: �ڴ���������������

	if (MessageBox(L"ж������ģ���Σ�գ����п��ܵ�������\r\n����Ļ�Ҫж����", L"ArkProtect", MB_ICONWARNING | MB_OKCANCEL) == IDCANCEL)
	{
		return;
	}

	if (m_Global->m_bIsRequestNow == TRUE)
	{
		return;
	}

	// ���ؽ�����Ϣ�б�
	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ArkProtect::CDriverCore::UnloadDriverInfoCallback, &m_DriverListCtrl, 0, NULL)
	);

}



/************************************************************************
*  Name : APInitializeDriverList
*  Param: void
*  Ret  : void
*  ��ʼ��ListControl
************************************************************************/
void CDriverDlg::APInitializeDriverList()
{
	m_Global->DriverCore().InitializeDriverList(&m_DriverListCtrl);
}



/************************************************************************
*  Name : APLoadDriverList
*  Param: void
*  Ret  : void
*  ���ؽ�����Ϣ��ListControl
************************************************************************/
void CDriverDlg::APLoadDriverList()
{
	if (m_Global->m_bIsRequestNow == TRUE)
	{
		return;
	}

	while (m_DriverIconList.Remove(0));

	m_DriverListCtrl.DeleteAllItems();

	m_DriverListCtrl.SetSelectedColumn(-1);

	// ���ؽ�����Ϣ�б�
	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ArkProtect::CDriverCore::QueryDriverInfoCallback, &m_DriverListCtrl, 0, NULL)
	);
}









