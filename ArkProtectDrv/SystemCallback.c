#include "SystemCallback.h"

extern DYNAMIC_DATA g_DynamicData;

//////////////////////////////////////////////////////////////////////////
//
UINT_PTR
APGetPspCreateProcessNotifyRoutineAddress()
{
	UINT_PTR PspCreateProcessNotifyRoutine = 0;
	UINT_PTR PsSetCreateProcessNotifyRoutine = 0;
	UINT_PTR PspSetCreateProcessNotifyRoutine = 0;

	PUINT8	StartSearchAddress = 0, EndSearchAddress = 0;
	PUINT8	i = NULL;
	UINT8   v1 = 0, v2 = 0, v3 = 0;
	INT32   iOffset = 0;    // ע�������ƫ�ƿ����ɸ� ���ܶ�UINT��

	APGetNtosExportVariableAddress(L"PsSetCreateProcessNotifyRoutine", (PVOID*)&PsSetCreateProcessNotifyRoutine);
	DbgPrint("%p\r\n", PsSetCreateThreadNotifyRoutine);

	/*
	Win7x64
	kd> u PsSetCreateProcessNotifyRoutine
	nt!PsSetCreateProcessNotifyRoutine:
	fffff800`042cd400 4533c0          xor     r8d,r8d
	fffff800`042cd403 e9e8fdffff      jmp     nt!PspSetCreateProcessNotifyRoutine (fffff800`042cd1f0)
	
	Win7x86
	1: kd> u PsSetCreateProcessNotifyRoutine
	nt!PsSetCreateProcessNotifyRoutine:
	83fd2899 8bff            mov     edi,edi
	......
	83fac8a3 ff7508          push    dword ptr [ebp+8]
	83fac8a6 e809000000      call    nt!PspSetCreateProcessNotifyRoutine (83fac8b4)
	83fac8ab 5d              pop     ebp

	����PsSetCreateProcessNotifyRoutine��һ�����壬��ʵ������PspSetCreateProcessNotifyRoutine
	win7x64:PsSetCreateProcessNotifyRoutine��4�ֽ�����ת��ƫ�ƣ��ȼ����PspSetCreateProcessNotifyRoutine������ַ
	win7x86:������ֱ�ӵ���PspSetCreateProcessNotifyRoutine��Ӳ����Ѱ��
	*/

	if (PsSetCreateThreadNotifyRoutine)
	{
#ifdef _WIN64
		PspSetCreateProcessNotifyRoutine = (*(INT32*)(PsSetCreateProcessNotifyRoutine + 4) + PsSetCreateProcessNotifyRoutine + 3);
#else
		StartSearchAddress = (PUINT8)PsSetCreateProcessNotifyRoutine;
		EndSearchAddress = StartSearchAddress + 0x500;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 5))
			{
				v1 = *i;
				v2 = *(i + 5);
				if (v1 == 0xe8 && v2 == 0x5d)		//  call offset pop     ebp
				{
					RtlCopyMemory(&iOffset, i + 1, 4);
					PspSetCreateProcessNotifyRoutine = (UINT_PTR)(iOffset + (UINT32)i + 5);;
					break;
				}
			}
		}
#endif // _WIN64
	}

	// �� PsSetCreateThreadNotifyRoutine ��ʹ���� PspCreateThreadNotifyRoutine Ӳ�����ƫ�ƻ��
	/*
	Win7 64:
	kd> u PspSetCreateProcessNotifyRoutine l 20
	nt!PspSetCreateProcessNotifyRoutine:
	fffff800`042cd1f0 48895c2408      mov     qword ptr [rsp+8],rbx
	......
	fffff800`042cd236 4c8d3563bdd6ff  lea     r14,[nt!PspCreateProcessNotifyRoutine (fffff800`04038fa0)]
	fffff800`042cd23d 418bc4          mov     eax,r12d

	Win7 32:
	3: kd> u 83fac8b4 l 10
	nt!PspSetCreateProcessNotifyRoutine:
	83fac8b4 8bff            mov     edi,edi
	......
	83fac8d6 c7450ca0a9f583  mov     dword ptr [ebp+0Ch],offset nt!PspCreateProcessNotifyRoutine (83f5a9a0)
	*/

	if (PspSetCreateProcessNotifyRoutine)
	{
		StartSearchAddress = (PUINT8)PspSetCreateProcessNotifyRoutine;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
#ifdef _WIN64
				if (v1 == 0x4c && v2 == 0x8d && v3 == 0x35)		// Ӳ����  lea r14
				{
					RtlCopyMemory(&iOffset, i + 3, 4);
					PspCreateProcessNotifyRoutine = (UINT_PTR)(iOffset + (UINT64)i + 7);
					break;
				}
#else
				if (v1 == 0xc7 && v2 == 0x45 && v3 == 0x0c)		// mov     dword ptr [ebp+0Ch]
				{
					RtlCopyMemory(&PspCreateProcessNotifyRoutine, i + 3, 4);
					break;
				}
#endif // _WIN64

			}
		}
	}
	return PspCreateProcessNotifyRoutine;
}


