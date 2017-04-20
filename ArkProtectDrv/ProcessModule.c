#include "ProcessModule.h"

extern DYNAMIC_DATA	g_DynamicData;

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
*  Name : APEnumProcessModuleByPeb
*  Param: EProcess			      ���̽ṹ��
*  Param: pmi			          ring3�ڴ�
*  Param: ModuleCount
*  Ret  : NTSTATUS
*  ͨ������peb��Ldr���������е�һ��������Wow64��
************************************************************************/
NTSTATUS
APEnumProcessModuleByZwQueryVirtualMemory(IN PEPROCESS EProcess, OUT PPROCESS_MODULE_INFORMATION pmi, IN UINT32 ModuleCount)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE   ProcessHandle = NULL;

	Status = ObOpenObjectByPointer(EProcess, 
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		GENERIC_ALL,
		*PsProcessType,
		KernelMode,
		&ProcessHandle);
	if (NT_SUCCESS(Status))
	{
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		PMEMORY_SECTION_NAME msn = (PMEMORY_SECTION_NAME)ExAllocatePool(PagedPool, MAX_PATH);
		if (msn)
		{
			__try
			{
				for (SIZE_T BaseAddress = 0; BaseAddress <= (SIZE_T)g_DynamicData.MaxUserSpaceAddress; BaseAddress += 0x10000)
				{
					SIZE_T ReturnLength = 0;

					Status = ZwQueryVirtualMemory(ProcessHandle,
						(PVOID)BaseAddress,
						MemoryBasicInformation,
						(PVOID)&mbi,
						sizeof(MEMORY_BASIC_INFORMATION),
						&ReturnLength);
					// �ж����ͣ�����Image���ѯSectionName
					if (NT_SUCCESS(Status) && mbi.Type == MEM_IMAGE)
					{
						Status = ZwQueryVirtualMemory(ProcessHandle,
							(PVOID)BaseAddress,
							MemorySectionName,
							(PVOID)msn,
							sizeof(MEMORY_SECTION_NAME),
							&ReturnLength);
						// �жϴ����ǲ���������ڴ治��
						if (!NT_SUCCESS(Status) && Status == STATUS_INFO_LENGTH_MISMATCH)
						{
							ExFreePool(msn);
							msn = (PMEMORY_SECTION_NAME)ExAllocatePool(PagedPool, ReturnLength);
							if (msn)
							{
								Status = ZwQueryVirtualMemory(ProcessHandle,
									(PVOID)BaseAddress,
									MemorySectionName,
									(PVOID)msn,
									sizeof(MEMORY_SECTION_NAME),
									&ReturnLength);
								if (!NT_SUCCESS(Status))
								{
									continue;
								}
							}
							else
							{
								continue;
							}
						}
					
						if (!APIsModuleInList((UINT_PTR)mbi.BaseAddress, (UINT32)mbi.RegionSize, pmi, ModuleCount))
						{
							if (ModuleCount > pmi->NumberOfModules)	// Ring3���Ĵ� �ͼ�����
							{	
								WCHAR  wzNtFullPath[MAX_PATH] = { 0 };
								SIZE_T TravelAddress = 0;

								// ģ������
								APDosPathToNtPath(msn->NameBuffer, wzNtFullPath);
								StringCchCopyW(pmi->ModuleEntry[pmi->NumberOfModules].wzFilePath, wcslen(wzNtFullPath) + 1, wzNtFullPath);
								
								// ģ�����ַ
								pmi->ModuleEntry[pmi->NumberOfModules].BaseAddress = (UINT_PTR)mbi.BaseAddress;
								
								// ���ģ���С
								for (SIZE_T TravelAddress = 0; TravelAddress <= (SIZE_T)g_DynamicData.MaxUserSpaceAddress; TravelAddress += mbi.RegionSize)
								{
									Status = ZwQueryVirtualMemory(ProcessHandle,
										(PVOID)TravelAddress,
										MemoryBasicInformation,
										(PVOID)&mbi,
										sizeof(MEMORY_BASIC_INFORMATION),
										&ReturnLength);
									if (NT_SUCCESS(Status) && mbi.Type != MEM_IMAGE)
									{
										break;
									}
								}

								pmi->ModuleEntry[pmi->NumberOfModules].SizeOfImage = TravelAddress - BaseAddress;
								
							}
							pmi->NumberOfModules++;
						}
					}
				}

				// ö�ٵ��˶���
				if (pmi->NumberOfModules)
				{
					Status = STATUS_SUCCESS;
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				DbgPrint("Catch Exception\r\n");
				Status = STATUS_UNSUCCESSFUL;
			}
		}
		ZwClose(ProcessHandle);
	}
	return Status;
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
					StringCchCopyW(pmi->ModuleEntry[pmi->NumberOfModules].wzFilePath, LdrDataTableEntry->FullDllName.Length, LdrDataTableEntry->FullDllName.Buffer);
				}
				pmi->NumberOfModules++;
			}
		}
	}
}


