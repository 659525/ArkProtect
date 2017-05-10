#pragma once
#include "stdafx.h"
#include <winioctl.h>

namespace ArkProtect 
{
#define LINK_NAME    L"\\??\\ArkProtectLinkName"

#define DRIVER_SERVICE_NAME		L"ArkProtectDrv"    // 开启的服务名


#define FILE_DEVICE_ARKPROTECT           0x8005

	//
	// Process
	//

#define IOCTL_ARKPROTECT_PROCESSNUM        (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x801, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESS       (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x802, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSMODULE (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x803, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSTHREAD (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x804, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSHANDLE (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x805, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSWINDOW (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x806, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMPROCESSMEMORY (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x807, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_TERMINATEPROCESS  (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x808, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

	//
	// Driver
	//

#define IOCTL_ARKPROTECT_ENUMDRIVER        (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x811, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_UNLOADRIVER       (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x812, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

	//
	// Kernel
	//
#define IOCTL_ARKPROTECT_ENUMSYSCALLBACK   (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x821, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMFILTERDRIVER  (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x822, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMIOTIMER       (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x823, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMDPCTIMER      (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x824, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

	//
	// Hook
	//
#define IOCTL_ARKPROTECT_ENUMSSDTHOOK      (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x831, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_RESUMESSDTHOOK    (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x832, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_ENUMSSSDTHOOK     (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x833, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ARKPROTECT_RESUMESSSDTHOOK   (UINT32)CTL_CODE(FILE_DEVICE_ARKPROTECT, 0x834, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

	//
	// Registry
	//


	//////////////////////////////////////////////////////////////////////////
	// 子窗体相关
	enum eChildDlg
	{
		cd_ProcessDialog,      // 进程模块对话框
		cd_DriverDialog,       // 驱动模块对话框
		cd_KernelDialog,       // 内核模块对话框
		cd_HookDialog,         // 内核钩子对话框
		cd_RegistryDialog,     // 注册表对话框
		cd_AboutDialog         // 关于信息对话框
	};

	// 状态栏
	enum eStatusBar
	{
		sb_Tip = WM_USER + 0x101,
		sb_Detail = WM_USER + 0x102
	};

	// PE文件位数
	enum ePeBit
	{
		pb_32,
		pb_64,
		pb_Unknown
	};

	// 查看进程信息的种类
	enum eProcessInfoKind
	{
		pik_Module,
		pik_Thread,
		pik_Handle,
		pik_Window,
		pik_Memory
	};

	// 查看内核模块项
	enum eKernelItem
	{
		ki_SysCallback,
		ki_FilterDriver,
		ki_IoTimer,
		ki_DpcTimer,
		ki_SysThread		
	};

	// 查看内核钩子项
	enum eHookItem
	{
		hi_Ssdt,
		hi_Sssdt,
		hi_NtkrnlFunc,
		hi_NtkrnlIAT,
		hi_NtkrnlEAT
	};

	//////////////////////////////////////////////////////////////////////////
	// ListCtrl的列表结构
	typedef struct _COLUMN_STRUCT
	{
		WCHAR*	wzTitle;
		UINT	nWidth;
	} COLUMN_STRUCT;

	


}