BOOLEAN
APGetCreateProcessCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR PspCreateProcessNotifyRoutine = APGetPspCreateProcessNotifyRoutineAddress();

	DbgPrint("%p\r\n", PspCreateProcessNotifyRoutine);

	if (!PspCreateProcessNotifyRoutine)
	{
		DbgPrint("PspCreateProcessNotifyRoutineAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		UINT32 i = 0;
		for (i = 0; i < PSP_MAX_CREATE_PROCESS_NOTIFY; i++)
		{
			UINT_PTR NotifyItem = 0;		// PspCreateThreadNotifyRoutine����ÿһ���Ա
			UINT_PTR CallbackAddress = 0;	// ��ʵ�Ļص����̵�ַ

			if (MmIsAddressValid((PVOID)(PspCreateProcessNotifyRoutine + i * sizeof(UINT_PTR))))
			{
				NotifyItem = *(PUINT_PTR)(PspCreateProcessNotifyRoutine + i * sizeof(UINT_PTR));

				if (NotifyItem > g_DynamicData.MinKernelSpaceAddress &&
					MmIsAddressValid((PVOID)(NotifyItem & ~7)))
				{
#ifdef _WIN64
					CallbackAddress = *(PUINT_PTR)(NotifyItem & ~7);
#else
					CallbackAddress = *(PUINT_PTR)((NotifyItem & ~7) + sizeof(UINT32));
#endif // _WIN64
					if (CallbackAddress && MmIsAddressValid((PVOID)CallbackAddress))
					{
						if (CallbackCount > sci->NumberOfCallbacks)
						{
							sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyCreateProcess;
							sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = CallbackAddress;
							sci->CallbackEntry[sci->NumberOfCallbacks].Description = NotifyItem;
						}
						sci->NumberOfCallbacks++;
					}
				}
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UINT_PTR
APGetPspCreateThreadNotifyRoutineAddress()
{
	// �� PsSetCreateThreadNotifyRoutine ��ʹ���� PspCreateThreadNotifyRoutine Ӳ�����ƫ�ƻ��
	/*
	Win7x64:
	2: kd> u PsSetCreateThreadNotifyRoutine l 10
	nt!PsSetCreateThreadNotifyRoutine:
	fffff800`0428cbf0 48895c2408      mov     qword ptr [rsp+8],rbx
	......
	fffff800`0428cc12 488d0d67c1d9ff  lea     rcx,[nt!PspCreateThreadNotifyRoutine (fffff800`04028d80)]

	Win7x86:
	0: kd> u PsSetCreateThreadNotifyRoutine l 20
	nt!PsSetCreateThreadNotifyRoutine:
	840f78e7 8bff            mov     edi,edi
	......
	840f7906 56              push    esi
	840f7907 be80a8f583      mov     esi,offset nt!PspCreateThreadNotifyRoutine (83f5a880)
	840f790c 6a00            push    0
	*/

	UINT_PTR PspCreateThreadNotifyRoutine = 0;
	UINT_PTR PsSetCreateThreadNotifyRoutine = 0;

	APGetNtosExportVariableAddress(L"PsSetCreateThreadNotifyRoutine", (PVOID*)&PsSetCreateThreadNotifyRoutine);
	DbgPrint("%p\r\n", PsSetCreateThreadNotifyRoutine);

	if (PsSetCreateThreadNotifyRoutine)
	{
		PUINT8	StartSearchAddress = (PUINT8)PsSetCreateThreadNotifyRoutine;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;    // ע�������ƫ�ƿ����ɸ� ���ܶ�UINT��

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
#ifdef _WIN64
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					RtlCopyMemory(&iOffset, i + 3, 4);
					PspCreateThreadNotifyRoutine = (UINT_PTR)(iOffset + (UINT64)i + 7);
					break;
				}
			}
#else
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 6))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 6);
				if (v1 == 0x56 && v2 == 0xbe && v3 == 0x6a)		// Ӳ����  lea rcx
				{
					RtlCopyMemory(&PspCreateThreadNotifyRoutine, i + 2, 4);
					break;
				}
			}
