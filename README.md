# Onken OS

Onken OS is a custom 64-bit operating system developed from scratch. It features a fully functioning hardware-accelerated Window Manager, zero-latency desktop compositing, and a retro Windows 95 aesthetic.

## Features
- **Diff-Based Desktop Compositor**: Achieves perfect 60 FPS window dragging by keeping a cached snapshot of the desktop in RAM and mathematically erasing cursors/menus in memory, completely bypassing function call overhead.
- **Hardware Interaction**: Hooks directly into QEMU/Bochs ACPI shutdown ports (`0x604` and `0xB004`) for instant system power-offs.
- **HHDM Memory Map**: Full support for High Half Direct Mapping via the Limine Bootloader Protocol.
- **Window Manager**: True Z-ordered window management system.
- **Core Userland Apps**:
  - `htop`: Built-in graphical task manager.
  - `yano`: (Yet Another Nano) Fully interactive, typeable text editor.
  - `System Info`: Displays custom kernel specifications and ASCII art.

## Build Instructions
Requirements: `nasm`, `gcc`, `xorriso`, `limine`

```bash
make clean
make all
make run
```
