#pragma once
#include <vector>
#include "Define.h"
#include "Global.hpp"
#include "ProcessCore.h"
#include "ProcessModule.h"

namespace ArkProtect
{
	enum eProcessThreadColumn
	{
		ptc_ThreadId,        // �߳�id
		ptc_EThread,
		ptc_Teb,
		ptc_Priority,
		ptc_Entrance,
		ptc_Module,
		ptc_switches,
		ptc_Status
	};

	typedef struct _PROCESS_THREAD_ENTRY_INFORMATION
	{
		UINT_PTR EThread;
		UINT32   ThreadId;
		UINT_PTR Teb;
		UINT8    Priority;
		UINT_PTR Win32StartAddress;
		UINT32   ContextSwitches;
		UINT8    State;
	} PROCESS_THREAD_ENTRY_INFORMATION, *PPROCESS_THREAD_ENTRY_INFORMATION;


	typedef struct _PROCESS_THREAD_INFORMATION
	{
		UINT32                           NumberOfThreads;
		PROCESS_THREAD_ENTRY_INFORMATION ThreadEntry[1];
	} PROCESS_THREAD_INFORMATION, *PPROCESS_THREAD_INFORMATION;


	class CProcessThread
	{
	public:
		CProcessThread(CGlobal *GlobalObject, PPROCESS_ENTRY_INFORMATION ProcessEntry);
		~CProcessThread();
		void InitializeProcessThreadList(CListCtrl * ListCtrl);




		BOOL EnumProcessThread();

		void QueryProcessThread(CListCtrl * ListCtrl);

		static DWORD CALLBACK QueryProcessThreadCallback(LPARAM lParam);





	private:
		int           m_iColumnCount = 8;		// �����б���
		COLUMN_STRUCT m_ColumnStruct[8] = {
			{ L"�߳�ID",			50 },
			{ L"ETHREAD",			125 },
			{ L"Teb",				125 },
			{ L"���ȼ�",			54 },
			{ L"�߳����",			125 },
			{ L"ģ��",				90 },
			{ L"�л�����",			68 },
			{ L"״̬",				50 } };



		std::vector<PROCESS_THREAD_ENTRY_INFORMATION> m_ProcessThreadEntryVector;
		std::vector<PROCESS_MODULE_ENTRY_INFORMATION> m_ProcessModuleEntryVector;


		CGlobal                    *m_Global;
		PPROCESS_ENTRY_INFORMATION m_ProcessEntry;

		static CProcessThread      *m_ProcessThread;

	};



}