#endif // _WIN64
		}
	}
	return PspCreateThreadNotifyRoutine;
}


BOOLEAN
APGetCreateThreadCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR PspCreateThreadNotifyRoutine = APGetPspCreateThreadNotifyRoutineAddress();

	DbgPrint("%p\r\n", PspCreateThreadNotifyRoutine);

	if (!PspCreateThreadNotifyRoutine)
	{
		DbgPrint("PspCreateThreadNotifyRoutineAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		UINT32 i = 0;
		for (i = 0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i++)
		{
			UINT_PTR NotifyItem = 0;		// PspCreateThreadNotifyRoutine����ÿһ���Ա
			UINT_PTR CallbackAddress = 0;	// ��ʵ�Ļص����̵�ַ

			if (MmIsAddressValid((PVOID)(PspCreateThreadNotifyRoutine + i * sizeof(UINT_PTR))))
			{
				NotifyItem = *(PUINT_PTR)(PspCreateThreadNotifyRoutine + i * sizeof(UINT_PTR));

				if (NotifyItem > g_DynamicData.MinKernelSpaceAddress &&
					MmIsAddressValid((PVOID)(NotifyItem & ~7)))
				{
#ifdef _WIN64
					CallbackAddress = *(PUINT_PTR)(NotifyItem & ~7);
#else
					CallbackAddress = *(PUINT_PTR)((NotifyItem & ~7) + sizeof(UINT32));
#endif // _WIN64

					if (CallbackAddress && MmIsAddressValid((PVOID)CallbackAddress))
					{
						if (CallbackCount > sci->NumberOfCallbacks)
						{
							sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyCreateThread;
							sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = CallbackAddress;
							sci->CallbackEntry[sci->NumberOfCallbacks].Description = NotifyItem;
						}
						sci->NumberOfCallbacks++;
					}
				}
			}
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
UINT_PTR
APGetPspLoadImageNotifyRoutineAddress()
{
	// �� PsSetLoadImageNotifyRoutine ��ʹ���� PspLoadImageNotifyRoutine Ӳ�����ƫ�ƻ��
	/*
	Win7 x64:
	0: kd> u PsSetLoadImageNotifyRoutine l 10
	nt!PsSetLoadImageNotifyRoutine:
	fffff800`0429cb70 48895c2408      mov     qword ptr [rsp+8],rbx
	......
	fffff800`0429cb92 488d0d87c1d9ff  lea     rcx,[nt!PspLoadImageNotifyRoutine (fffff800`04038d20)]
	fffff800`0429cb99 4533c0          xor     r8d,r8d

	Win7 x86:
	0: kd> u PsSetLoadImageNotifyRoutine l 10
	nt!PsSetLoadImageNotifyRoutine:
	83f889ca 8bff            mov     edi,edi
	......
	83f889e1 7425            je      nt!PsSetLoadImageNotifyRoutine+0x3e (83f88a08)
	83f889e3 be40a8f583      mov     esi,offset nt!PspLoadImageNotifyRoutine (83f5a840)
	83f889e8 6a00            push    0
	*/

	UINT_PTR PspLoadImageNotifyRoutine = 0;
	UINT_PTR PsSetLoadImageNotifyRoutine = 0;

	APGetNtosExportVariableAddress(L"PsSetLoadImageNotifyRoutine", (PVOID*)&PsSetLoadImageNotifyRoutine);
	DbgPrint("%p\r\n", PsSetLoadImageNotifyRoutine);

	if (PsSetLoadImageNotifyRoutine)
	{
		PUINT8	StartSearchAddress = (PUINT8)PsSetLoadImageNotifyRoutine;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;    // ע�������ƫ�ƿ����ɸ� ���ܶ�UINT��

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
#ifdef _WIN64
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					RtlCopyMemory(&iOffset, i + 3, 4);
					PspLoadImageNotifyRoutine = (UINT_PTR)(iOffset + (UINT64)i + 7);
					break;
				}
			}
#else
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 5) && MmIsAddressValid(i + 6))
			{
				v1 = *i;
				v2 = *(i + 5);
				v3 = *(i + 6);
				if (v1 == 0xbe && v2 == 0x6a && v3 == 0x00)		// Ӳ����  lea rcx
				{
					RtlCopyMemory(&PspLoadImageNotifyRoutine, i + 1, 4);
					break;
				}
			}
#endif // _WIN64

		}
	}
	return PspLoadImageNotifyRoutine;
}

