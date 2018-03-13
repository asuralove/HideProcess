#include <Windows.h>
#include <stdio.h>

typedef struct _INOUT_PARAM {
	int pid;
} INOUT_PARAM, *PINOUT_PARAM;

#define DUMMYDRV_REQUEST1    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

VOID main(
	VOID
)
{
	HANDLE          h;
	INOUT_PARAM     tmp;
	DWORD           bytesIO;

	int pid;
	printf("pid=");
	scanf_s("%d", &pid);

	h = CreateFile(TEXT("\\\\.\\HideProcess"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h != INVALID_HANDLE_VALUE) {

		tmp.pid = pid;

		DeviceIoControl(h, DUMMYDRV_REQUEST1,
			&tmp, sizeof(tmp), &tmp,
			sizeof(tmp), &bytesIO, NULL);

		CloseHandle(h);
	}

}