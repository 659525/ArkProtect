#include "stdafx.h"
#include "ProcessModule.h"

namespace ArkProtect
{
	CProcessModule *CProcessModule::m_ProcessModule;

	CProcessModule::CProcessModule(CGlobal *GlobalObject, PPROCESS_ENTRY_INFORMATION ProcessEntry)
		: m_Global(GlobalObject)
		, m_ProcessEntry(ProcessEntry)
	{
		m_ProcessModule = this;
	}


	CProcessModule::~CProcessModule()
	{
	}



	/************************************************************************
	*  Name : InitializeProcessModuleList
	*  Param: ProcessInfoList        ProcessModule�Ի����ListControl�ؼ�
	*  Ret  : void
	*  ��ʼ��ListControl����Ϣ
	************************************************************************/
	void CProcessModule::InitializeProcessModuleList(CListCtrl *ProcessInfoList)
	{
		ProcessInfoList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

		for (int i = 0; i < m_iColumnCount; i++)
		{
			ProcessInfoList->InsertColumn(i, m_ColumnStruct[i].wzTitle, LVCFMT_LEFT, (int)(m_ColumnStruct[i].nWidth*(m_Global->iDpiy / 96.0)));
		}
	}

	

	/************************************************************************
	*  Name : EnumProcessModule
	*  Param: void
	*  Ret  : BOOL
	*  ��������ͨ�ţ�ö�ٽ���ģ����Ϣ
	************************************************************************/
	BOOL CProcessModule::EnumProcessModule()
	{
		BOOL bOk = FALSE;

		m_ProcessModuleEntryVector.clear();

		UINT32   Count = 0x100;
		DWORD	 dwReturnLength = 0;
		PPROCESS_MODULE_INFORMATION pmi = NULL;

		do
		{
			UINT32 OutputLength = 0;

			if (pmi)
			{
				free(pmi);
				pmi = NULL;
			}

			OutputLength = sizeof(PROCESS_MODULE_INFORMATION) + Count * sizeof(PROCESS_MODULE_ENTRY_INFORMATION);

			pmi = (PPROCESS_MODULE_INFORMATION)malloc(OutputLength);
			if (!pmi)
			{
				break;
			}

			RtlZeroMemory(pmi, OutputLength);

			bOk = DeviceIoControl(m_Global->m_DeviceHandle,
				IOCTL_ARKPROTECT_ENUMPROCESSMODULE,
				&m_ProcessEntry->ProcessId,		// InputBuffer
				sizeof(UINT32),
				pmi,
				OutputLength,
				&dwReturnLength,
				NULL);

			Count = (UINT32)pmi->NumberOfModules + 1000;

		} while (bOk == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

		if (bOk && pmi)
		{
			for (UINT32 i = 0; i < pmi->NumberOfModules; i++)
			{
				// ���ƽ�����Ϣ�ṹ
				//PerfectProcessInfo(pmi->ModuleEntry[i]);
				m_ProcessModuleEntryVector.push_back(pmi->ModuleEntry[i]);
			}
			bOk = TRUE;
		}

		if (pmi)
		{
			free(pmi);
			pmi = NULL;
		}

		if (m_ProcessModuleEntryVector.empty())
		{
			return FALSE;
		}

		return bOk;
	}



	/************************************************************************
	*  Name : QueryProcessModule
	*  Param: ListCtrl
	*  Ret  : void
	*  ��ѯ������Ϣ
	************************************************************************/
	void CProcessModule::QueryProcessModule(CListCtrl *ListCtrl)
	{
		ListCtrl->DeleteAllItems();
		m_ProcessModuleEntryVector.clear();

		if (EnumProcessModule() == FALSE)
		{
			m_Global->UpdateStatusBarDetail(L"Process Module Initialize failed");
			return;
		}

	//	InsertProcessModuleList(ListCtrl);
	}



	/************************************************************************
	*  Name : QueryProcessModuleCallback
	*  Param: lParam ��ListCtrl��
	*  Ret  : DWORD
	*  ��ѯ����ģ��Ļص�
	************************************************************************/
	DWORD CALLBACK CProcessModule::QueryProcessModuleCallback(LPARAM lParam)
	{
		CListCtrl *ListCtrl = (CListCtrl*)lParam;

		m_ProcessModule->m_Global->m_bIsRequestNow = TRUE;      // ��TRUE����������û�з���ǰ����ֹ����������ͨ�ŵĲ���

		m_ProcessModule->m_Global->UpdateStatusBarTip(L"Process Module");
		m_ProcessModule->m_Global->UpdateStatusBarDetail(L"Process Module is loading now...");

		m_ProcessModule->QueryProcessModule(ListCtrl);

		m_ProcessModule->m_Global->m_bIsRequestNow = FALSE;

		return 0;
	}




}