BOOLEAN
APGetLoadImageCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR PspLoadImageNotifyRoutine = APGetPspLoadImageNotifyRoutineAddress();

	DbgPrint("%p\r\n", PspLoadImageNotifyRoutine);

	if (!PspLoadImageNotifyRoutine)
	{
		DbgPrint("PspLoadImageNotifyRoutineAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		UINT32 i = 0;
		for (i = 0; i < PSP_MAX_LOAD_IMAGE_NOTIFY; i++)		// 64λ�����СΪ64
		{
			UINT_PTR NotifyItem = 0;		// PspLoadImageNotifyRoutine����ÿһ���Ա
			UINT_PTR CallbackAddress = 0;	// ��ʵ�Ļص����̵�ַ

			if (MmIsAddressValid((PVOID)(PspLoadImageNotifyRoutine + i * sizeof(UINT_PTR))))
			{
				NotifyItem = *(PUINT_PTR)(PspLoadImageNotifyRoutine + i * sizeof(UINT_PTR));

				if (NotifyItem > g_DynamicData.MinKernelSpaceAddress &&
					MmIsAddressValid((PVOID)(NotifyItem & ~7)))
				{
#ifdef _WIN64
					CallbackAddress = *(PUINT_PTR)(NotifyItem & ~7);
#else
					CallbackAddress = *(PUINT_PTR)((NotifyItem & ~7) + sizeof(UINT32));
#endif // _WIN64

					if (CallbackAddress && MmIsAddressValid((PVOID)CallbackAddress))
					{
						if (CallbackCount > sci->NumberOfCallbacks)
						{
							sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyLoadImage;
							sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = CallbackAddress;
							sci->CallbackEntry[sci->NumberOfCallbacks].Description = NotifyItem;
						}
						sci->NumberOfCallbacks++;
					}
				}
			}
		}
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Win7֮�� ����ListEntry�ṹ xp֮ǰ ����Vector����ṹ
UINT_PTR
APGetCallbackListHeadAddress()
{
	/*
	0: kd> u CmUnRegisterCallback l 30
	nt!CmUnRegisterCallback:
	fffff800`042c67d0 48894c2408      mov     qword ptr [rsp+8],rcx
	......
	fffff800`042c689e 488d0dcb90dcff  lea     rcx,[nt!CallbackListHead (fffff800`0408f970)]

	*/

	UINT_PTR CmUnRegisterCallback = 0;

	APGetNtosExportVariableAddress(L"CmUnRegisterCallback", (PVOID*)&CmUnRegisterCallback);
	DbgPrint("%p\r\n", CmUnRegisterCallback);

	if (CmUnRegisterCallback != 0)
	{
		PUINT8	StartSearchAddress = (PUINT8)CmUnRegisterCallback;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL, j = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					// ǰ��Ҳ������ lea rcx,������Ҫ���һ���ж�
					j = i - 5;
					if (MmIsAddressValid(j) && MmIsAddressValid(j + 1) && MmIsAddressValid(j + 2))
					{
						v1 = *j;
						v2 = *(j + 1);
						v3 = *(j + 2);
						if (v1 == 0x48 && v2 == 0x8d && v3 == 0x54)		// Ӳ����  lea rdx
						{
							RtlCopyMemory(&iOffset, i + 3, 4);
							return (UINT_PTR)(iOffset + (UINT64)i + 7);
						}
					}
				}
			}
		}
	}

	return 0;
}

