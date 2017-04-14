#include "Private.h"


/************************************************************************
*  Name : APGetNtosExportVariableAddress
*  Param: wzVariableName		Ŀ���������   ��˫�֣�
*  Param: VariableAddress		Ŀ�������ַ ��OUT��
*  Ret  : BOOLEAN
*  ͨ��ȫ�ֱ�����������ַ�����Ʒ���Ntos��������ȫ�ֱ�����������ַ����ַ���������� x86�»��SSDT��ַ
************************************************************************/
BOOLEAN
APGetNtosExportVariableAddress(IN const WCHAR *wzVariableName, OUT PVOID *VariableAddress)
{
	UNICODE_STRING	uniVariableName = { 0 };

	if (wzVariableName && wcslen(wzVariableName) > 0)
	{
		RtlInitUnicodeString(&uniVariableName, wzVariableName);

		//��Ntoskrnlģ��ĵ������л��һ�����������ĵ�ַ
		*VariableAddress = MmGetSystemRoutineAddress(&uniVariableName);		// ��������ֵ��PVOID���Ų����˶�άָ��
	}

	if (*VariableAddress == NULL)
	{
		return FALSE;
	}

	return TRUE;
}