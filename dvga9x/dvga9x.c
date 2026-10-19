#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

#if DEBUG

#define printf(x) DebugPrint x

static FILE *fpDebug = NULL;
static int fdDebug = -1;

void DebugPrint(const char *format, ...)
{
    va_list args;
    char buffer[1024];
    int i;
    va_start(args, format);
    if (fpDebug) {
        vfprintf(fpDebug, format, args);
        fflush(fpDebug);
        if (fdDebug != -1) {
            _commit(fdDebug);
        }
    } else {
        i = _vsnprintf(buffer, sizeof(buffer), format, args);
        buffer[i < 0 ? sizeof(buffer) - 1 : i] = '\0';
        OutputDebugStringA(buffer);
    }
    va_end(args);
}

#else

#define printf(x)

#endif

typedef DWORD (__stdcall *PFN_VXDCALL)(DWORD, DWORD, DWORD);

// unchecked = 0, Win9x = 1, WinNT = -1
static LONG nIsWin9x = 0;

static BOOL IsWin9x() {
    HMODULE hKernel32;
    if (nIsWin9x != 0) {
        return nIsWin9x > 0;
    }
    // avoid compatibility mode
    hKernel32 = GetModuleHandle("KERNEL32.DLL");
    if (!hKernel32) {
        printf(("Unable to get handler for KERNEL32.DLL, error = %lu\n", GetLastError()));
        nIsWin9x = -1;
        return FALSE;
    }
    if (!GetProcAddress(hKernel32, "QT_Thunk")) {
        nIsWin9x = -1;
        return FALSE;
    }
#if DEBUG
    fpDebug = fopen("dvga9x.log", "wt");
    // enable fdDebug for hard lockup
    // fdDebug = _fileno(fpDebug);
#endif
    system("PAL6BITS.COM");
    nIsWin9x = 1;
    return TRUE;
}

static DWORD GetBusData(DWORD bus, DWORD slot, DWORD func, DWORD offset) {
    _outpd(0xcf8, (1U << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC));
    return _inpd(0xcfc);
}

/*
typedef struct tagDIOCRegs {
    DWORD   reg_EBX;
    DWORD   reg_EDX;
    DWORD   reg_ECX;
    DWORD   reg_EAX;
    DWORD   reg_EDI;
    DWORD   reg_ESI;
    DWORD   reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;
*/

