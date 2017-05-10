#include "ProcessCore.h"


extern DYNAMIC_DATA	g_DynamicData;

typedef
UINT_PTR
(*pfnObGetObjectType)(PVOID Object);


/************************************************************************
*  Name : APGetPsIdleProcess
*  Param: void
*  Ret  : UINT_PTR			PsIdleProcess
*  ���System Idle Process ��EProcess��ַ
************************************************************************/
UINT_PTR
APGetPsIdleProcess()
{
	UINT_PTR PsIdleProcess = 0;
	UINT_PTR PsInitialSystemProcessAddress = (UINT_PTR)&PsInitialSystemProcess;

	if (PsInitialSystemProcessAddress && MmIsAddressValid((PVOID)((PUINT8)PsInitialSystemProcessAddress + 0xA0)))
	{
		PsIdleProcess = *(PUINT_PTR)((PUINT8)PsInitialSystemProcessAddress + 0xA0);
		if (PsIdleProcess <= 0xffff)
		{
			PsIdleProcess = *(PUINT_PTR)((PUINT8)PsInitialSystemProcessAddress + 0xB0);
		}
	}
	return PsIdleProcess;
}


/************************************************************************
*  Name : APGetObjectType
*  Param: Object				�������׵�ַ
*  Ret  : UINT_PTR				���������ͣ�
*  x64��ͨ��ObGetObjectType��ö�������/ x86��ͨ��ObjectHeader->TypeIndex��ö�������
************************************************************************/
UINT_PTR
APGetObjectType(IN PVOID Object)
{
	UINT_PTR	ObjectType = 0;

	pfnObGetObjectType	ObGetObjectTypeAddress = NULL;
	if (MmIsAddressValid && Object && MmIsAddressValid(Object))
	{
		APGetNtosExportVariableAddress(L"ObGetObjectType", (PVOID*)&ObGetObjectTypeAddress);
		if (ObGetObjectTypeAddress)
		{
			ObjectType = ObGetObjectTypeAddress(Object);
		}
	}

	return ObjectType;
}


/************************************************************************
*  Name : APIsActiveProcess
*  Param: Object				�������׵�ַ
*  Ret  : BOOLEAN
*  ͨ���Ƿ���ھ�����жϽ����Ƿ��� TRUE���/ FALSE������
************************************************************************/
BOOLEAN
APIsActiveProcess(IN PEPROCESS EProcess)
{
	BOOLEAN bOk = FALSE;

	if (EProcess &&
		MmIsAddressValid(EProcess) &&
		MmIsAddressValid((PVOID)((PUINT8)EProcess + g_DynamicData.ObjectTable)))
	{
		PVOID ObjectTable = *(PVOID*)((PUINT8)EProcess + g_DynamicData.ObjectTable);

		if (ObjectTable &&
			MmIsAddressValid(ObjectTable))
		{
			bOk = TRUE;
		}
	}

	return bOk;
}


/************************************************************************
*  Name : IsValidProcess
*  Param: EProcess				���������
*  Ret  : BOOLEAN
*  �ж��Ƿ�Ϊ�Ϸ����� TRUE�Ϸ�/ FALSE�Ƿ�
************************************************************************/
BOOLEAN
APIsValidProcess(IN PEPROCESS EProcess)
{
	UINT_PTR    ObjectType;
	BOOLEAN		bOk = FALSE;

	UINT_PTR	ProcessType = ((UINT_PTR)*PsProcessType);		// ����ȫ�ֱ��������̶�������

	if (ProcessType && EProcess && MmIsAddressValid((PVOID)(EProcess)))
	{
		ObjectType = APGetObjectType((PVOID)EProcess);   //*PsProcessType 

		if (ObjectType &&
			ObjectType == ProcessType &&
			APIsActiveProcess(EProcess))
		{
			bOk = TRUE;
		}
	}

	return bOk;
}


/************************************************************************
*  Name : APGetProcessCount
*  Param: OutputBuffer			Ring3����������Ž��̸���
*  Ret  : NTSTATUS
*  ��õ�ǰ���̸���
************************************************************************/
NTSTATUS
APGetProcessNum(OUT PVOID OutputBuffer)
{
	UINT32 ProcessCount = 0;

	for (UINT32 ProcessId = 0; ProcessId < MAX_PROCESS_COUNT; ProcessId += 4)
	{
		NTSTATUS  Status = STATUS_UNSUCCESSFUL;
		PEPROCESS EProcess = NULL;

		if (ProcessId == 0)
		{
			ProcessCount++;
			continue;
		}

		Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
		if (NT_SUCCESS(Status))
		{
			if (APIsValidProcess(EProcess))
			{
				ProcessCount++;
			}

			ObDereferenceObject(EProcess);   // ������
		}
	}

	*(PUINT32)OutputBuffer = ProcessCount;

	return STATUS_SUCCESS;
}