BOOLEAN
APGetRegisterCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR CallbackListHead = APGetCallbackListHeadAddress();

	DbgPrint("%p\r\n", CallbackListHead);

	if (!CallbackListHead)
	{
		DbgPrint("CallbackListHeadAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		PCM_NOTIFY_ENTRY	Notify = NULL;
		PLIST_ENTRY			NotifyListEntry = (PLIST_ENTRY)(*(PUINT_PTR)CallbackListHead);	// Flink

		/*
		WinDbg����
		3: kd> dt _list_entry fffff800`0408f970
		nt!_LIST_ENTRY
		[ 0xfffff800`0408f970 - 0xfffff800`0408f970 ]
		+0x000 Flink            : 0xfffff800`0408f970 _LIST_ENTRY [ 0xfffff800`0408f970 - 0xfffff800`0408f970 ]
		+0x008 Blink            : 0xfffff800`0408f970 _LIST_ENTRY [ 0xfffff800`0408f970 - 0xfffff800`0408f970 ]

		3: kd> dq fffff800`0408f970
		fffff800`0408f970  fffff800`0408f970 fffff800`0408f970
		fffff800`0408f980  00000000`00000000 00000000`00000000
		fffff800`0408f990  00000000`00000000 00000000`00000000

		Ŀǰϵͳû�����ֻص�
		*/

		do
		{
			Notify = (PCM_NOTIFY_ENTRY)NotifyListEntry;
			if (MmIsAddressValid(Notify))
			{
				if (MmIsAddressValid((PVOID)(Notify->Function)) && Notify->Function > g_DynamicData.MinKernelSpaceAddress)
				{
					if (CallbackCount > sci->NumberOfCallbacks)
					{
						sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyCmpCallBack;
						sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = (UINT_PTR)Notify->Function;
						sci->CallbackEntry[sci->NumberOfCallbacks].Description = (UINT_PTR)Notify->Cookie.QuadPart;
					}
					sci->NumberOfCallbacks++;
				}
			}
			NotifyListEntry = NotifyListEntry->Flink;
		} while (NotifyListEntry != ((PLIST_ENTRY)(*(PUINT_PTR)CallbackListHead)));
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UINT_PTR
APGetKeBugCheckCallbackListHeadAddress()
{
	/*
	0: kd> u KeRegisterBugCheckCallback l 50
	nt!KeRegisterBugCheckCallback:
	fffff800`03f390b0 48895c2420      mov     qword ptr [rsp+20h],rbx
	......
	fffff800`03f391bc 488d0d7d111500  lea     rcx,[nt!KeBugCheckCallbackListHead (fffff800`0408a340)]
	*/

	UINT_PTR KeRegisterBugCheckCallback = 0;

	APGetNtosExportVariableAddress(L"KeRegisterBugCheckCallback", (PVOID*)&KeRegisterBugCheckCallback);
	DbgPrint("%p\r\n", KeRegisterBugCheckCallback);

	if (KeRegisterBugCheckCallback != 0)
	{
		PUINT8	StartSearchAddress = (PUINT8)KeRegisterBugCheckCallback;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL, j = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;    // ע�������ƫ�ƿ����ɸ� ���ܶ�UINT��
		UINT64  VariableAddress = 0;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					j = i - 3;
					if (MmIsAddressValid(j) && MmIsAddressValid(j + 1) && MmIsAddressValid(j + 2))
					{
						v1 = *j;
						v2 = *(j + 1);
						v3 = *(j + 2);
						if (v1 == 0x48 && v2 == 0x03 && v3 == 0xc1)		// Ӳ����  add rax, rcx
						{
							RtlCopyMemory(&iOffset, i + 3, 4);
							return (UINT_PTR)(iOffset + (UINT64)i + 7);
						}
					}
				}
			}
		}
	}
	return 0;
}

