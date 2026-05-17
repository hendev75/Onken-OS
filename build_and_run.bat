@echo off
title Onken OS - Build and Run
echo ==========================================
echo Onken OS - Windows Build ^& Run Script
echo ==========================================
echo.
echo [1] Cleaning previous build in WSL...
wsl make clean

echo.
echo [2] Building ISO in WSL...
wsl make all

if not exist oneko.iso (
    echo.
    echo [ERROR] Build failed! oneko.iso not found.
    pause
    exit /b 1
)

echo.
echo [3] Launching natively in Windows QEMU...
:: Using Windows native QEMU allows sound to route to Windows audio drivers properly.
:: We use dsound (DirectSound) for the audio backend to enable the PC speaker.
qemu-system-x86_64 -m 512 -cdrom oneko.iso -vga std -serial stdio -global VGA.xres=1280 -global VGA.yres=720 -audiodev dsound,id=snd0 -machine pcspk-audiodev=snd0

if %errorlevel% neq 0 (
    echo.
    echo [WARNING] QEMU exited with an error. 
    echo If QEMU complains about "dsound", replace it with "wasapi" or remove the audio flags in this bat file.
    pause
)
