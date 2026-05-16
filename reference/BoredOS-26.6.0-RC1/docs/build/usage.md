# Building, Running, and Deployment

BoredOS uses a single top-level `Makefile` to orchestrate the build process.

## The Build Process

When you run `make` in the root directory, the following stages occur automatically:

1.  **Limine Setup (`limine-setup`)**:
    If the Limine bootloader binaries are missing, the Makefile automatically clones the appropriate Limine binary release from GitHub and compiles its host utility.
2.  **Kernel Compilation**:
    All `.c` files in `src/core`, `src/mem`, `src/dev`, `src/sys`, `src/fs`, `src/wm`, and `src/net` are compiled into object files (`.o`) inside the `build/` directory using `x86_64-elf-gcc`.
3.  **Kernel Assembly**:
    All `.asm` files in `src/arch/` are assembled into object files using `nasm`.
4.  **Kernel Linking**:
    `x86_64-elf-ld` links the kernel object files together, instructed by the `linker.ld` script, outputting the `boredos.elf` kernel binary.
5.  **Userland Compilation**:
    The Makefile shifts into the `src/userland` directory, compiling the custom `libc` and generating `bin/*.elf` executable user applications.
6.  **ISO Generation**:
    The `iso_root` directory is staged. The kernel, Limine configuration, fonts, images, and userland applications are copied in. Finally, `xorriso` generates the `boredos.iso` bootable image.

## Minimum System Requirements

To run BoredOS successfully (either in emulation or on bare metal), your target machine should meet the following minimum requirements:

-   **CPU**: An `x86_64` (64-bit) compatible processor.
-   **Memory**: Approximately `~256 MB` of RAM.
-   **Display**: A VGA-compatible display (required for the GUI Window Manager).
-   **Networking (Optional)**: A compatible Network Interface Card (NIC) is required if you want to use the networking stack (e.g., an Intel E1000 or similar supported by the [`net/nic/`](../../src/net/nic/) drivers). Networking is not strictly required for the OS to boot or run offline applications.


## Running in Emulation

To test the generated ISO quickly without real hardware, use the QEMU emulator:

```sh
make run
```

This command automatically detects your operating system and invokes QEMU with specific arguments:
-   `-m 4G`: Allocates 4 Gigabytes of RAM.
-   `-cdrom boredos.iso`: Mounts the built OS image as a CD-ROM.
-   `-smp 4`: Enables 4 CPU cores.

## Running on Bare Metal

> [!CAUTION]
> Running hobby operating systems on real hardware is at your own risk and may cause undefined behavior.

To boot BoredOS on a physical PC:

1.  Build the OS to get `boredos.iso`.
2.  Use a flashing tool like **Balena Etcher** or `dd` to write the ISO to a USB flash drive.
3.  Reboot your target computer, enter the BIOS/UEFI setup.
4.  **Boot Configuration**: BoredOS supports booting on both modern **UEFI** systems and legacy **BIOS** systems natively via Limine. Ensure **Secure Boot** is disabled in your firmware settings.
5.  Select the USB drive from the boot menu.