/************************************************************************
*  Name : APGetParentProcessId
*  Param: EProcess			   ������ṹ
*  Ret  : UINT_PTR
*  ��ø�����Id
************************************************************************/
UINT_PTR
APGetParentProcessId(IN PEPROCESS EProcess)
{
	if (MmIsAddressValid &&
		EProcess &&
		MmIsAddressValid(EProcess) &&
		MmIsAddressValid((PVOID)((PUINT8)EProcess + g_DynamicData.ObjectTable)))
	{
		UINT_PTR  ParentProcessId = 0;

		ParentProcessId = *(PUINT_PTR)((PUINT8)EProcess + g_DynamicData.InheritedFromUniqueProcessId);

		return ParentProcessId;
	}

	return 0;
}


/************************************************************************
*  Name : GetProcessFullPathByProcessId
*  Param: ProcessId					����Id				��IN��
*  Param: ProcessFullPath			��������·��		��OUT��
*  Ret  : NTSTATUS
*  ͨ��FileObject��ý�������·��
************************************************************************/
NTSTATUS
APGetProcessFullPath(IN PEPROCESS EProcess, OUT PWCHAR ProcessFullPath)
{
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;

	if (APIsValidProcess(EProcess))
	{
		/*
		3: kd> dt _EPROCESS fffffa801ac21060
		nt!_EPROCESS
		+0x000 Pcb              : _KPROCESS
		......
		+0x268 SectionObject    : 0xfffff8a0`01bf2a50 Void
		*/
		PSECTION_OBJECT SectionObject = (PSECTION_OBJECT)(*(PUINT_PTR)((PUINT8)EProcess + g_DynamicData.SectionObject));

		if (SectionObject && MmIsAddressValid(SectionObject))
		{
			/*
			3: kd> dt _SECTION_OBJECT 0xfffff8a0`01bf2a50
			nt!_SECTION_OBJECT
			+0x000 StartingVa       : (null)
			......
			+0x028 Segment          : 0xfffff8a0`01deb000 _SEGMENT_OBJECT
			*/
			PSEGMENT Segment = SectionObject->Segment;

			if (Segment && MmIsAddressValid(Segment))
			{
				/*
				3: kd> dt _SEGMENT 0xfffff8a0`01deb000
				nt!_SEGMENT
				+0x000 ControlArea      : 0xfffffa80`1ac18800 _CONTROL_AREA
				......
				*/
				PCONTROL_AREA ControlArea = Segment->ControlArea;

				if (ControlArea && MmIsAddressValid(ControlArea))
				{
					/*
					3: kd> dt _CONTROL_AREA 0xfffffa80`1ac18800
					nt!_CONTROL_AREA
					+0x000 Segment          : 0xfffff8a0`01deb000 _SEGMENT
					......
					+0x040 FilePointer      : _EX_FAST_REF

					3: kd> dq 0xfffffa80`1ac18800+40
					fffffa80`1ac18840  fffffa80`1ac18d44 00000000`00000000
					*/
					PFILE_OBJECT FileObject = (PFILE_OBJECT)((UINT_PTR)ControlArea->FilePointer & 0xFFFFFFFFFFFFFFF8);

					if (FileObject && MmIsAddressValid(FileObject))
					{
						POBJECT_NAME_INFORMATION    oni = NULL;
						/*
						3: kd> dt _FILE_OBJECT fffffa80`1ac18d40
						nt!_FILE_OBJECT
						+0x000 Type             : 0n5
						......
						+0x058 FileName         : _UNICODE_STRING "\Windows\explorer.exe"
						*/
						Status = IoQueryFileDosDeviceName(FileObject, &oni);
						if (NT_SUCCESS(Status))
						{
							UINT32 ProcessFullPathLength = 0;

							if (oni->Name.Length >= MAX_PATH)
							{
								ProcessFullPathLength = MAX_PATH - 1;
							}
							else
							{
								ProcessFullPathLength = oni->Name.Length;
							}

							//RtlCopyMemory(ProcessFullPath, oni->Name.Buffer, ProcessFullPathLength);

							RtlStringCchCopyW(ProcessFullPath, ProcessFullPathLength + 1, oni->Name.Buffer);

							Status = STATUS_SUCCESS;

							DbgPrint("%S\r\n", ProcessFullPath);
						}
					}
				}
			}
		}
	}

	return Status;
}


