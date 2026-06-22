#include <ntddk.h>

#ifndef MmGetSystemAddressForMdlSafe
  #define MmGetSystemAddressForMdlSafe(x, y) MmGetSystemAddressForMdl(x)
#endif
#ifndef MmWriteCombined
  #define MmWriteCombined TRUE
#endif
#ifndef MmCached
  #define MmCached TRUE
#endif

#ifdef NDEBUG
  #define printf(x)
#else
  #define printf(x) DbgPrint x
#endif

#ifdef DVGA2
  #define DEVICE_NAME  L"\\Device\\DirectVGA2"
  #define SYMLINK_NAME L"\\DosDevices\\DirectVGA2"
#else
  #define DEVICE_NAME  L"\\Device\\DirectVGA"
  #define SYMLINK_NAME L"\\DosDevices\\DirectVGA"
#endif
// 2m is enough for 800x600x32
#define MAP_MEM_LEN  0x200000

NTSTATUS Stub(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  printf(("Stub: %u\n", stack->MajorFunction));
  Irp->IoStatus.Status = STATUS_SUCCESS;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}

#ifdef DVGA2

NTSTATUS DirectWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
  PUCHAR mappedAddr = *((PVOID *) DeviceObject->DeviceExtension);
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  PUCHAR buffer = NULL;
  ULONG dstOffset, dstWidth, srcWidth, height, safeHeight, size, i;

  if (Irp->MdlAddress != NULL) {
    buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
  }
  if (buffer == NULL) {
    printf(("Unable to get address for MDL\n"));
    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  if (stack->Parameters.Write.Length < sizeof(ULONG)*4) {
    printf(("Buffer too small for headers\n"));
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;
  }
  dstOffset = *(PULONG) buffer;
  dstWidth = *((PULONG) buffer + 1);
  srcWidth = *((PULONG) buffer + 2);
  height = *((PULONG) buffer + 3);
  buffer += sizeof(ULONG)*4;
  if (srcWidth == 0 || dstWidth < srcWidth) {
    printf(("Error: invalid srcWidth %u or dstWidth %u\n", srcWidth, dstWidth));
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;
  }
  if (dstOffset >= MAP_MEM_LEN || MAP_MEM_LEN - dstOffset < srcWidth) {
    printf(("Error: dstOffset (0x%X) + srcWidth (0x%X) exceeds MAP_MEM_LEN (0x%X)\n", dstOffset, srcWidth, MAP_MEM_LEN));
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_PARAMETER;
  }
  safeHeight = (MAP_MEM_LEN - dstOffset - srcWidth)/dstWidth + 1;
  if (safeHeight < height) {
    printf(("Truncating height by dst: requested %u, safe %u\n", height, safeHeight));
    height = safeHeight;
  }
  safeHeight = (stack->Parameters.Write.Length - sizeof(ULONG)*4)/srcWidth;
  if (safeHeight < height) {
    printf(("Truncating height by src: requested %u, safe %u\n", height, safeHeight));
    height = safeHeight;
  }
  size = srcWidth*height;
  if (sizeof(ULONG)*4 + size != stack->Parameters.Write.Length) {
    printf(("Size mismatch: %u + %u x %u = %u, length = %u\n", sizeof(ULONG)*4, srcWidth, height, sizeof(ULONG)*4 + size, stack->Parameters.Write.Length));
  }

  printf(("Ready to write %u x %u = %u bytes from 0x%p to 0x%p\n", srcWidth, height, size, buffer, mappedAddr));
  __try {
    mappedAddr += dstOffset;
    for (i = 0; i < height; i ++) {
      RtlCopyMemory(mappedAddr, buffer, srcWidth);
      mappedAddr += dstWidth;
      buffer += srcWidth;
    }
    printf(("RtlCopyMemory Done\n"));
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(ULONG)*4 + size;
  } __except (
    DbgPrint("Exception Addr: 0x%p\n", ((PEXCEPTION_POINTERS) GetExceptionInformation())->ExceptionRecord->ExceptionAddress),
    EXCEPTION_EXECUTE_HANDLER
  ) {
    printf(("RtlCopyMemory Exception. Current IRQL: %u, Exception Code: 0x%08X\n", KeGetCurrentIrql(), GetExceptionCode()));
    Irp->IoStatus.Status = GetExceptionCode();
    Irp->IoStatus.Information = 0;
  }

  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  return Irp->IoStatus.Status;
}

#else

