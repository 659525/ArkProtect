#pragma once
#include <Windows.h>
#include <strsafe.h>
#include <winsvc.h>     // ������Ҫ
#include "afxcmn.h"
#include "Define.h"
#pragma comment(lib, "Version.lib")      // GetFileVersionInfo ��Ҫ���Ӵ˿�

namespace ArkProtect 
{
	typedef BOOL(WINAPI *pfnIsWow64Process) (HANDLE, PBOOL);

	class CGlobal
	{
	public:
		CGlobal() {};
		~CGlobal() {};

		//////////////////////////////////////////////////////////////////////////
		// ͨ�ú���


		BOOL QueryOSBit()
		{
#if defined(_WIN64)
			return TRUE;  // 64λ����ֻ��64bitϵͳ������
#elif defined(_WIN32)
			// 32λ������32/64λϵͳ�����С�
			// ���Ա����ж�
			BOOL bIs64 = FALSE;
			pfnIsWow64Process fnIsWow64Process;

			fnIsWow64Process = (pfnIsWow64Process)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
			if (fnIsWow64Process != NULL)
			{
				return fnIsWow64Process(GetCurrentProcess(), &bIs64) && bIs64;
			}
			return FALSE;
#else
			return FALSE; // Win64��֧��16λϵͳ
#endif
		}


