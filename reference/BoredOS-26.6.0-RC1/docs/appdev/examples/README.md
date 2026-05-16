<div align="center">
  <h1>Example Applications</h1>
  <p><em>From basic output to complex Graphical and Network applications.</em></p>
</div>

---

Welcome to the examples directory! These guides are designed to help you understand how to write C applications for the BoredOS userland, utilizing the custom `libc` SDK. 

The examples are listed in order of increasing complexity. Click on a tutorial to view the complete source code and an explanation of the concepts it introduces.

## 🟢 Beginner

*   **[`01_hello_cli.md`](01_hello_cli.md)**: The absolute basics. Learn how to write a simple Terminal program that outputs text and processes standard system calls.
*   **[`02_basic_window.md`](02_basic_window.md)**: An introduction to `libui.h`. Learn how to create an empty window, set up a basic event loop, and handle the "Close" button cleanly.

## 🟡 Intermediate

*   **[`03_bouncing_ball.md`](03_bouncing_ball.md)**: Dive deeper into graphical rendering. This example introduces the `ui_mark_dirty` command, framerate independence via `sys_yield()`, and state management to animate a shape moving around the screen and bouncing off the window edges.

## 🔴 Advanced

*   **[`04_tcp_client.md`](04_tcp_client.md)**: Using the lwIP networking stack. This example demonstrates how to perform a DNS lookup, connect to an external server over TCP (like an HTTP server), send a raw request, and print the response to the terminal.

---

> [!TIP]
> If you want to test these out, simply create a new `.c` file in `src/userland/cli/` (for terminal apps) or `src/userland/gui/` (for windowed apps), paste the example code, then run `make clean && make run` from the project root!
