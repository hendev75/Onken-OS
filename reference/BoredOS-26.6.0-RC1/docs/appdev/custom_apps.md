<div align="center">
  <h1>Creating a Custom App</h1>
  <p><em>A step-by-step tutorial on writing a new graphical C application.</em></p>
</div>

---

This guide explains how to write a new "Hello World" application locally, compile it as an `.elf` binary into the `bin/` folder, and launch it inside BoredOS.

> [!TIP]
> **Looking for working code?** Check out the [Examples Directory](examples/README.md) for full source code demonstrating basic CLI, Windows, Animations, and TCP Networking.

## Step 1: Write the C Source

Applications reside entirely in the `src/userland/` directory. Create a new file, for example, `src/userland/gui/hello.c`.

> [!TIP]
> Group CLI apps into `src/userland/cli/` and windowed apps into `src/userland/gui/` for organization.

```c
// src/userland/gui/hello.c
#include <stdlib.h>
#include <libui.h>
#include <syscall.h>

int main(void) {
    // Attempt to open a 300x200 window
    ui_window_t wid = ui_window_create("My Custom App", 100, 100, 300, 200);
    if (wid < 0) {
        printf("Error creating window!\n");
        return 1;
    }
    
    // Write text in center
    ui_draw_string(wid, 50, 90, "Hello, BoredOS!!", 0xFFFFFFFF);
    
    // Commit drawing to screen
    ui_mark_dirty(wid, 0, 0, 300, 200);

    gui_event_t event;
    while (1) {
        if (ui_get_event(wid, &event)) {
            if (event.type == GUI_EVENT_CLOSE) {
                break; // Exit loop if 'X' is clicked
            }
        }
        
        sys_yield();
    }

    return 0; // Returning 0 smoothly exits the process via crt0.asm
}
```

## Step 2: Edit the Makefile

Now you need to tell the build system to compile `hello.c`. Fortunately, the `src/userland/Makefile` is designed to detect new C files largely automatically!

1.  Open `src/userland/Makefile`.
2.  Find the line specifying `APP_SOURCES_FULL`:
    ```make
    APP_SOURCES_FULL = $(wildcard cli/*.c gui/*.c sys/*.c games/*.c *.c)
    ```
    Since you placed the file in `gui/hello.c`, the wildcard logic will pick it up automatically.
3.  The Makefile will generate `bin/hello.elf` during the build phase.

## Step 3: Bundle it into the OS

The main overarching `Makefile` (in the project root) takes binaries from `src/userland/bin/*.elf` and places them into the `iso_root/bin/` directory, while also adding them to `limine.conf` as loadable boot modules.

1.  Go back to the root of the OS:
    ```sh
    cd ../..
    ```
2.  Compile the entire project to build the ISO and test in QEMU:
    ```sh
    make clean && make run
    ```

## Step 4: Run it inside BoredOS

1.  When BoredOS boots, launch the **Terminal** application.
2.  The OS automatically maps built applications to standard shell commands. Simply type your application's filename (without the `.elf` extension).
3.  Type `hello` in the terminal and press Enter.
4.  Your custom window will appear!

> [!NOTE]
> You can also open your app by opening the file explorer, navigating to the `bin` directory, and double-clicking the executable.

---