/************************************************************************
*  Name : APEnumProcessInfo
*  Param: OutputBuffer			Ring3Buffer      ��OUT��
*  Param: OutputLength			Ring3BufferLength��IN��
*  Ret  : NTSTATUS
*  ͨ��FileObject��ý�������·��
************************************************************************/
NTSTATUS
APEnumProcessInfo(OUT PVOID OutputBuffer, IN UINT32 OutputLength)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;

	PEPROCESS EProcess = NULL;
	PPROCESS_INFORMATION pi = (PPROCESS_INFORMATION)OutputBuffer;
	UINT32 ProcessesCount = (OutputLength - sizeof(PROCESS_INFORMATION)) / sizeof(PROCESS_ENTRY_INFORMATION);

	// ͨ������id��ö�ٽ���
	for (UINT32 ProcessId = 0; ProcessId < MAX_PROCESS_COUNT; ProcessId += 4)
	{
		if (ProcessesCount > pi->NumberOfProcesses)
		{
			if (ProcessId == 0)
			{
				// Idle
				pi->ProcessEntry[pi->NumberOfProcesses].ProcessId = 0;
				pi->ProcessEntry[pi->NumberOfProcesses].EProcess = (UINT_PTR)APGetPsIdleProcess();   // ȫ�ֵ���
				pi->ProcessEntry[pi->NumberOfProcesses].ParentProcessId = 0;

				DbgPrint("Process Id:%d\r\n", pi->ProcessEntry[pi->NumberOfProcesses].ProcessId);
				DbgPrint("EProcess:%p\r\n", pi->ProcessEntry[pi->NumberOfProcesses].EProcess);
				DbgPrint("EProcess:%d\r\n", pi->ProcessEntry[pi->NumberOfProcesses].ParentProcessId);

				pi->NumberOfProcesses++;
			}
			else
			{
				// ��������
				Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
				if (NT_SUCCESS(Status) && APIsValidProcess(EProcess))
				{
					pi->ProcessEntry[pi->NumberOfProcesses].ProcessId = ProcessId;
					pi->ProcessEntry[pi->NumberOfProcesses].EProcess = (UINT_PTR)EProcess;
					pi->ProcessEntry[pi->NumberOfProcesses].ParentProcessId = (UINT32)APGetParentProcessId(EProcess);
					APGetProcessFullPath(EProcess, pi->ProcessEntry[pi->NumberOfProcesses].wzFilePath);

					DbgPrint("Process Id:%d\r\n", pi->ProcessEntry[pi->NumberOfProcesses].ProcessId);
					DbgPrint("EProcess:%p\r\n", pi->ProcessEntry[pi->NumberOfProcesses].EProcess);
					DbgPrint("EProcess:%d\r\n", pi->ProcessEntry[pi->NumberOfProcesses].ParentProcessId);
					DbgPrint("EProcess:%s\r\n", pi->ProcessEntry[pi->NumberOfProcesses].wzFilePath);

					pi->NumberOfProcesses++;

					ObDereferenceObject(EProcess);
				}
			}
			Status = STATUS_SUCCESS;
		}
		else
		{
			DbgPrint("Not Enough Ring3 Memory\r\n");
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}
	}

	return Status;
}


/************************************************************************
*  Name : APTerminateProcess
*  Param: ProcessId
*  Ret  : NTSTATUS
*  ��������
************************************************************************/
NTSTATUS
APTerminateProcess(IN UINT32 ProcessId)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;
	
	if (ProcessId <= 4)
	{
		Status = STATUS_ACCESS_DENIED;
	}
	else
	{
		PEPROCESS EProcess = NULL;

		Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
		if (NT_SUCCESS(Status) && APIsValidProcess(EProcess))
		{
			Status = APTerminateProcessByIterateThreadListHead(EProcess);

			ObDereferenceObject(EProcess);
		}
	}

	return Status;
}


/************************************************************************
*  Name : APGetGuiProcess
*  Param: void
*  Ret  : PEPROCESS
*  �������̣����csrss Eprocess
************************************************************************/
PEPROCESS
APGetGuiProcess()
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;

	for (UINT32 i = 4; i < MAX_PROCESS_COUNT; i += 4)
	{
		Status = PsLookupProcessByProcessId((HANDLE)i, &EProcess);
		if (NT_SUCCESS(Status) && EProcess && MmIsAddressValid((PVOID)EProcess))
		{
			CHAR *ProcessImageName = (CHAR*)PsGetProcessImageFileName(EProcess);
			
			ObDereferenceObject(EProcess);

			if (!_strnicmp("csrss.exe", ProcessImageName, strlen("csrss.exe")))
			{
				DbgPrint("EProcess = %p ProcessId = %ld ImageName = %s\r\n", EProcess, PsGetProcessId(EProcess), PsGetProcessImageFileName(EProcess));
				return EProcess;
			}
		}
	}

	return NULL;
}