		BOOL LoadNtDriver(WCHAR *wzServiceName, WCHAR *wzDriverPath)
		{
			WCHAR wzDriverFullPath[MAX_PATH] = { 0 };
			GetFullPathName(wzDriverPath, MAX_PATH, wzDriverFullPath, NULL);
		
			// ������������ƹ����������ӣ�����ָ�������ݿ�
			m_ManagerHandle = OpenSCManagerW(NULL,			// ָ������������� NULL ---> ���Ӵ����ؼ�����ķ�����ƹ�����
				NULL,										// ָ��Ҫ�򿪵ķ�����ƹ������ݿ������ NULL ---> SERVICES_ACTIVE_DATABASE���ݿ�
				SC_MANAGER_ALL_ACCESS);						// ָ��������ʿ��ƹ�������Ȩ��
			if (m_ManagerHandle == NULL)						// ����ָ���ķ�����ƹ��������ݿ�ľ��
			{
				return FALSE;
			}

			// ����һ��������󣬲�������ӵ�ָ���ķ�����ƹ��������ݿ�
			m_ServiceHandle = CreateServiceW(m_ManagerHandle,		// ������ƹ������ά���ĵǼ����ݿ�ľ��
				wzServiceName,									// �����������ڴ����Ǽ����ݿ��еĹؼ���
				wzServiceName,									// �������������û������ʶ����
				SERVICE_ALL_ACCESS,								// ���񷵻�����		����Ȩ��
				SERVICE_KERNEL_DRIVER,							// ��������			�������������
				SERVICE_DEMAND_START,							// �����ʱ����		���ɷ��������SCM�����ķ���
				SERVICE_ERROR_NORMAL,							// ��������ʧ�ܵ����س̶�
				wzDriverFullPath,								// �������������ļ�·��
				NULL, NULL, NULL, NULL, NULL);
			if (m_ServiceHandle == NULL)							// ���ط�����
			{
				if (ERROR_SERVICE_EXISTS == GetLastError())		// �Ѵ�����ͬ�ķ���
				{
					// �Ǿʹ򿪷���
					m_ServiceHandle = OpenServiceW(m_ManagerHandle, wzServiceName, SERVICE_ALL_ACCESS);
					if (m_ServiceHandle == NULL)
					{
						return FALSE;
					}
				}
				else
				{
					return FALSE;
				}
			}

			// ��������
			BOOL bOk = StartServiceW(m_ServiceHandle, 0, NULL);
			if (!bOk)
			{
				if ((GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
					|| GetLastError() == ERROR_IO_PENDING)
				{
					return FALSE;
				}
			}

			m_bDriverService = TRUE;

			return TRUE;
		}


		// ж����������
		void UnloadNTDriver(WCHAR *wzServiceName)
		{
			if (m_ServiceHandle)
			{
				SERVICE_STATUS ServiceStatus;

				// ֹͣ����
				BOOL bOk = ControlService(m_ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
				if (bOk && m_bDriverService)
				{
					// ɾ������
					bOk = DeleteService(m_ServiceHandle);
				}

				CloseServiceHandle(m_ServiceHandle);
			}		

			if (m_ManagerHandle)
			{
				CloseServiceHandle(m_ManagerHandle);
			}
		}


		void UpdateStatusBarTip(LPCWSTR wzBuffer)
		{
			::SendMessage(this->AppDlg->m_hWnd, sb_Tip, 0, (LPARAM)wzBuffer);
		}


		void UpdateStatusBarDetail(LPCWSTR wzBuffer)
		{
			::SendMessage(this->AppDlg->m_hWnd, sb_Tip, 0, (LPARAM)wzBuffer);
		}


		// ����ļ�����

		CString GetFileCompanyName(CString strFilePath)
		{
			CString strCompanyName = 0;;

			if (strFilePath.IsEmpty())
			{
				return NULL;
			}

			// ������Idle System
			if (!strFilePath.CompareNoCase(L"Idle") || !strFilePath.CompareNoCase(L"System"))
			{
				return NULL;
			}

			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			LPWSTR lpstrFilename = strFilePath.GetBuffer();
			DWORD  dwHandle = 0;
			DWORD  dwVerInfoSize = GetFileVersionInfoSizeW(lpstrFilename, &dwHandle);

			if (dwVerInfoSize)
			{
				LPVOID Buffer = malloc(sizeof(UINT8) * dwVerInfoSize);

				if (Buffer)
				{
					if (GetFileVersionInfo(lpstrFilename, dwHandle, dwVerInfoSize, Buffer))
					{
						UINT cbTranslate = 0;

						if (VerQueryValue(Buffer, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
						{
							LPCWSTR lpwszBlock = 0;
							UINT    cbSizeBuf = 0;
							WCHAR   wzSubBlock[MAX_PATH] = { 0 };

							if ((cbTranslate / sizeof(struct LANGANDCODEPAGE)) > 0)
							{
								StringCchPrintf(wzSubBlock, sizeof(wzSubBlock) / sizeof(WCHAR),
									L"\\StringFileInfo\\%04x%04x\\CompanyName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
							}

							if (VerQueryValue(Buffer, wzSubBlock, (LPVOID*)&lpwszBlock, &cbSizeBuf))
							{
								WCHAR wzCompanyName[MAX_PATH] = { 0 };

								StringCchCopy(wzCompanyName, MAX_PATH / sizeof(WCHAR), (LPCWSTR)lpwszBlock);   // ��ϵͳ���ڴ�����ݿ����������Լ��ڴ浱��
								strCompanyName = wzCompanyName;
							}
						}
					}
					free(Buffer);
				}
			}

			return strCompanyName;
		}


		CString GetLongPath(CString strFilePath)
		{
			if (strFilePath.Find(L'~') != -1)
			{
				WCHAR wzLongPath[MAX_PATH] = { 0 };
				DWORD dwReturn = GetLongPathName(strFilePath, wzLongPath, MAX_PATH);
				if (dwReturn < MAX_PATH && dwReturn != 0)
				{
					strFilePath = wzLongPath;
				}
			}

			return strFilePath;
		}


		CString TrimPath(WCHAR *wzFilePath)
		{
			CString strFilePath;

			// ���磺"C:\\"
			if (wzFilePath[1] == ':' && wzFilePath[2] == '\\')
			{
				strFilePath = wzFilePath;
			}
			else if (wcslen(wzFilePath) > wcslen(L"\\SystemRoot\\") &&
				!_wcsnicmp(wzFilePath, L"\\SystemRoot\\", wcslen(L"\\SystemRoot\\")))
			{
				WCHAR wzSystemDirectory[MAX_PATH] = { 0 };
				GetWindowsDirectory(wzSystemDirectory, MAX_PATH);
				strFilePath.Format(L"%s\\%s", wzSystemDirectory, wzFilePath + wcslen(L"\\SystemRoot\\"));
			}
			else if (wcslen(wzFilePath) > wcslen(L"system32\\") &&
				!_wcsnicmp(wzFilePath, L"system32\\", wcslen(L"system32\\")))
			{
				WCHAR wzSystemDirectory[MAX_PATH] = { 0 };
				GetWindowsDirectory(wzSystemDirectory, MAX_PATH);
				strFilePath.Format(L"%s\\%s", wzSystemDirectory, wzFilePath/* + wcslen(L"system32\\")*/);
			}
			else if (wcslen(wzFilePath) > wcslen(L"\\??\\") &&
				!_wcsnicmp(wzFilePath, L"\\??\\", wcslen(L"\\??\\")))
			{
				strFilePath = wzFilePath + wcslen(L"\\??\\");
			}
			else if (wcslen(wzFilePath) > wcslen(L"%ProgramFiles%") &&
				!_wcsnicmp(wzFilePath, L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
			{
				WCHAR wzSystemDirectory[MAX_PATH] = { 0 };
				if (GetWindowsDirectory(wzSystemDirectory, MAX_PATH) != 0)
				{
					strFilePath = wzSystemDirectory;
					strFilePath = strFilePath.Left(strFilePath.Find('\\'));
					strFilePath += L"\\Program Files";
					strFilePath += wzFilePath + wcslen(L"%ProgramFiles%");
				}
			}
			else
			{
				strFilePath = wzFilePath;
			}

			strFilePath = GetLongPath(strFilePath);

			return strFilePath;
		}



		//////////////////////////////////////////////////////////////////////////


		CWnd *AppDlg = NULL;         // ����������ָ��
		CWnd *ProcessDlg = NULL;     // �������ģ�鴰��ָ��


		int iDpix = 0;               // Logical pixels/inch in X
		int iDpiy = 0;               // Logical pixels/inch in Y

		int iResizeX = 0;
		int iResizeY = 0;

		BOOL      m_bIsRequestNow = FALSE;    // ��ǰ�Ƿ����������㷢������
		HANDLE    m_DeviceHandle = NULL;    // ���ǵ������豸������
		SC_HANDLE m_ManagerHandle = NULL;	// SCM�������ľ��
		SC_HANDLE m_ServiceHandle = NULL;	// NT��������ķ�����
		BOOL      m_bDriverService = FALSE; // ָʾ�������������Ƿ�����
	};
}

