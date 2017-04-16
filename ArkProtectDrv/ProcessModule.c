#include "ProcessModule.h"



/************************************************************************
*  Name : APIsModuleInList
*  Param: BaseAddress			ģ�����ַ��OUT��
*  Param: ModuleSize			ģ���С��IN��
*  Ret  : NTSTATUS
*  ͨ��FileObject��ý�������·��
************************************************************************/
BOOLEAN
APIsModuleInList(IN UINT_PTR BaseAddress, IN UINT32 ModuleSize, IN PPROCESS_MODULE_INFORMATION pmi, IN UINT32 ModuleCount)
{
	BOOLEAN bOk = FALSE;
	UINT32  i = 0;
	ModuleCount = pmi->NumberOfModules > ModuleCount ? ModuleCount : pmi->NumberOfModules;

	for (i = 0; i < ModuleCount; i++)
	{
		if (BaseAddress == pmi->ModuleEntry[i].BaseAddress &&
			ModuleSize == pmi->ModuleEntry[i].SizeOfImage)
		{
			bOk = TRUE;
			break;
		}
	}
	return bOk;
}


/************************************************************************
*  Name : APFillProcessModuleInfoByTravelLdr
*  Param: LdrListEntry			ģ�����ַ��OUT��
*  Param: ModuleSize			ģ���С��IN��
*  Ret  : NTSTATUS
*  ͨ��FileObject��ý�������·��
************************************************************************/
VOID
APFillProcessModuleInfoByTravelLdr(IN PLIST_ENTRY LdrListEntry, IN eLdrType LdrType, OUT PPROCESS_MODULE_INFORMATION pmi, IN UINT32 ModuleCount)
{

	for (PLIST_ENTRY TravelListEntry = LdrListEntry->Flink;
		TravelListEntry != LdrListEntry;
		TravelListEntry = (PLIST_ENTRY)TravelListEntry->Flink)
	{
		PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = NULL;
		switch (LdrType)
		{
		case lt_InLoadOrderModuleList:
		{
			LdrDataTableEntry = CONTAINING_RECORD(TravelListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			break;
		}
		case lt_InMemoryOrderModuleList:
		{
			LdrDataTableEntry = CONTAINING_RECORD(TravelListEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			break;
		}
		case lt_InInitializationOrderModuleList:
		{
			LdrDataTableEntry = CONTAINING_RECORD(TravelListEntry, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
			break;
		}
		default:
			break;
		}

		if ((PUINT8)LdrDataTableEntry > 0 && MmIsAddressValid(LdrDataTableEntry))
		{
			// ����
			if (!APIsModuleInList((UINT_PTR)LdrDataTableEntry->DllBase, LdrDataTableEntry->SizeOfImage, pmi, ModuleCount))
			{
				if (ModuleCount > pmi->NumberOfModules)	// Ring3���Ĵ� �ͼ�����
				{
					pmi->ModuleEntry[pmi->NumberOfModules].BaseAddress = (UINT_PTR)LdrDataTableEntry->DllBase;
					pmi->ModuleEntry[pmi->NumberOfModules].SizeOfImage = LdrDataTableEntry->SizeOfImage;
					//wcsncpy(pmi->ModuleEntry[pmi->NumberOfModules].wzFullPath, LdrDataTableEntry->FullDllName.Buffer, LdrDataTableEntry->FullDllName.Length);
					StringCchCopyW(pmi->ModuleEntry[pmi->NumberOfModules].wzFullPath, LdrDataTableEntry->FullDllName.Length, LdrDataTableEntry->FullDllName.Buffer);
				}
				pmi->NumberOfModules++;
			}
		}
	}
}


/************************************************************************
*  Name : APFillProcessModuleInfoByTravelLdr
*  Param: LdrListEntry			ģ�����ַ��OUT��
*  Param: ModuleSize			ģ���С��IN��
*  Ret  : NTSTATUS
*  ͨ��FileObject��ý�������·��
************************************************************************/
NTSTATUS
APEnumProcessModuleByPeb(IN PEPROCESS EProcess, OUT PPROCESS_MODULE_INFORMATION pmi, IN UINT32 ModuleCount)
{
	BOOLEAN bAttach = FALSE;
	KAPC_STATE ApcState;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	KeStackAttachProcess(EProcess, &ApcState);     // attach��Ŀ�������
	bAttach = TRUE;

	__try
	{
		LARGE_INTEGER	Interval = { 0 };
		Interval.QuadPart = -25011 * 10 * 1000;		// 250 ����

		if (TRUE)		// ��Ҫ���� Wow64������
		{
			PPEB_LDR_DATA LdrData = NULL;
			PPEB Peb = PsGetProcessPeb(EProcess);
			if (Peb == NULL)
			{
				return Status;
			}

			for (INT i = 0; Peb->Ldr == 0 && i < 10; i++)
			{
				// Sleep �ȴ�����
				KeDelayExecutionThread(KernelMode, TRUE, &Interval);
			}

			LdrData = Peb->Ldr;
			if ((PUINT8)LdrData > 0)
			{
				// ö����������
				APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InLoadOrderModuleList), lt_InLoadOrderModuleList, pmi, ModuleCount);
				APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InMemoryOrderModuleList), lt_InMemoryOrderModuleList, pmi, ModuleCount);
				APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InInitializationOrderModuleList), lt_InInitializationOrderModuleList, pmi, ModuleCount);
				Status = STATUS_SUCCESS;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("EnumDllModuleByPeb Catch __Except\r\n");
		Status = STATUS_UNSUCCESSFUL;
	}

	if (bAttach)
	{
		KeUnstackDetachProcess(&ApcState);
		bAttach = FALSE;
	}

	return Status;
}



NTSTATUS
APEnumProcessModule(IN UINT32 ProcessId, OUT PVOID OutputBuffer, IN UINT32 OutputLength)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;

	UINT32    ModuleCount = (OutputLength - sizeof(PROCESS_MODULE_INFORMATION)) / sizeof(PROCESS_MODULE_ENTRY_INFORMATION);
	PEPROCESS EProcess = NULL;

	if (ProcessId == 0)
	{
		return Status;
	}
	else
	{
		Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
	}

	if (NT_SUCCESS(Status) && APIsValidProcess(EProcess))
	{
		PPROCESS_MODULE_INFORMATION pmi = (PPROCESS_MODULE_INFORMATION)ExAllocatePool(PagedPool, OutputLength);
		if (pmi)
		{
			RtlZeroMemory(pmi, OutputLength);

			Status = APEnumProcessModuleByPeb(EProcess, pmi, ModuleCount);
			if (NT_SUCCESS(Status))
			{
				if (ModuleCount >= pmi->NumberOfModules)
				{
					RtlCopyMemory(OutputBuffer, pmi, OutputLength);
					Status = STATUS_SUCCESS;
				}
				else
				{
					((PPROCESS_MODULE_INFORMATION)OutputBuffer)->NumberOfModules = pmi->NumberOfModules;    // ��Ring3֪����Ҫ���ٸ�
					Status = STATUS_BUFFER_TOO_SMALL;	// ��ring3�����ڴ治������Ϣ
				}
			}

			ExFreePool(pmi);
			pmi = NULL;
		}
	}

	if (EProcess)
	{
		ObDereferenceObject(EProcess);
	}

	return Status;
}






