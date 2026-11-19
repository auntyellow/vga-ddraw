#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

#if _DEBUG

#define printf(x) DebugPrint9x x

static FILE *fpDebug = NULL;
static int fdDebug = -1;

void DebugPrint9x(const char *format, ...)
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

#define DVGA_ADDR_UNAVAILABLE 0xFFFFFFFF

typedef DWORD (__stdcall *PFN_VXDCALL)(DWORD, DWORD, DWORD);

#pragma pack(2)
typedef struct {
    DWORD edi, esi, ebp, dwReserved, ebx, edx, ecx, eax;
    WORD wFlags, es, ds, fs, gs, ip, cs, sp, ss;
} DPMI_REG_REC;
#pragma pack()

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

static DWORD GetBusData(DWORD bus, DWORD slot, DWORD func, DWORD offset) {
    _outpd(0xCF8, (1U << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC));
    return _inpd(0xCFC);
}

// unchecked = 0, available = valid memory, unavailable = 0xFFFFFFFF;
static DWORD dwDVGAAddress = 0;

static BOOL IsAvailable() {
    DWORD bus, slot, vendorId, headerType, maxFunc, func, baseClass, bar0, dwAddr;
    /*
    HANDLE hVWin32;
    DIOC_REGISTERS regs;
    BOOL bSuccess;
    DWORD cbReturned;
    */
    HMODULE hKernel32;
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeaders;
    PIMAGE_EXPORT_DIRECTORY pExportDir;
    PFN_VXDCALL pVxDCall;
    DWORD dwRVA;
    WORD wax, wbx, wcx;
    BOOLEAN bcf;
    DPMI_REG_REC drr;

    if (dwDVGAAddress) {
        return dwDVGAAddress != DVGA_ADDR_UNAVAILABLE;
    }

#if _DEBUG
    fpDebug = fopen("dvga9x.log", "wt");
    // enable fdDebug for hard lockup
    // fdDebug = _fileno(fpDebug);
#endif

    hKernel32 = GetModuleHandle("KERNEL32.DLL");
    if (!hKernel32) {
        printf(("Unable to get handler for KERNEL32.DLL, error = %lu\n", GetLastError()));
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
    }
    // avoid compatibility mode
    if (!GetProcAddress(hKernel32, "QT_Thunk")) {
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
    }

    dwAddr = 0xE0000000;
    for (bus = 0; bus < 256; bus ++) {
        for (slot = 0; slot < 32; slot ++) {
            vendorId = GetBusData(bus, slot, 0, 0) & 0xFFFF;
            if (vendorId == 0xFFFF) {
                continue;
            }
            headerType = GetBusData(bus, slot, 0, 0xC);
            maxFunc = (headerType & 0x00800000) ? 8 : 1;
            for (func = 0; func < maxFunc; func ++) {
                if (func > 0) {
                    vendorId = GetBusData(bus, slot, func, 0) & 0xFFFF;
                    if (vendorId == 0xFFFF) {
                        continue;
                    }
                }
                baseClass = GetBusData(bus, slot, func, 8) >> 24;
                if (baseClass != 0x03) {
                    continue;
                }
                bar0 = GetBusData(bus, slot, func, 0x10);
                printf(("bus: %lu, slot: %lu, func: %lu, vendorID: 0x%04lX, bar0: 0x%08lX\n", bus, slot, func, vendorId, bar0));
                if (bar0 >= 0x80000000 && !(bar0 & 1)) {
                    dwAddr = bar0 & ~0xF;
                }
            }
        }
    }

    /*
    hVWin32 = CreateFile("\\\\.\\VWIN32", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVWin32 == INVALID_HANDLE_VALUE) {
        printf(("Unable to get handler for VWin32, error = %lu\n", GetLastError()));
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
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
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
    }
    if (regs.reg_Flags & 1) {
        printf(("_MapPhysToLinear failed, error = 0x%08lX\n", regs.reg_EAX));
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
    }
    dwAddr = (regs.reg_EBX << 16) + regs.reg_ECX;
    */

    // refer to https://github.com/starofrainnight/winio/blob/v2.x/Source/Dll/k32exp.c#L38
    pDosHeader = (PIMAGE_DOS_HEADER) hKernel32;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf(("Invalid magic %04x\n", pDosHeader->e_magic));
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
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
        "movw $0x20, %%si\n\t"
        "movw $0, %%di\n\t"
        :
        : "b" (wbx), "c" (wcx)
        : "si", "di"
    );
    pVxDCall(0x2A0029, 0x800, dwAddr);
    __asm__ __volatile__ (
        "setcb %0\n\t"
        : "=m" (bcf), "=a" (wax), "=b" (wbx), "=c" (wcx)
        :
    );
#endif
    if (bcf) {
        printf(("Failed to call _MapPhysToLinear, error = 0x%04X", wax));
        dwDVGAAddress = DVGA_ADDR_UNAVAILABLE;
        return FALSE;
    }
    dwDVGAAddress = ((DWORD) wbx << 16) + wcx;
    printf(("_MapPhysToLinear returns 0x%08lX\n", dwDVGAAddress));

    // VMDisp9x uses 8 bits, while WritePalette uses 6 bits
    ZeroMemory(&drr, sizeof(drr));
    drr.eax = 0x4F08;
    drr.ebx = 0x600;
#ifdef _MSC_VER
    __asm {
        mov bx, 0x10;
        mov cx, 0;
        push ds;
        pop es;
        lea edi, drr;
    }
    pVxDCall(0x2A0029, 0x300, 0);
    __asm {
        setc bcf;
        mov wax, ax;
    }
#else
    __asm__ __volatile__ (
        "movw $0x10, %%bx\n\t"
        "movw $0, %%cx\n\t"
        "push %%ds\n\t"
        "pop %%es\n\t"
        "leal %[src], %%edi\n\t"
        :
        : [src] "m" (drr)
        : "bx", "cx", "edi"
    );
    pVxDCall(0x2A0029, 0x300, 0);
    __asm__ __volatile__ (
        "setcb %0\n\t"
        : "=m" (bcf), "=a" (wax)
        :
    );
#endif
    if (bcf) {
        printf(("Failed to simulate int 10h, error = 0x%04X", wax));
        // Ignore
    }

    return TRUE;
}

LPVOID GetDVGAAddress(void) {
    return IsAvailable() ? (LPVOID) dwDVGAAddress : NULL;
}

void WritePalette(LPPALETTEENTRY lpPalEntries, LONG nSize) {
    LONG i;
    PALETTEENTRY palEntry;
    if (!IsAvailable()) {
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