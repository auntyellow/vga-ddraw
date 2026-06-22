# vga-ddraw

A DirectDraw API re-implementation for smooth 2D gaming under standard VGA drivers in QEMU, v86, and other emulators.

## Overview

- Based on FunkyFr3sh's [cnc-ddraw 1.3.4.0](https://github.com/FunkyFr3sh/cnc-ddraw/tree/1.3.4.0).
- Includes the Windows NT driver components `\\.\DirectVGA` and `\\.\DirectPalette` as part of this project: `\\.\DirectVGA` accelerates rendering to the VGA framebuffer, and `\\.\DirectPalette` stabilizes 8-bit output by directly modifying the VGA palette.

## Supported platforms

- Windows 2000 / XP
- Windows NT 4.0 / ReactOS: DirectPalette is required for 8-bit mode
- Windows NT 3.51: DirectPalette is required for 8-bit mode; msvcrt.dll (from Windows NT 4.0) is required
- Windows 95 / 98 / ME: DirectVGA is not supported; a GDI-based software renderer is used instead

## Use case

This project targets legacy Windows gaming environments where the guest system only exposes a traditional VGA framebuffer. It helps DirectDraw-based games run more smoothly inside emulators and virtual machines that do not provide modern GPU acceleration.

The following demos were tested under [Windows NT 3.51](https://winworldpc.com/product/windows-nt-3x/351) using the [v86 emulator](https://github.com/copy/v86):

- [C&C Red Alert (RA95) Demo](https://www.chess-wizard.com/minigames/minigame_ra95demo.htm)
- [StarCraft Shareware](https://www.chess-wizard.com/minigames/minigame_scdemo.htm)

## Changes from `cnc-ddraw`

- Supports 24-bit and 32-bit color modes (24/32 bpp).
- Removes OpenGL and D3D9 renderers (retains the software renderer only).
- Removes screenshot functionality.
- Disables windowed mode, cutscenes, and stretching behavior.
- Improves compatibility for Windows 95, NT 3.51 and NT 4.0.

Why choose `cnc-ddraw` 1.3.4.0?

- It is the base used in FunkyFr3sh's [Red Alert 1 Installer](https://funkyfr3sh.cnc-comm.com/).
- Version 1.3.5.0 introduces a dependency on [Detours](https://github.com/microsoft/detours), which makes it incompatible with Windows 9x and NT 3.51.

## How to Compile

### NT 5.x Drivers

- Install [DDK 5.1.2600.1106](https://winworldpc.com/product/windows-sdk-ddk/xp-nt-51) into `C:\WINDDK\2600.1106`
- Run `dvga\makefile.bat`

### NT 3.51 / 4.0 Drivers

- Install [MSVC 2.1](https://winworldpc.com/product/visual-c/2x) into `C:\MSVC20`
- Install [DDK 3.51.1057.1](https://winworldpc.com/product/windows-sdk-ddk/nt-3x) into `C:\DDK`
- Run `dvga\makefile_nt351.bat`

### DirectDraw

- Install mingw32 (apt install gcc-mingw-w64-i686)
- Run `makefile.bat` or `makefile.sh`

### Tests

- Install MSVC 6.0 (Visual Studio 6.0) or mingw32 (apt install gcc-mingw-w64-i686)
- Run `dvga\maketest.bat`, `test\makefile.bat` or `dvga/maketest.sh`, `test/makefile.sh`