/************************************************************************
*  Name : APEnumProcessModuleByPeb
*  Param: EProcess			      ���̽ṹ��
*  Param: pmi			          ring3�ڴ�
*  Param: ModuleCount			  
*  Ret  : NTSTATUS
*  ͨ������peb��Ldr���������е�һ��������Wow64��
************************************************************************/
NTSTATUS
APEnumProcessModuleByPeb(IN PEPROCESS EProcess, OUT PPROCESS_MODULE_INFORMATION pmi, IN UINT32 ModuleCount)
{
	BOOLEAN bAttach = FALSE;
	KAPC_STATE ApcState;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PPEB Peb = NULL;

	KeStackAttachProcess(EProcess, &ApcState);     // attach��Ŀ�������,��ΪҪ���ʵ����û��ռ��ַ
	bAttach = TRUE;

	__try
	{
		LARGE_INTEGER	Interval = { 0 };
		Interval.QuadPart = -25011 * 10 * 1000;		// 250 ����

		if (PsGetProcessWow64Process(EProcess))		// ��Ҫ���� Wow64������
		{
			PPEB32 Peb32 = (PPEB32)PsGetProcessWow64Process(EProcess);
			if (Peb32 == NULL)
			{
				return Status;
			}
			
			for (INT i = 0; !Peb32->Ldr && i < 10; i++)
			{
				// Sleep �ȴ�����
				KeDelayExecutionThread(KernelMode, TRUE, &Interval);
			}

			if (Peb32->Ldr)
			{
				ProbeForRead((PVOID)Peb32->Ldr, sizeof(UINT32), sizeof(UINT8));

				// Travel InLoadOrderModuleList
				for (PLIST_ENTRY32 TravelListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)Peb32->Ldr)->InLoadOrderModuleList.Flink;
					TravelListEntry != &((PPEB_LDR_DATA32)Peb32->Ldr)->InLoadOrderModuleList;
					TravelListEntry = (PLIST_ENTRY32)TravelListEntry->Flink)
				{
					PLDR_DATA_TABLE_ENTRY32 LdrDataTableEntry32 = NULL;
					LdrDataTableEntry32 = CONTAINING_RECORD(TravelListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

					if ((PUINT8)LdrDataTableEntry32 > 0 && MmIsAddressValid(LdrDataTableEntry32))
					{
						// ����
						if (!APIsModuleInList((UINT_PTR)LdrDataTableEntry32->DllBase, LdrDataTableEntry32->SizeOfImage, pmi, ModuleCount))
						{
							if (ModuleCount > pmi->NumberOfModules)	// Ring3���Ĵ� �ͼ�����
							{
								pmi->ModuleEntry[pmi->NumberOfModules].BaseAddress = (UINT_PTR)LdrDataTableEntry32->DllBase;
								pmi->ModuleEntry[pmi->NumberOfModules].SizeOfImage = LdrDataTableEntry32->SizeOfImage;
								StringCchCopyW(pmi->ModuleEntry[pmi->NumberOfModules].wzFilePath, LdrDataTableEntry32->FullDllName.Length, (LPCWSTR)LdrDataTableEntry32->FullDllName.Buffer);
							}
							pmi->NumberOfModules++;
						}
					}
				}
				// ö�ٵ��˶���
				if (pmi->NumberOfModules)
				{
					Status = STATUS_SUCCESS;
				}
			}
		}

		// Native process
		Peb = PsGetProcessPeb(EProcess);
		if (Peb == NULL)
		{
			return Status;
		}

		for (INT i = 0; Peb->Ldr == 0 && i < 10; i++)
		{
			// Sleep �ȴ�����
			KeDelayExecutionThread(KernelMode, TRUE, &Interval);
		}

		if (Peb->Ldr > 0)
		{
			// ��Ϊpeb���û������ݣ������޷�����
			ProbeForRead((PVOID)Peb->Ldr, sizeof(PVOID), sizeof(UINT8));

			for (PLIST_ENTRY TravelListEntry = Peb->Ldr->InLoadOrderModuleList.Flink;
				TravelListEntry != &Peb->Ldr->InLoadOrderModuleList;
				TravelListEntry = (PLIST_ENTRY)TravelListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = NULL;
				LdrDataTableEntry = CONTAINING_RECORD(TravelListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

				if ((PUINT8)LdrDataTableEntry > 0 && MmIsAddressValid(LdrDataTableEntry))
				{
					// ����
					if (!APIsModuleInList((UINT_PTR)LdrDataTableEntry->DllBase, LdrDataTableEntry->SizeOfImage, pmi, ModuleCount))
					{
						if (ModuleCount > pmi->NumberOfModules)	// Ring3���Ĵ� �ͼ�����
						{
							pmi->ModuleEntry[pmi->NumberOfModules].BaseAddress = (UINT_PTR)LdrDataTableEntry->DllBase;
							pmi->ModuleEntry[pmi->NumberOfModules].SizeOfImage = LdrDataTableEntry->SizeOfImage;
							StringCchCopyW(pmi->ModuleEntry[pmi->NumberOfModules].wzFilePath, LdrDataTableEntry->FullDllName.Length, LdrDataTableEntry->FullDllName.Buffer);
						}
						pmi->NumberOfModules++;
					}
				}
			}

			// ö����������
//			APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InLoadOrderModuleList), lt_InLoadOrderModuleList, pmi, ModuleCount);
//			APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InMemoryOrderModuleList), lt_InMemoryOrderModuleList, pmi, ModuleCount);
//			APFillProcessModuleInfoByTravelLdr((PLIST_ENTRY)&(LdrData->InInitializationOrderModuleList), lt_InInitializationOrderModuleList, pmi, ModuleCount);
			// ö�ٵ��˶���
			if (pmi->NumberOfModules)
			{
				Status = STATUS_SUCCESS;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Catch Exception\r\n");
		Status = STATUS_UNSUCCESSFUL;
	}

	if (bAttach)
	{
		KeUnstackDetachProcess(&ApcState);
		bAttach = FALSE;
	}

	return Status;
}


/************************************************************************
*  Name : APEnumProcessModule
*  Param: ProcessId			      ����Id
*  Param: OutputBuffer            ring3�ڴ�
*  Param: OutputLength
*  Ret  : NTSTATUS
*  ö�ٽ���ģ��
************************************************************************/
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

/*			Status = APEnumProcessModuleByZwQueryVirtualMemory(EProcess, pmi, ModuleCount);
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
			else
			{*/
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
			//}

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






