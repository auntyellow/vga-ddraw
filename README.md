# vga-ddraw

A DirectDraw API re-implementation for smooth 2D gaming under standard VGA drivers in QEMU, v86, and other emulators.

## Overview

- Based on FunkyFr3sh's [cnc-ddraw 1.3.4.0](https://github.com/FunkyFr3sh/cnc-ddraw/tree/1.3.4.0).
- Includes the components `DirectVGA` and `DirectPalette` as part of this project: `DirectVGA` accelerates rendering to the VGA linear frame buffer, and `DirectPalette` stabilizes 8-bit output by directly modifying the VGA palette.

## Performance

When running the [v86 emulator](https://github.com/copy/v86) in a modern browser on a very old computer (e.g. 4th-gen Core i5), vga-ddraw easily reaches 900+ FPS (5~10x of cnc-ddraw's soft renderer).

## Supported platforms

- Windows 7 (32-bit only)
- Windows 2000 / XP
- ReactOS<sup> [2]</sup>
- Windows NT 4.0<sup> [2]</sup>
- Windows NT 3.51<sup> [2,3]</sup> 
- Windows 95 / 98 / ME<sup> [4]</sup>

Notes:

1. DirectVGA (`dvga.sys`) is required for all NT-like OSes. The absence of DirectVGA causes vga-ddraw to fall back to software rendering.
2. DirectPalette (`dpal.sys`) is required for 8-bit mode. The absence of DirectPalette causes color anomalies.
3. `msvcrt.dll` (from Windows NT 4.0) is required.
4. No additional driver installation is required because of the built-in DirectVGA and DirectPalette for Win9x.

## Use case

This project targets legacy Windows gaming environments where the guest system exposes the VGA linear frame buffer. It helps DirectDraw-based games run more smoothly inside emulators and virtual machines that do not provide modern GPU acceleration.

The following demos were tested under [Windows NT 3.51](https://winworldpc.com/product/windows-nt-3x/351) or [Windows 95](https://winworldpc.com/product/windows-95/rtm) using the v86 emulator:

- [C&C Red Alert (RA95) Demo](https://www.chess-wizard.com/minigames/minigame_ra95demo.htm)
- [StarCraft Shareware](https://www.chess-wizard.com/minigames/minigame_scdemo.htm)
- [Need for Speed III Demo](http://www.chess-wizard.com/minigames/minigame_nfs3demo.htm)

Note that **vga-ddraw** is not the only way to run these games in v86. [ddraw-wrapper](wrapper/README.md) and [VMDisp9x](https://github.com/JHRobotics/vmdisp9x) are alternative solutions.

## How to use

### WinNT

- Under Windows 7 / XP / 2000 / ReactOS, run `install.bat` to install DirectVGA (`dvga.sys`) and DirectPalette (`dpal.sys`) drivers. 
- Under Windows NT 4.0 / 3.51, move `install.bat` into `nt351` folder and run it.
- Put `ddraw.dll` into the same folder as the game executable file, or backup and overwrite system's `ddraw.dll`. 

### Win9x

- **vga-ddraw** includes the built-in DirectVGA and DirectPalette for Win9x (won't be loaded if WinNT is detected), so no additional driver installation is required.
- Win9x's `dsound.dll` relies on `VidMem*` functions in ddraw.dll, so replacing system's `ddraw.dll` will cause `dsound.dll` to fail to load. As a workaround, we need to rename `ddraw.dll` to `vdraw.dll` and modify the Import Directory of the game executable file using [CFF Explorer](https://ntcore.com/explorer-suite/).
- If the game loads `ddraw.dll` dynamically like [NFS3](http://www.chess-wizard.com/minigames/minigame_nfs3demo.htm), we need to rename system's `ddraw.dll` to `ddra0.dll` and modify the Import Directory of `dsound.dll`, and place vga-ddraw's `ddraw.dll` into `system` folder.
- DirectPalette sets 6-bit VGA RGB palette to VGA ports 0x3c8 and 0x3c9, but VMDisp9x uses 8-bit by default. If VMDisp9x is installed, we should put `PAL6BITS.COM` into `windows` folder and `PAL6BITS.COM` will be called automatically during initialization.

## Changes from `cnc-ddraw`

- Supports 24-bit and 32-bit color modes (24/32 bpp).
- Removes OpenGL and D3D9 renderers (retains the software renderer only).
- Removes screenshot functionality.
- Disables windowed mode, cutscenes, and stretching behavior.
- Improves compatibility for Windows 95, NT 3.51 and NT 4.0.
- Supports DirectDraw 5.0 (IDirectDraw2 and IDirectDrawSurface3).
- Adds the built-in DirectVGA and DirectPalette for Win9x.

Why choose `cnc-ddraw` 1.3.4.0?

- It is the base used in FunkyFr3sh's [Red Alert 1 Installer](https://funkyfr3sh.cnc-comm.com/).
- Version 1.3.5.0 introduces a dependency on [Detours](https://github.com/microsoft/detours), which makes it incompatible with Windows 9x and NT 3.51.

## How to Build

### NT 5.x Drivers

- Install [DDK 5.1.2600.1106](https://winworldpc.com/product/windows-sdk-ddk/xp-nt-51) into `C:\WINDDK\2600.1106`
- Run `dvga\makefile.bat`

### NT 3.51 / 4.0 Drivers

- Install [MSVC 2.1](https://winworldpc.com/product/visual-c/2x) into `C:\MSVC20`
- Install [DDK 3.51.1057.1](https://winworldpc.com/product/windows-sdk-ddk/nt-3x) into `C:\DDK`
- Run `dvga\makefile_nt351.bat`

### DirectDraw

- Install mingw32 (`sudo apt install gcc-mingw-w64-i686`)
- Run `makefile.bat` or `makefile.sh`

### Tests

- Install MSVC 6.0 (Visual Studio 6.0) or mingw32 (`sudo apt install gcc-mingw-w64-i686`)
- Run `dvga\maketest.bat`, `dvga9x\maketest.bat`, `test\makefile.bat` or `dvga/maketest.sh`, `dvga9x/maketest.sh`, `test/makefile.sh`

## ddraw-wrapper

[ddraw-wrapper](wrapper/README.md) is a lightweight DirectDraw interceptor that forwards DirectDraw calls to the real `ddraw.dll` from the Windows system directory. It is useful for debugging DirectDraw usage, tracing API calls, and modifying or replacing DirectDraw behavior in legacy Windows games.