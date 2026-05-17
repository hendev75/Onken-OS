@echo off
echo Building Onken OS Nightly...
wsl make clean
wsl make all

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Running in QEMU (Windows native for sound support)...
qemu-system-x86_64 -m 512 -cdrom oneko.iso -vga std -serial stdio -global VGA.xres=1280 -global VGA.yres=720 -audiodev dsound,id=snd0 -machine pcspk-audiodev=snd0
