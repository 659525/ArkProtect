// HookDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ArkProtectApp.h"
#include "HookDlg.h"
#include "afxdialogex.h"

#include "ArkProtectAppDlg.h"

// CHookDlg �Ի���

IMPLEMENT_DYNAMIC(CHookDlg, CDialogEx)

CHookDlg::CHookDlg(CWnd* pParent /*=NULL*/, ArkProtect::CGlobal *GlobalObject)
	: CDialogEx(IDD_HOOK_DIALOG, pParent)
	, m_Global(GlobalObject)
{
	// ����Ի���ָ��
	m_Global->m_HookDlg = this;
}

CHookDlg::~CHookDlg()
{
}

void CHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOOK_LISTCTRL, m_HookListCtrl);
	DDX_Control(pDX, IDC_HOOK_LISTBOX, m_HookListBox);
	DDX_Control(pDX, IDC_HOOKMODULE_LISTBOX, m_HookModuleListBox);
}


BEGIN_MESSAGE_MAP(CHookDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_HOOK_LISTBOX, &CHookDlg::OnLbnSelchangeHookListbox)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_HOOK_LISTCTRL, &CHookDlg::OnNMCustomdrawHookListctrl)
	ON_COMMAND(ID_SSDT_FRESHEN, &CHookDlg::OnSsdtFreshen)
	ON_COMMAND(ID_SSDT_RESUME, &CHookDlg::OnSsdtResume)
	ON_NOTIFY(NM_RCLICK, IDC_HOOK_LISTCTRL, &CHookDlg::OnNMRClickHookListctrl)
END_MESSAGE_MAP()


// CHookDlg ��Ϣ�������


BOOL CHookDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	APInitializeHookItemList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CHookDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()

	CRect   Rect;
	GetClientRect(Rect);
	dc.FillSolidRect(Rect, RGB(255, 255, 255));

	CRect HookListBoxRect;
	CRect HookModuleListBoxRect;
	CRect HookListCtrlRect;

	m_HookListBox.GetWindowRect(&HookListBoxRect);
	ClientToScreen(&Rect);

	HookListBoxRect.left -= Rect.left;
	HookListBoxRect.right -= Rect.left;
	HookListBoxRect.top -= Rect.top;
	HookListBoxRect.bottom = Rect.Height() - 2;

	m_HookListBox.MoveWindow(HookListBoxRect);

	CPoint StartPoint;
	StartPoint.x = (LONG)(HookListBoxRect.right) + 2;
	StartPoint.y = -1;

	CPoint EndPoint;
	EndPoint.x = (LONG)(HookListBoxRect.right) + 2;
	EndPoint.y = Rect.Height() + 2;

	HookListCtrlRect.left = StartPoint.x + 1;
	HookListCtrlRect.right = Rect.Width();
	HookListCtrlRect.top = 0;
	HookListCtrlRect.bottom = Rect.Height();
	m_HookListCtrl.MoveWindow(HookListCtrlRect);

	COLORREF Color(RGB(190, 190, 190));

	CClientDC aDC(this);			//CClientDC�Ĺ��캯����Ҫһ�����������������ָ���ͼ���ڵ�ָ�룬������thisָ��Ϳ�����
	CPen pen(PS_SOLID, 1, Color);	//����һ����������󣬹���ʱ���û�������
	aDC.SelectObject(&pen);
	aDC.MoveTo(StartPoint);
	aDC.LineTo(EndPoint);

	if (m_HookModuleListBox.IsWindowVisible())
	{
		HookListBoxRect.bottom = Rect.Height() / 2;

		StartPoint.x = -1;
		StartPoint.y = (LONG)(HookListBoxRect.bottom) + 2;
		EndPoint.x = (LONG)(HookListBoxRect.right) + 2;
		EndPoint.y = (LONG)(HookListBoxRect.bottom) + 2;

		m_HookModuleListBox.MoveWindow(
			HookListBoxRect.left,
			HookListBoxRect.bottom + 5,
			HookListBoxRect.Width(),
			Rect.Height() - HookListBoxRect.bottom - 7
		);

		CPen pen2(PS_SOLID, 1, Color);	//����һ����������󣬹���ʱ���û�������
		aDC.SelectObject(&pen2);
		aDC.MoveTo(StartPoint);
		aDC.LineTo(EndPoint);
	}

	m_HookListBox.MoveWindow(&HookListBoxRect);
}


void CHookDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	if (bShow == TRUE)
	{
		// ���¸�������Ϣ CurrentChildDlg �� ���õ�ǰ�Ӵ��ڵ�button
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_CurrentChildDlg = ArkProtect::cd_HookDialog;
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_HookButton.EnableWindow(FALSE);

		m_HookListBox.SetCurSel(ArkProtect::hi_Ssdt);

		OnLbnSelchangeHookListbox();

		m_HookListCtrl.SetFocus();

	}
	else
	{
		m_iCurSel = 65535;
	}
}



void CHookDlg::OnLbnSelchangeHookListbox()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	int iCurSel = m_HookListBox.GetCurSel();

	switch (iCurSel)
	{
	case ArkProtect::hi_Ssdt:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::hi_Ssdt)
		{
			m_HookListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		m_HookModuleListBox.ShowWindow(FALSE);

		// ��ʼ��ListCtrl
		m_Global->SsdtHook().InitializeSsdtList(&m_HookListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CSsdtHook::QuerySsdtHookCallback, &m_HookListCtrl, 0, NULL)
		);

		break;
	}
	case ArkProtect::hi_Sssdt:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::hi_Sssdt)
		{
			m_HookListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		m_HookModuleListBox.ShowWindow(FALSE);

		// ��ʼ��ListCtrl
		m_Global->SssdtHook().InitializeSssdtList(&m_HookListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CSssdtHook::QuerySssdtHookCallback, &m_HookListCtrl, 0, NULL)
		);

		break;
	}

	default:
		break;
	}

	m_HookListCtrl.SetFocus();

}


void CHookDlg::OnNMCustomdrawHookListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		BOOL bHooked = 0;
		int iItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		clrNewTextColor = RGB(0, 0, 0);
		clrNewBkColor = RGB(255, 255, 255);

		bHooked = (BOOL)m_HookListCtrl.GetItemData(iItem);
		if (bHooked == TRUE)
		{
			clrNewTextColor = RGB(255, 0, 0);
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}



void CHookDlg::OnSsdtFreshen()
{
	// TODO: �ڴ���������������
	
	m_iCurSel = 65535;

	m_HookListBox.SetCurSel(ArkProtect::hi_Ssdt);

	OnLbnSelchangeHookListbox();

	m_HookListCtrl.SetFocus();

}



void CHookDlg::OnSsdtResume()
{
	// TODO: �ڴ���������������

	if (m_Global->m_bIsRequestNow == TRUE)
	{
		return;
	}

	// ���ؽ�����Ϣ�б�
	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ArkProtect::CSsdtHook::ResumeSsdtHookCallback, &m_HookListCtrl, 0, NULL)
	);
}



/************************************************************************
*  Name : APInitializeHookItemList
*  Param: void
*  Ret  : void
*  ��ʼ���ں˹��ӵ�ListBox���ں��
************************************************************************/
void CHookDlg::APInitializeHookItemList()
{
	m_HookListBox.AddString(L"SSDT");
	m_HookListBox.InsertString(ArkProtect::hi_Sssdt, L"ShadowSSDT");
	m_HookListBox.InsertString(ArkProtect::hi_NtkrnlFunc, L"�ں˺���");
	m_HookListBox.InsertString(ArkProtect::hi_NtkrnlIAT, L"�ں˵����");
	m_HookListBox.InsertString(ArkProtect::hi_NtkrnlEAT, L"�ں˵�����");

	m_HookListBox.SetItemHeight(-1, (UINT)(16 * (m_Global->iDpiy / 96.0)));
	m_HookModuleListBox.SetItemHeight(-1, (UINT)(16 * (m_Global->iDpiy / 96.0)));
}






void CHookDlg::OnNMRClickHookListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu Menu;
	Menu.LoadMenuW(IDR_SSDT_MENU);
	CMenu* SubMenu = Menu.GetSubMenu(0);	// �Ӳ˵�

	CPoint Pt;
	GetCursorPos(&Pt);         // �õ����λ��

	int	iCount = SubMenu->GetMenuItemCount();

	// ���û��ѡ��,����ˢ�� ����ȫ��Disable
	if (m_HookListCtrl.GetSelectedCount() == 0)
	{
		for (int i = 0; i < iCount; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
		}

		SubMenu->EnableMenuItem(ID_DRIVER_FRESHEN, MF_BYCOMMAND | MF_ENABLED);
	}

	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Pt.x, Pt.y, this);

	*pResult = 0;
}