BOOLEAN
APGetBugCheckCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR KeBugCheckCallbackListHead = APGetKeBugCheckCallbackListHeadAddress();

	DbgPrint("%p\r\n", KeBugCheckCallbackListHead);

	if (!KeBugCheckCallbackListHead)
	{
		DbgPrint("KeBugCheckCallbackListHeadAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		/*
		2: kd> dt _list_entry fffff800`0407a340
		nt!_LIST_ENTRY
		[ 0xfffffa80`198d7ea0 - 0xfffff800`04413400 ]
		+0x000 Flink            : 0xfffffa80`198d7ea0 _LIST_ENTRY [ 0xfffffa80`1989eea0 - 0xfffff800`0407a340 ]
		+0x008 Blink            : 0xfffff800`04413400 _LIST_ENTRY [ 0xfffff800`0407a340 - 0xfffffa80`1976aea0 ]

		2: kd> dq 0xfffffa80`198d7ea0
		fffffa80`198d7ea0  fffffa80`1989eea0 fffff800`0407a340
		fffffa80`198d7eb0  fffff880`014f1b00 fffffa80`198d71a0

		2: kd> u fffff880`014f1b00
		fffff880`014f1b00 4883ec28        sub     rsp,28h		; ����ջ���Ǻ����ı�־
		fffff880`014f1b04 81faa0160000    cmp     edx,16A0h
		fffff880`014f1b0a 754b            jne     fffff880`014f1b57
		fffff880`014f1b0c 4c8b81f80c0000  mov     r8,qword ptr [rcx+0CF8h]
		fffff880`014f1b13 0fba697c18      bts     dword ptr [rcx+7Ch],18h
		fffff880`014f1b18 4d85c0          test    r8,r8
		fffff880`014f1b1b 743a            je      fffff880`014f1b57
		fffff880`014f1b1d 8b818c140000    mov     eax,dword ptr [rcx+148Ch]

		*/

		UINT_PTR     Dispatch = 0;
/*		PLIST_ENTRY	 DispatchListEntry = ((PLIST_ENTRY)KeBugCheckCallbackListHead)->Flink;	// Flink

		do
		{
			Dispatch = *(PUINT_PTR)((PUINT8)DispatchListEntry + sizeof(LIST_ENTRY));	// ��ListEntry
			if (Dispatch && MmIsAddressValid((PVOID)Dispatch))
			{
				UINT_PTR CurrentCount = sci->NumberOfCallbacks;
				if (NumberOfCallbacks > CurrentCount)
				{
					sci->Callbacks[CurrentCount].Type = NotifyKeBugCheck;
					sci->Callbacks[CurrentCount].CallbackAddress = Dispatch;
					sci->Callbacks[CurrentCount].Description = DispatchListEntry;
				}
				sci->NumberOfCallbacks++;
			}
			DispatchListEntry = DispatchListEntry->Flink;
		} while (DispatchListEntry != KeBugCheckCallbackListHead);
		*/

		for (PLIST_ENTRY TravelListEntry = ((PLIST_ENTRY)KeBugCheckCallbackListHead)->Flink;
			TravelListEntry != (PLIST_ENTRY)KeBugCheckCallbackListHead;
			TravelListEntry = TravelListEntry->Flink)
		{
			Dispatch = *(PUINT_PTR)((PUINT8)TravelListEntry + sizeof(LIST_ENTRY));	// ��ListEntry,������Ǻ���

			if (Dispatch && MmIsAddressValid((PVOID)Dispatch))
			{
				if (CallbackCount > sci->NumberOfCallbacks)
				{
					sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyKeBugCheck;
					sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = Dispatch;
					sci->CallbackEntry[sci->NumberOfCallbacks].Description = (UINT_PTR)TravelListEntry;
				}
				sci->NumberOfCallbacks++;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ���� KeBugCheckCallbackListHead �պþ��� KeBugCheckReasonCallbackListHead�ĺ���
//////////////////////////////////////////////////////////////////////////

UINT_PTR
APGetKeBugCheckReasonCallbackListHeadAddress()
{
	/*
	0: kd> u KeRegisterBugCheckReasonCallback l 50
	nt!KeRegisterBugCheckReasonCallback:
	fffff800`03f38da0 48895c2418      mov     qword ptr [rsp+18h],rbx
	......
	fffff800`03f38ea8 488b0581141500  mov     rax,qword ptr [nt!KeBugCheckReasonCallbackListHead (fffff800`0408a330)]
	fffff800`03f38eaf 488d0d7a141500  lea     rcx,[nt!KeBugCheckReasonCallbackListHead (fffff800`0408a330)]
	*/

	UINT_PTR KeRegisterBugCheckReasonCallback = 0;

	APGetNtosExportVariableAddress(L"KeRegisterBugCheckReasonCallback", (PVOID*)&KeRegisterBugCheckReasonCallback);
	DbgPrint("%p\r\n", KeRegisterBugCheckReasonCallback);

	if (KeRegisterBugCheckReasonCallback != 0)
	{
		PUINT8	StartSearchAddress = (PUINT8)KeRegisterBugCheckReasonCallback;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL, j = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					j = i - 7;
					if (MmIsAddressValid(j) && MmIsAddressValid(j + 1) && MmIsAddressValid(j + 2))
					{
						v1 = *j;
						v2 = *(j + 1);
						v3 = *(j + 2);
						if (v1 == 0x48 && v2 == 0x8b && v3 == 0x05)		// Ӳ����  mov rax
						{
							RtlCopyMemory(&iOffset, i + 3, 4);
							return (UINT_PTR)(iOffset + (UINT64)i + 7);
						}
					}
				}
			}
		}
	}
	return 0;
}

