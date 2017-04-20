#pragma once
#include "stdafx.h"
#include <winioctl.h>

namespace ArkProtect 
{
#define LINK_NAME    L"\\??\\ArkProtectLinkName"

#define DRIVER_SERVICE_NAME		L"ArkProtectDrv"    // �����ķ�����


#define FILE_DEVICE_ARKPROTECT           0x8005

#define IOCTL_ARKPROTECT_PROCESSNUM        (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x801, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESS       (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x802, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSMODULE (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x803, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


	//////////////////////////////////////////////////////////////////////////
	// �Ӵ������
	enum eChildDlg
	{
		cd_ProcessDialog,      // ����ģ��Ի���
		cd_DriverDialog,       // ����ģ��Ի���
		cd_KernelDialog,       // �ں�ģ��Ի���
		cd_HookDialog,         // �ں˹��ӶԻ���
		cd_AboutDialog         // ������Ϣ�Ի���
	};

	// ״̬��
	enum eStatusBar
	{
		sb_Tip = WM_USER + 0x101,
		sb_Detail = WM_USER + 0x102
	};

	// PE�ļ�λ��
	enum ePeBit
	{
		pb_32,
		pb_64,
		pb_Unknown
	};

	enum eProcessInfoKind
	{
		pik_Module,
		pik_Thread,
		pik_Handle,
		pik_Window,
		pik_Memory
	};

	//////////////////////////////////////////////////////////////////////////
	// ListCtrl���б�ṹ
	typedef struct _COLUMN_STRUCT
	{
		WCHAR*	wzTitle;
		UINT	nWidth;
	} COLUMN_STRUCT;

	


}



