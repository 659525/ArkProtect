// KernelDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ArkProtectApp.h"
#include "KernelDlg.h"
#include "afxdialogex.h"

#include "ArkProtectAppDlg.h"

// CKernelDlg �Ի���

IMPLEMENT_DYNAMIC(CKernelDlg, CDialogEx)

CKernelDlg::CKernelDlg(CWnd* pParent /*=NULL*/, ArkProtect::CGlobal *GlobalObject)
	: CDialogEx(IDD_KERNEL_DIALOG, pParent)
	, m_Global(GlobalObject)
{
	// ����Ի���ָ��
	m_Global->m_KernelDlg = this;
}

CKernelDlg::~CKernelDlg()
{
}

void CKernelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KERNEL_LISTBOX, m_KernelListBox);
	DDX_Control(pDX, IDC_KERNEL_LISTCTRL, m_KernelListCtrl);
}


BEGIN_MESSAGE_MAP(CKernelDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_KERNEL_LISTBOX, &CKernelDlg::OnLbnSelchangeKernelListbox)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_KERNEL_LISTCTRL, &CKernelDlg::OnNMCustomdrawKernelListctrl)
END_MESSAGE_MAP()


// CKernelDlg ��Ϣ�������


BOOL CKernelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	APInitializeKernelItemList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CKernelDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()

	CRect   Rect;
	GetClientRect(Rect);
	dc.FillSolidRect(Rect, RGB(255, 255, 255));  // �����ɫ

	CRect KernelListBoxRect;
	CRect KernelListCtrlRect;

	// ���ListBox��Rect
	m_KernelListBox.GetWindowRect(&KernelListBoxRect);
	ClientToScreen(&Rect);
	KernelListBoxRect.left -= Rect.left;
	KernelListBoxRect.right -= Rect.left;
	KernelListBoxRect.top -= Rect.top;
	KernelListBoxRect.bottom = Rect.Height() - 2;

	m_KernelListBox.MoveWindow(KernelListBoxRect);

	CPoint StartPoint;
	StartPoint.x = (LONG)(KernelListBoxRect.right) + 2;
	StartPoint.y = -1;

	CPoint EndPoint;
	EndPoint.x = (LONG)(KernelListBoxRect.right) + 2;
	EndPoint.y = Rect.Height() + 2;

	KernelListCtrlRect.left = StartPoint.x + 1;
	KernelListCtrlRect.right = Rect.Width();
	KernelListCtrlRect.top = 0;
	KernelListCtrlRect.bottom = Rect.Height();
	m_KernelListCtrl.MoveWindow(KernelListCtrlRect);

	COLORREF ColorRef(RGB(190, 190, 190));

	CClientDC ClientDc(this);
	CPen Pen(PS_SOLID, 1, ColorRef);
	ClientDc.SelectObject(&Pen);
	ClientDc.MoveTo(StartPoint);
	ClientDc.LineTo(EndPoint);

}



void CKernelDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	if (bShow == TRUE)
	{
		// ���¸�������Ϣ CurrentChildDlg �� ���õ�ǰ�Ӵ��ڵ�button
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_CurrentChildDlg = ArkProtect::cd_KernelDialog;
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_KernelButton.EnableWindow(FALSE);

		m_KernelListBox.SetCurSel(ArkProtect::ki_SysCallback);

		OnLbnSelchangeKernelListbox();

		m_KernelListCtrl.SetFocus();

	}
	else
	{
		m_iCurSel = 65535;
	}
}


void CKernelDlg::OnNMCustomdrawKernelListctrl(NMHDR *pNMHDR, LRESULT *pResult)
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
		BOOL bNotTrust = FALSE;
		int iItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		clrNewTextColor = RGB(0, 0, 0);
		clrNewBkColor = RGB(255, 255, 255);

		bNotTrust = (BOOL)m_KernelListCtrl.GetItemData(iItem);
		if (bNotTrust == TRUE)
		{
			clrNewTextColor = RGB(0, 0, 255);
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}


void CKernelDlg::OnLbnSelchangeKernelListbox()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	int iCurSel = m_KernelListBox.GetCurSel();

	switch (iCurSel)
	{
	case ArkProtect::ki_SysCallback:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::ki_SysCallback)
		{
			m_KernelListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		// ��ʼ��ListCtrl
		m_Global->SystemCallback().InitializeCallbackList(&m_KernelListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CSystemCallback::QuerySystemCallbackCallback, &m_KernelListCtrl, 0, NULL)
		);

		break;
	}
	case ArkProtect::ki_FilterDriver:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::ki_FilterDriver)
		{
			m_KernelListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		// ��ʼ��ListCtrl
		m_Global->FilterDriver().InitializeFilterDriverList(&m_KernelListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CFilterDriver::QueryFilterDriverCallback, &m_KernelListCtrl, 0, NULL)
		);

		break;
	}
	case ArkProtect::ki_IoTimer:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::ki_IoTimer)
		{
			m_KernelListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		// ��ʼ��ListCtrl
		m_Global->IoTimer().InitializeIoTimerList(&m_KernelListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CIoTimer::QueryIoTimerCallback, &m_KernelListCtrl, 0, NULL)
		);

		break;
	}
	case ArkProtect::ki_DpcTimer:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::ki_DpcTimer)
		{
			m_KernelListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		// ��ʼ��ListCtrl
		m_Global->DpcTimer().InitializeDpcTimerList(&m_KernelListCtrl);

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CDpcTimer::QueryDpcTimerCallback, &m_KernelListCtrl, 0, NULL)
		);

		break;
	}
	case ArkProtect::ki_SysThread:
	{
		if (m_Global->m_bIsRequestNow == TRUE || m_iCurSel == ArkProtect::ki_SysThread)
		{
			m_KernelListBox.SetCurSel(m_iCurSel);
			break;
		}

		m_iCurSel = iCurSel;

		// ��ʼ��ListCtrl
		m_Global->ProcessThread().InitializeProcessThreadList(&m_KernelListCtrl);

		m_Global->ProcessCore().ProcessEntry()->ProcessId = 4;

		// ���ؽ�����Ϣ�б�
		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ArkProtect::CProcessThread::QueryProcessThreadCallback, &m_KernelListCtrl, 0, NULL)
		);

		break;
	}

	default:
		break;
	}

	m_KernelListCtrl.SetFocus();
}



/************************************************************************
*  Name : APInitializeKernelItemList
*  Param: void
*  Ret  : void
*  ��ʼ���ں�ģ���ListBox���ں��
************************************************************************/
void CKernelDlg::APInitializeKernelItemList()
{
	m_KernelListBox.AddString(L"ϵͳ�ص�");
	m_KernelListBox.InsertString(ArkProtect::ki_FilterDriver, L"��������");
	m_KernelListBox.InsertString(ArkProtect::ki_IoTimer, L"IOTimer");
	m_KernelListBox.InsertString(ArkProtect::ki_DpcTimer, L"DPCTimer");
	m_KernelListBox.InsertString(ArkProtect::ki_SysThread, L"ϵͳ�߳�");

	m_KernelListBox.SetItemHeight(-1, (UINT)(16 * (m_Global->iDpiy / 96.0)));
}