BOOLEAN
APGetBugCheckReasonCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR KeBugCheckReasonCallbackListHead = APGetKeBugCheckReasonCallbackListHeadAddress();

	DbgPrint("%p\r\n", KeBugCheckReasonCallbackListHead);

	if (!KeBugCheckReasonCallbackListHead)
	{
		DbgPrint("KeBugCheckReasonCallbackListHeadAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		/*
		2: kd> dt _list_entry fffff800`0407a330
		nt!_LIST_ENTRY
		[ 0xfffffa80`18dd07b8 - 0xfffff880`010b61e0 ]
		+0x000 Flink            : 0xfffffa80`18dd07b8 _LIST_ENTRY [ 0xfffffa80`19f71120 - 0xfffff800`0407a330 ]
		+0x008 Blink            : 0xfffff880`010b61e0 _LIST_ENTRY [ 0xfffff800`0407a330 - 0xfffff880`00f60560 ]

		2: kd> dq 0xfffffa80`18dd07b8
		fffffa80`18dd07b8  fffffa80`19f71120 fffff800`0407a330
		fffffa80`18dd07c8  fffff880`00f49054 fffffa80`18dd0800
		fffffa80`18dd07d8  fffff300`19d19856 00000001`00000002
		fffffa80`18dd07e8  00000000`00000000 0000057f`e71ee188
		fffffa80`18dd07f8  696e6f6d`00000000 00726f74`696e6f6d
		fffffa80`18dd0808  00000000`00000000 00000000`00000000
		fffffa80`18dd0818  00000000`00000000 00000000`00000001
		fffffa80`18dd0828  00000000`00000000 206c6148`0225001a

		2: kd> u fffff880`00f49054
		fffff880`00f49054 48895c2408      mov     qword ptr [rsp+8],rbx
		fffff880`00f49059 4889742410      mov     qword ptr [rsp+10h],rsi
		fffff880`00f4905e 57              push    rdi
		fffff880`00f4905f 4883ec20        sub     rsp,20h
		fffff880`00f49063 4181780c00100000 cmp     dword ptr [r8+0Ch],1000h
		fffff880`00f4906b 498bf8          mov     rdi,r8
		fffff880`00f4906e 727d            jb      fffff880`00f490ed
		fffff880`00f49070 488d9ae8feffff  lea     rbx,[rdx-118h]

		*/

		UINT_PTR     Dispatch = 0;

		for (PLIST_ENTRY TravelListEntry = ((PLIST_ENTRY)KeBugCheckReasonCallbackListHead)->Flink;
			TravelListEntry != (PLIST_ENTRY)KeBugCheckReasonCallbackListHead;
			TravelListEntry = TravelListEntry->Flink)
		{
			Dispatch = *(PUINT_PTR)((PUINT8)TravelListEntry + sizeof(LIST_ENTRY));	// ��ListEntry

			if (Dispatch && MmIsAddressValid((PVOID)Dispatch))
			{
				if (CallbackCount > sci->NumberOfCallbacks)
				{
					sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyKeBugCheckReason;
					sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = Dispatch;
					sci->CallbackEntry[sci->NumberOfCallbacks].Description = (UINT_PTR)TravelListEntry;
				}
				sci->NumberOfCallbacks++;
			}
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

UINT_PTR
APGetIopNotifyShutdownQueueHeadAddress()
{
	/*
	2: kd> u IoRegisterShutdownNotification l 20
	nt!IoRegisterShutdownNotification:
	fffff800`04278f50 48895c2408      mov     qword ptr [rsp+8],rbx
	......
	fffff800`04278f8a 488d0dff59e0ff  lea     rcx,[nt!IopNotifyShutdownQueueHead (fffff800`0407e990)]
	*/

	UINT_PTR IoRegisterShutdownNotification = 0;

	APGetNtosExportVariableAddress(L"IoRegisterShutdownNotification", (PVOID*)&IoRegisterShutdownNotification);
	DbgPrint("%p\r\n", IoRegisterShutdownNotification);

	if (IoRegisterShutdownNotification != 0)
	{
		PUINT8	StartSearchAddress = (PUINT8)IoRegisterShutdownNotification;
		PUINT8	EndSearchAddress = StartSearchAddress + 0x500;
		PUINT8	i = NULL;
		UINT8   v1 = 0, v2 = 0, v3 = 0;
		INT32   iOffset = 0;    // ע�������ƫ�ƿ����ɸ� ���ܶ�UINT��
		UINT64  VariableAddress = 0;

		for (i = StartSearchAddress; i < EndSearchAddress; i++)
		{
			if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
			{
				v1 = *i;
				v2 = *(i + 1);
				v3 = *(i + 2);
				if (v1 == 0x48 && v2 == 0x8d && v3 == 0x0d)		// Ӳ����  lea rcx
				{
					RtlCopyMemory(&iOffset, i + 3, 4);
					return iOffset + (UINT64)i + 7;
				}
			}
		}
	}
	return 0;
}

UINT_PTR
APGetShutdownDispatch(IN PDEVICE_OBJECT DeviceObject)
{
	PDRIVER_OBJECT DriverObject = NULL;
	UINT_PTR ShutdownDispatch = 0;

	if (DeviceObject && MmIsAddressValid((PVOID)DeviceObject))
	{
		DriverObject = DeviceObject->DriverObject;
		if (DriverObject && MmIsAddressValid((PVOID)DriverObject))
		{
			ShutdownDispatch = (UINT_PTR)DriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
		}
	}

	return ShutdownDispatch;
}

BOOLEAN
APGetShutDownCallbackNotify(OUT PSYS_CALLBACK_INFORMATION sci, IN UINT32 CallbackCount)
{
	UINT_PTR IopNotifyShutdownQueueHead = APGetIopNotifyShutdownQueueHeadAddress();

	DbgPrint("%p\r\n", IopNotifyShutdownQueueHead);

	if (!IopNotifyShutdownQueueHead)
	{
		DbgPrint("IopNotifyShutdownQueueHeadAddress NULL\r\n");
		return FALSE;
	}
	else
	{
		for (PLIST_ENTRY DispatchListEntry = ((PLIST_ENTRY)IopNotifyShutdownQueueHead)->Flink;
			DispatchListEntry != (PLIST_ENTRY)IopNotifyShutdownQueueHead;
			DispatchListEntry = DispatchListEntry->Flink)
		{
			UINT_PTR Address = (UINT_PTR)((PUINT8)DispatchListEntry + sizeof(LIST_ENTRY));   // ��ListEntry

			if (Address && MmIsAddressValid((PVOID)Address))
			{
				PDEVICE_OBJECT DeviceObject = (PDEVICE_OBJECT)(*(PUINT_PTR)Address);

				if (DeviceObject && MmIsAddressValid((PVOID)DeviceObject))
				{
					if (CallbackCount > sci->NumberOfCallbacks)
					{
						sci->CallbackEntry[sci->NumberOfCallbacks].Type = ct_NotifyShutdown;
						sci->CallbackEntry[sci->NumberOfCallbacks].CallbackAddress = APGetShutdownDispatch(DeviceObject);		// ͨ���豸�����ҵ��������� SHUTDOWN ��ǲ����
						sci->CallbackEntry[sci->NumberOfCallbacks].Description = (UINT_PTR)DeviceObject;
					}
					sci->NumberOfCallbacks++;
				}
			}
		}
	}

	return TRUE;
}



/************************************************************************
*  Name : APEnumSystemCallback
*  Param: OutputBuffer			Ring3Buffer      ��OUT��
*  Param: OutputLength			Ring3BufferLength��IN��
*  Ret  : NTSTATUS
*  ö�����е�ϵͳ�ص�
************************************************************************/
NTSTATUS
APEnumSystemCallback(OUT PVOID OutputBuffer, IN UINT32 OutputLength)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	PSYS_CALLBACK_INFORMATION sci = (PSYS_CALLBACK_INFORMATION)OutputBuffer;

	UINT32 CallbackCount = (OutputLength - sizeof(SYS_CALLBACK_INFORMATION)) / sizeof(SYS_CALLBACK_ENTRY_INFORMATION);

	APGetCreateProcessCallbackNotify(sci, CallbackCount);   // �������̻ص�
	APGetCreateThreadCallbackNotify(sci, CallbackCount);    // �����̻߳ص�
	APGetLoadImageCallbackNotify(sci, CallbackCount);		 // ӳ����� ж�ػص�
	APGetRegisterCallbackNotify(sci, CallbackCount);       // ע���ص�
	APGetBugCheckCallbackNotify(sci, CallbackCount);       // ������ص�
	APGetBugCheckReasonCallbackNotify(sci, CallbackCount); // 
	APGetShutDownCallbackNotify(sci, CallbackCount);       // �ػ��ص�

	// 
	// ע���ص�
	// �ļ�ϵͳ�ı�ص�

	if (CallbackCount >= sci->NumberOfCallbacks)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_BUFFER_TOO_SMALL;
	}

	return Status;
}