LPVOID GetDVGAAddress() {
    DWORD bus, slot, func, vendorId, baseClass, bar0, dwAddr;
    /*
    HANDLE hVWin32;
    DIOC_REGISTERS regs;
    BOOL bSuccess;
    DWORD cbReturned;
    */
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeaders;
    PIMAGE_EXPORT_DIRECTORY pExportDir;
    PFN_VXDCALL pVxDCall;
    DWORD dwRVA;
    WORD wax, wbx, wcx;
    BOOLEAN bcf;

    if (!IsWin9x()) {
        return NULL;
    }

    dwAddr = 0xE0000000;
    for (bus = 0; bus < 256; bus ++) {
        for (slot = 0; slot < 32; slot ++) {
            for (func = 0; func < 8; func ++) {
                vendorId = GetBusData(bus, slot, func, 0) & 0xFFFF;
                if (vendorId == 0xFFFF) {
                    continue;
                }
                baseClass = GetBusData(bus, slot, func, 8) >> 24;
                if (baseClass != 0x03) {
                    continue;
                }
                bar0 = GetBusData(bus, slot, func, 0x10);
                printf(("bus: %lu, slot: %lu, func: %lu, vendorID: 0x%04lX, bar0: 0x%08lX\n", bus, slot, func, vendorId, bar0));
                if (!(bar0 & 1)) {
                    dwAddr = bar0 & ~0xF;
                }
            }
        }
    }

    /*
    hVWin32 = CreateFile("\\\\.\\VWIN32", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVWin32 == INVALID_HANDLE_VALUE) {
        printf(("Unable to get handler for VWin32, error = %lu\n", GetLastError()));
        return NULL;
    }
    ZeroMemory(&regs, sizeof(regs));
    regs.reg_EAX = 0x800;
    regs.reg_EBX = dwAddr >> 16;
    regs.reg_ECX = dwAddr & 0xFFFF;
    regs.reg_ESI = 0x20;
    bSuccess = DeviceIoControl(hVWin32, 0x29, &regs, sizeof(regs), &regs, sizeof(regs), &cbReturned, NULL);
    if (!bSuccess) {
        // failed here
        printf(("Unable to call DPMI, error = %lu\n", GetLastError()));
        return NULL;
    }
    if (regs.reg_Flags & 1) {
        printf(("_MapPhysToLinear failed, error = 0x%08lX\n", regs.reg_EAX));
        return NULL;
    }
    dwAddr = (regs.reg_EBX << 16) + regs.reg_ECX;
    */

    // refer to https://github.com/starofrainnight/winio/blob/v2.x/Source/Dll/k32exp.c#L38
    pDosHeader = (PIMAGE_DOS_HEADER) GetModuleHandle("KERNEL32.DLL");
    if (!pDosHeader) {
        printf(("Unable to get handler for KERNEL32.DLL, error = %lu\n", GetLastError()));
        return NULL;
    }
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf(("Invalid magic %04x\n", pDosHeader->e_magic));
        return NULL;
    }
    pNtHeaders = (PIMAGE_NT_HEADERS) ((LPBYTE) pDosHeader + pDosHeader->e_lfanew);
    dwRVA = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    pExportDir = (PIMAGE_EXPORT_DIRECTORY) ((LPBYTE) pDosHeader + dwRVA);
    dwRVA = ((LPDWORD) ((LPBYTE) pDosHeader + pExportDir->AddressOfFunctions))[0];
    pVxDCall = (PFN_VXDCALL) ((LPBYTE) pDosHeader + dwRVA);

    // refer to https://github.com/starofrainnight/winio/blob/v2.x/Source/Dll/Phys32.cpp#L51
    wbx = (WORD) (dwAddr >> 16);
    wcx = (WORD) dwAddr;
#ifdef _MSC_VER
    __asm {
        mov bx, wbx;
        mov cx, wcx;
        mov si, 0x20;
        mov di, 0;
    }
    pVxDCall(0x2A0029, 0x800, dwAddr);
    __asm {
        setc bcf;
        mov wax, ax;
        mov wbx, bx;
        mov wcx, cx;
    }
#else
    __asm__ __volatile__ (
        "movw %0, %%bx\n\t"
        "movw %1, %%cx\n\t"
        "movw $0x20, %%si\n\t"
        "movw $0, %%di\n\t"
        :
        : "m" (wbx), "m" (wcx)
        : "bx", "cx", "si", "di"
    );
    pVxDCall(0x2A0029, 0x800, dwAddr);
    __asm__ __volatile__ (
        "setcb %0\n\t"
        "movw %%ax, %1\n\t"
        "movw %%bx, %2\n\t"
        "movw %%cx, %3\n\t"
        : "=m" (bcf), "=m" (wax), "=m" (wbx), "=m" (wcx)
        :
        :
    );
#endif
    if (bcf) {
        printf(("Failed to call _MapPhysToLinear, error = %04X", wax));
        return NULL;
    }
    dwAddr = ((DWORD) wbx << 16) + wcx;
    printf(("_MapPhysToLinear returns 0x%08lX\n", dwAddr));
    return (LPVOID) dwAddr;
}

void WritePalette(LPPALETTEENTRY lpPalEntries, LONG nSize) {
    LONG i;
    PALETTEENTRY palEntry;
    if (!IsWin9x()) {
        return;
    }
    for (i = 0; i < nSize; i ++) {
        palEntry = lpPalEntries[i];
        _outp(0x3c8, (BYTE) i);
        _outp(0x3c9, palEntry.peRed >> 2);
        _outp(0x3c9, palEntry.peGreen >> 2);
        _outp(0x3c9, palEntry.peBlue >> 2);
    }
}