NTSTATUS DirectWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
  PUCHAR mappedAddr = *((PVOID *) DeviceObject->DeviceExtension);
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  ULONG size = stack->Parameters.Write.Length;
  PUCHAR buffer = NULL;

  if (Irp->MdlAddress != NULL) {
    buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
  }
  if (buffer == NULL) {
    printf(("Unable to get address for MDL\n"));
    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  if (size > MAP_MEM_LEN) {
    printf(("Warning: Write size %u exceeds MAP_MEM_LEN (0x%X). Truncating.\n", size, MAP_MEM_LEN));
    size = MAP_MEM_LEN;
  }

  printf(("Ready to write %u bytes from 0x%p to 0x%p\n", size, buffer, mappedAddr));
  __try {
    RtlCopyMemory(mappedAddr, buffer, size);
    printf(("RtlCopyMemory Done\n"));
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = size;
  } __except (
    DbgPrint("Exception Addr: 0x%p\n", ((PEXCEPTION_POINTERS) GetExceptionInformation())->ExceptionRecord->ExceptionAddress),
    EXCEPTION_EXECUTE_HANDLER
  ) {
    printf(("RtlCopyMemory Exception. Current IRQL: %u, Exception Code: 0x%08X\n", KeGetCurrentIrql(), GetExceptionCode()));
    Irp->IoStatus.Status = GetExceptionCode();
    Irp->IoStatus.Information = 0;
  }

  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  return Irp->IoStatus.Status;
}

#endif

void DriverUnload(PDRIVER_OBJECT DriverObject) {
  UNICODE_STRING symlinkName;
  PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
  PVOID *mappedAddr = (PVOID *) deviceObject->DeviceExtension;

  if (*mappedAddr != NULL) {
    MmUnmapIoSpace(*mappedAddr, MAP_MEM_LEN);
    *mappedAddr = NULL;
  }

  RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
  IoDeleteSymbolicLink(&symlinkName);
  IoDeleteDevice(deviceObject);
  printf(("Driver Unloaded\n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
  ULONG i, bus, slot, bytesRead, quadPart;
  PCI_COMMON_CONFIG pciConfig;
  UNICODE_STRING deviceName, symlinkName;
  PDEVICE_OBJECT deviceObject;
  PVOID *mappedAddr;
  PHYSICAL_ADDRESS physAddr;
  NTSTATUS status;

  RtlInitUnicodeString(&deviceName, DEVICE_NAME);
  RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
  printf(("DriverEntry\n"));
  status = IoCreateDevice(DriverObject, sizeof(PVOID), &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
  if (!NT_SUCCESS(status)) {
    printf(("Failed to create device object, status = %x\n", status));
    return status;
  }
  deviceObject->Flags |= DO_DIRECT_IO;

  status = IoCreateSymbolicLink(&symlinkName, &deviceName);
  if (!NT_SUCCESS(status)) {
    printf(("Failed to create symbolic link, status = %x\n", status));
    IoDeleteDevice(deviceObject);
    return status;
  }

  // ReactOS/QEMU: 0xFE000000, ReactOS/v86,ReactOS/VBox,WinXP/VBox: 0xE0000000, most modern hardwares: 0xD0000000
  physAddr.QuadPart = 0xE0000000;
  for (bus = 0; bus < 256; bus ++) {
    for (slot = 0; slot < 32; slot ++) {
      bytesRead = HalGetBusData(PCIConfiguration, bus, slot, &pciConfig, sizeof(pciConfig));
      if (bytesRead == 0 || pciConfig.VendorID == 0xFFFF) {
        continue;
      }
      if (pciConfig.BaseClass != 0x03) {
        continue;
      }
      quadPart = pciConfig.u.type0.BaseAddresses[0] & ~0xF;
      printf(("bus: %u, slot: %u, vendorID: 0x%04X, addr: 0x%08X\n", bus, slot, pciConfig.VendorID, quadPart));
      // skip windowed framebuffer (usually 0xA0000~0xBFFFF)
      if (quadPart >= 0x80000000) {
        physAddr.QuadPart = quadPart;
      }
    }
  }
  mappedAddr = (PVOID *) deviceObject->DeviceExtension;
  *mappedAddr = NULL;
  __try {
    *mappedAddr = MmMapIoSpace(physAddr, MAP_MEM_LEN, MmWriteCombined);
    if (*mappedAddr == NULL) {
      printf(("MmMapIoSpace Failed with MmWriteCombined\n"));
      // MmCached looks okay on v86, QEMU and VBox
      *mappedAddr = MmMapIoSpace(physAddr, MAP_MEM_LEN, MmCached);
      // Fallback to MmNonCached?
    }
    printf(("MmMapIoSpace Done\n"));
  } __except (
    DbgPrint("Exception Addr: 0x%p\n", ((PEXCEPTION_POINTERS) GetExceptionInformation())->ExceptionRecord->ExceptionAddress),
    EXCEPTION_EXECUTE_HANDLER
  ) {
    printf(("MmMapIoSpace Exception. Current IRQL: %u, Exception Code: 0x%08X\n", KeGetCurrentIrql(), GetExceptionCode()));
  }
  if (*mappedAddr == NULL) {
    printf(("MmMapIoSpace Failed\n"));
    IoDeleteSymbolicLink(&symlinkName);
    IoDeleteDevice(deviceObject);
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
    DriverObject->MajorFunction[i] = Stub;
  }
  DriverObject->MajorFunction[IRP_MJ_WRITE] = DirectWrite;
  DriverObject->DriverUnload = DriverUnload;
  deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

  return STATUS_SUCCESS;
}
