<div align="center">
  <h1>UI API (<code>libui.h</code>)</h1>
  <p><em>Comprehensive manual for interacting with the Window Manager.</em></p>
</div>

---

The UI library (`libui.h`) is the sole mechanism for Graphical Userland Applications to draw to the screen and receive input events in BoredOS. It wraps `SYS_GUI` kernel calls.

## Window Management

A "Window" is a reserved drawing canvas managed by the compositor.

*   `ui_window_t ui_window_create(const char *title, int x, int y, int w, int h);`
    Creates a new window at `(x, y)` with dimensions `w`x`h`. Returns a window handle.
    **Flags** are currently embedded in the syscall; standard windows include decorations (titlebar, borders).
*   `void ui_window_set_title(ui_window_t win, const char *title);`
    Dynamically update the text displayed in the window's titlebar.
*   `void ui_window_set_resizable(ui_window_t win, bool resizable);`
    Enable or disable the user's ability to resize the window by dragging its edges.
*   `void ui_get_screen_size(uint64_t *out_w, uint64_t *out_h);`
    Query the global screen resolution of the display.

## Drawing Primitives

All drawing functions write to an off-screen buffer associated with the window. **You must call `ui_mark_dirty()` to instruct the compositor to push your changes to the physical screen.**

*   `void ui_draw_rect(ui_window_t win, int x, int y, int w, int h, uint32_t color);`
    Draw a solid filled rectangle.
*   `void ui_draw_rounded_rect_filled(ui_window_t win, int x, int y, int w, int h, int radius, uint32_t color);`
    Fill a rectangle with rounded corners of a specified `radius`.
*   `void ui_draw_image(ui_window_t win, int x, int y, int w, int h, uint32_t *image_data);`
    Blit a raw ARGB pixel buffer (`image_data`) directly into the window canvas.
*   `void ui_mark_dirty(ui_window_t win, int x, int y, int w, int h);`
    Mark a specific rectangular region of the window as "dirty". The Window Manager will redraw this area on the next compositing pass.

> [!TIP]
> Colors are defined as 32-bit unsigned integers in **ARGB** format: `0xAARRGGBB`.
> E.g., `0xFF000000` is opaque black, `0xFFFF0000` is opaque red.

## Text Rendering

BoredOS provides multiple text rendering methodologies, including a default system font and scaled/bitmap alternatives.

*   `void ui_draw_string(ui_window_t win, int x, int y, const char *str, uint32_t color);`
    Draw text using the default system typeface.
*   `void ui_draw_string_bitmap(ui_window_t win, int x, int y, const char *str, uint32_t color);`
    Draw text using a secondary fast bitmap font renderer.
*   `void ui_draw_string_scaled(ui_window_t win, int x, int y, const char *str, uint32_t color, float scale);`
    Draw text scaled up or down by a floating-point multiplier.
*   `void ui_draw_string_scaled_sloped(ui_window_t win, int x, int y, const char *str, uint32_t color, float scale, float slope);`
    Draw scaled text with an italic-like slope/shear applied.
*   `void ui_set_font(ui_window_t win, const char *path);`
    Load and set a custom `.ttf` or bitmap font from the filesystem for this window.

### Font Metrics
Used for calculating layout bounds before drawing:
*   `uint32_t ui_get_string_width(const char *str);`
*   `uint32_t ui_get_font_height(void);`
*   `uint32_t ui_get_string_width_scaled(const char *str, float scale);`
*   `uint32_t ui_get_font_height_scaled(float scale);`

## Event Handling

Applications must continuously poll for events inside an infinite `$while(1)` loop.

*   `bool ui_get_event(ui_window_t win, gui_event_t *ev);`
    Returns `true` if an event was waiting in the queue, populating the `ev` structure. Returns `false` if the queue is empty.

> [!IMPORTANT]
> Because `ui_get_event` is non-blocking, you must call `sleep(ms);` or `sys_system(SYSTEM_CMD_SLEEP, ms, ...)` inside your event loop if no event was received. 
> 
> Historically, BoredOS used `sys_yield()`, but in the **Multi-Core (SMP)** architecture, yielding alone will still pin a CPU core to 100% usage. Using a short sleep (e.g., 5-10ms) ensures your app remains responsive while allowing the CPU to actually idle.

### Graphical Event Structure

```c
typedef struct {
    int type; // Specifies the event class (see below)
    int arg1; // Generic argument 1
    int arg2; // Generic argument 2
    int arg3; // Generic argument 3
} gui_event_t;
```

### Event Types & Arguments

| Event Constant | `type` ID | Trigger | `arg1` | `arg2` | `arg3` |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `GUI_EVENT_NONE` | `0` | Empty event | - | - | - |
| `GUI_EVENT_PAINT` | `1` | Window needs redrawing | - | - | - |
| `GUI_EVENT_CLICK` | `2` | Mouse click down | X Coord | Y Coord | Button State |
| `GUI_EVENT_RIGHT_CLICK` | `3` | Mouse right-click down | X Coord | Y Coord | Button State |
| `GUI_EVENT_CLOSE` | `4` | User clicked 'X' button | - | - | - |
| `GUI_EVENT_KEY` | `5` | Keyboard key pressed | Keycode | Modifiers | - |
| `GUI_EVENT_KEYUP` | `10` | Keyboard key released | Keycode | Modifiers | - |
| `GUI_EVENT_MOUSE_DOWN` | `6` | Generic mouse button down | X Coord | Y Coord | Button State |
| `GUI_EVENT_MOUSE_UP` | `7` | Generic mouse button release | X Coord | Y Coord | Button State |
| `GUI_EVENT_MOUSE_MOVE` | `8` | Mouse cursor moved | X Coord | Y Coord | - |
| `GUI_EVENT_MOUSE_WHEEL` | `9` | Scroll wheel rotated | Scroll Delta | - | - |
| `GUI_EVENT_RESIZE` | `11` | Window dimensions changed| New Width | New Height | - |

*(Note: Coordinate arguments (`arg1`, `arg2`) for mouse events are typically relative to the top-left corner of the window's client area).*

---

> [!TIP]
> **Looking for Buttons, TextBoxes, or Scrollbars?**
> While `libui.h` provides the foundation for drawing, most applications should use the higher-level [**Widget API**](widget_api.md) (`libwidget.h`) for standard interactive components.

---
