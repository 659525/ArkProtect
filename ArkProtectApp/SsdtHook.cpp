#include "stdafx.h"
#include "SsdtHook.h"
#include "Global.hpp"

namespace ArkProtect
{
	CSsdtHook *CSsdtHook::m_SsdtHook;

	CSsdtHook::CSsdtHook(CGlobal *GlobalObject)
		: m_Global(GlobalObject)
		, m_DriverCore(GlobalObject->DriverCore())
	{
		m_SsdtHook = this;
	}


	CSsdtHook::~CSsdtHook()
	{
	}


	/************************************************************************
	*  Name : InitializeSsdtList
	*  Param: ListCtrl               ListControl�ؼ�
	*  Ret  : void
	*  ��ʼ��ListControl����Ϣ
	************************************************************************/
	void CSsdtHook::InitializeSsdtList(CListCtrl *ListCtrl)
	{
		while (ListCtrl->DeleteColumn(0));
		ListCtrl->DeleteAllItems();

		ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

		for (int i = 0; i < m_iColumnCount; i++)
		{
			ListCtrl->InsertColumn(i, m_ColumnStruct[i].wzTitle, LVCFMT_LEFT, (int)(m_ColumnStruct[i].nWidth*(m_Global->iDpiy / 96.0)));
		}
	}



	/************************************************************************
	*  Name : EnumSsdtHook
	*  Param: void
	*  Ret  : BOOL
	*  ��������ͨ�ţ�ö��Ssdt��Ϣ
	************************************************************************/
	BOOL CSsdtHook::EnumSsdtHook()
	{
		m_SsdtHookEntryVector.clear();

		BOOL bOk = FALSE;
		UINT32   Count = 0x100;
		DWORD	 dwReturnLength = 0;
		PSSDT_HOOK_INFORMATION shi = NULL;

		do
		{
			UINT32 OutputLength = 0;

			if (shi)
			{
				free(shi);
				shi = NULL;
			}

			OutputLength = sizeof(SSDT_HOOK_INFORMATION) + Count * sizeof(SSDT_HOOK_ENTRY_INFORMATION);

			shi = (PSSDT_HOOK_INFORMATION)malloc(OutputLength);
			if (!shi)
			{
				break;
			}

			RtlZeroMemory(shi, OutputLength);

			bOk = DeviceIoControl(m_Global->m_DeviceHandle,
				IOCTL_ARKPROTECT_ENUMSSDTHOOK,
				NULL,		// InputBuffer
				0,
				shi,
				OutputLength,
				&dwReturnLength,
				NULL);

			Count = (UINT32)shi->NumberOfSsdtHooks + 100;

		} while (bOk == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

		if (bOk && shi)
		{
			m_SsdtFunctionCount = shi->NumberOfSsdtFunctions;    // ����SsdtFunction�ĸ���
			for (UINT32 i = 0; i < shi->NumberOfSsdtHooks; i++)
			{
				// ���ƽ�����Ϣ�ṹ
				m_SsdtHookEntryVector.push_back(shi->SsdtHookEntry[i]);
			}
			bOk = TRUE;
		}

		if (shi)
		{
			free(shi);
			shi = NULL;
		}

		if (m_SsdtHookEntryVector.empty())
		{
			return FALSE;
		}

		return bOk;
	}


	/************************************************************************
	*  Name : InsertSsdtHookInfoList
	*  Param: ListCtrl
	*  Ret  : void
	*  ��ListControl����������Ϣ
	************************************************************************/
	void CSsdtHook::InsertSsdtHookInfoList(CListCtrl *ListCtrl)
	{
		UINT32 SsdtHookNum = 0;
		size_t Size = m_SsdtHookEntryVector.size();
		for (size_t i = 0; i < Size; i++)
		{
			SSDT_HOOK_ENTRY_INFORMATION SsdtHookEntry = m_SsdtHookEntryVector[i];

			CString strOrdinal, strFunctionName, strCurrentAddress, strOriginalAddress, strStatus, strFilePath;

			strOrdinal.Format(L"%d", SsdtHookEntry.Ordinal);
			strFunctionName = SsdtHookEntry.wzFunctionName;
			strCurrentAddress.Format(L"0x%p", SsdtHookEntry.CurrentAddress);
			strOriginalAddress.Format(L"0x%p", SsdtHookEntry.OriginalAddress);
			strStatus = L"Hooked";
			strFilePath = m_DriverCore.GetDriverPathByAddress(SsdtHookEntry.CurrentAddress);


			int iItem = ListCtrl->InsertItem(ListCtrl->GetItemCount(), strOrdinal);
			ListCtrl->SetItemText(iItem, shc_FunctionName, strFunctionName);
			ListCtrl->SetItemText(iItem, shc_CurrentAddress, strCurrentAddress);
			ListCtrl->SetItemText(iItem, shc_OriginalAddress, strOriginalAddress);
			ListCtrl->SetItemText(iItem, shc_Status, strStatus);
			ListCtrl->SetItemText(iItem, shc_FilePath, strFilePath);

			SsdtHookNum++;

			CString strStatusContext;
			strStatusContext.Format(L"Ssdt Hook Info is loading now, Count:%d", SsdtHookNum);

			m_Global->UpdateStatusBarDetail(strStatusContext);
		}

		CString strStatusContext;
		strStatusContext.Format(L"Ssdt����: %d�����ҹ�����: %d",m_SsdtFunctionCount, Size);
		m_Global->UpdateStatusBarDetail(strStatusContext);
	}


	/************************************************************************
	*  Name : QuerySsdtHook
	*  Param: ListCtrl
	*  Ret  : void
	*  ��ѯ������Ϣ
	************************************************************************/
	void CSsdtHook::QuerySsdtHook(CListCtrl *ListCtrl)
	{
		ListCtrl->DeleteAllItems();
		m_SsdtHookEntryVector.clear();
		m_DriverCore.DriverEntryVector().clear();

		if (EnumSsdtHook() == FALSE)
		{
			m_Global->UpdateStatusBarDetail(L"Ssdt Hook Initialize failed");
			return;
		}

		if (m_DriverCore.EnumDriverInfo() == FALSE)
		{
			m_Global->UpdateStatusBarDetail(L"Ssdt Hook Initialize failed");
			return;
		}

		InsertSsdtHookInfoList(ListCtrl);
	}


	/************************************************************************
	*  Name : QuerySsdtHookCallback
	*  Param: lParam ��ListCtrl��
	*  Ret  : DWORD
	*  ��ѯ����ģ��Ļص�
	************************************************************************/
	DWORD CALLBACK CSsdtHook::QuerySsdtHookCallback(LPARAM lParam)
	{
		CListCtrl *ListCtrl = (CListCtrl*)lParam;

		m_SsdtHook->m_Global->m_bIsRequestNow = TRUE;      // ��TRUE����������û�з���ǰ����ֹ����������ͨ�ŵĲ���

		m_SsdtHook->m_Global->UpdateStatusBarTip(L"Ssdt");
		m_SsdtHook->m_Global->UpdateStatusBarDetail(L"Ssdt is loading now...");

		m_SsdtHook->QuerySsdtHook(ListCtrl);

		m_SsdtHook->m_Global->m_bIsRequestNow = FALSE;

		return 0;
	}

}