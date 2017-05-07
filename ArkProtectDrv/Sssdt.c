#include "Sssdt.h"






/************************************************************************
*  Name : APEnumSssdtHook
*  Param: OutputBuffer            ring3�ڴ�
*  Param: OutputLength
*  Ret  : NTSTATUS
*  ö�ٽ���ģ��
************************************************************************/
NTSTATUS
APEnumSssdtHook(OUT PVOID OutputBuffer, IN UINT32 OutputLength)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;

	UINT32    SssdtFunctionCount = (OutputLength - sizeof(SSSDT_HOOK_INFORMATION)) / sizeof(SSSDT_HOOK_ENTRY_INFORMATION);

	PSSSDT_HOOK_INFORMATION shi = (PSSSDT_HOOK_INFORMATION)OutputBuffer;

/*	// 1.��õ�ǰ��SSDT
	g_CurrentSsdtAddress = (PKSERVICE_TABLE_DESCRIPTOR)APGetCurrentSsdtAddress();
	if (g_CurrentSsdtAddress && MmIsAddressValid(g_CurrentSsdtAddress))
	{
		// 2.��ʼ��Ssdt��������
		Status = APInitializeSsdtFunctionName();
		if (NT_SUCCESS(Status))
		{
			// 3.�����ں�SSDT(�õ�ԭ�ȵ�SSDT������ַ����)
			Status = APReloadNtkrnl();
			if (NT_SUCCESS(Status))
			{
				// 4.�Ա�Original&Current
				for (UINT32 i = 0; i < g_CurrentSsdtAddress->Limit; i++)
				{
					if (SsdtFunctionCount >= shi->NumberOfSsdtFunctions)
					{
#ifdef _WIN64
						// 64λ�洢���� ƫ�ƣ���28λ��
						INT32 OriginalOffset = g_SsdtItem[i] >> 4;
						INT32 CurrentOffset = (*(PINT32)((UINT64)g_CurrentSsdtAddress->Base + i * 4)) >> 4;    // ������λ����λ

						UINT64 CurrentSsdtFunctionAddress = (UINT_PTR)((UINT_PTR)g_CurrentSsdtAddress->Base + CurrentOffset);
						UINT64 OriginalSsdtFunctionAddress = g_OriginalSsdtFunctionAddress[i];

#else
						// 32λ�洢���� ���Ե�ַ
						UINT32 CurrentSsdtFunctionAddress = *(UINT32*)((UINT32)g_CurrentSsdtAddress->Base + i * 4);
						UINT32 OriginalSsdtFunctionAddress = g_SsdtItem[i];

#endif // _WIN64

						if (OriginalSsdtFunctionAddress != CurrentSsdtFunctionAddress)   // ������Hook��
						{
							shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].bHooked = TRUE;
						}
						else
						{
							shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].bHooked = FALSE;
						}
						shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].Ordinal = i;
						shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].CurrentAddress = CurrentSsdtFunctionAddress;
						shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].OriginalAddress = OriginalSsdtFunctionAddress;

						RtlStringCchCopyW(shi->SsdtHookEntry[shi->NumberOfSsdtFunctions].wzFunctionName, wcslen(g_SsdtFunctionName[i]) + 1, g_SsdtFunctionName[i]);

						Status = STATUS_SUCCESS;
					}
					else
					{
						Status = STATUS_BUFFER_TOO_SMALL;
					}
					shi->NumberOfSsdtFunctions++;
				}
			}
			else
			{
				DbgPrint("Reload Ntkrnl & Ssdt Failed\r\n");
			}
		}
		else
		{
			DbgPrint("Initialize Ssdt Function Name Failed\r\n");
		}
	}
	else
	{
		DbgPrint("Get Current Ssdt Failed\r\n");
	}
*/
	return Status;
}

