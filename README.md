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

