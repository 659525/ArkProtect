#ifndef CXX_Imports_H

#include <ntifs.h>

PEPROCESS PsIdleProcess;     // Idle �� EProcess������ȫ�ֱ���


NTKERNELAPI
UCHAR *
PsGetProcessImageFileName(
	__in PEPROCESS Process
);

#endif // !CXX_Imports_H
