<div align="center">
  <h1>Widget API (<code>libwidget.h</code>)</h1>
  <p><em>High-level UI components for BoredOS applications.</em></p>
</div>

---

The Widget library (`libwidget.h`) provides a set of reusable UI components built on top of `libui.h`. It uses an abstract `widget_context_t` to decouple component logic from specific drawing implementations, making it easier to build complex graphical interfaces.

## Widget Context

To use any widget, you must first define a `widget_context_t`. This structure contains function pointers for basic drawing operations (rects, strings) and theme preferences.

```c
typedef struct {
    void *user_data;
    void (*draw_rect)(void *user_data, int x, int y, int w, int h, uint32_t color);
    void (*draw_rounded_rect_filled)(void *user_data, int x, int y, int w, int h, int r, uint32_t color);
    void (*draw_string)(void *user_data, int x, int y, const char *str, uint32_t color);
    int (*measure_string_width)(void *user_data, const char *str);
    void (*mark_dirty)(void *user_data, int x, int y, int w, int h);
    bool use_light_theme;
} widget_context_t;
```

> [!TIP]
> Usually, `user_data` is set to your `ui_window_t` handle, and the functions are simple wrappers around `ui_draw_rect`, `ui_draw_string`, etc.

---

## Button (`widget_button_t`)

Standard interactive button with hover and click states.

*   `void widget_button_init(widget_button_t *btn, int x, int y, int w, int h, const char *text);`
*   `void widget_button_draw(widget_context_t *ctx, widget_button_t *btn);`
*   `bool widget_button_handle_mouse(widget_button_t *btn, int mx, int my, bool mouse_down, bool mouse_clicked, void *user_data);`

### Usage Example:
```c
widget_button_t my_btn;
widget_button_init(&my_btn, 10, 10, 80, 25, "Click Me");
my_btn.on_click = my_callback_func;

// In your event loop:
widget_button_handle_mouse(&my_btn, ev.arg1, ev.arg2, is_down, is_clicked, my_data);
```

---

## Scrollbar (`widget_scrollbar_t`)

Vertical scrollbar supporting dragging and track-paging.

*   `void widget_scrollbar_init(widget_scrollbar_t *sb, int x, int y, int w, int h);`
*   `void widget_scrollbar_update(widget_scrollbar_t *sb, int content_height, int scroll_y);`
*   `void widget_scrollbar_draw(widget_context_t *ctx, widget_scrollbar_t *sb);`
*   `bool widget_scrollbar_handle_mouse(widget_scrollbar_t *sb, int mx, int my, bool mouse_down, void *user_data);`

> [!NOTE]
> The scrollbar automatically calculates the "thumb" size based on the ratio of `h` to `content_height`.

--- 

## TextBox (`widget_textbox_t`)

Editable text field with focus support and keyboard handling.

*   `void widget_textbox_init(widget_textbox_t *tb, int x, int y, int w, int h, char *buffer, int max_len);`
*   `void widget_textbox_draw(widget_context_t *ctx, widget_textbox_t *tb);`
*   `bool widget_textbox_handle_mouse(widget_textbox_t *tb, int mx, int my, bool mouse_clicked, void *user_data);`
*   `bool widget_textbox_handle_key(widget_textbox_t *tb, char c, void *user_data);`

---

## Dropdown (`widget_dropdown_t`)

Selection menu for picking one item from a list.

*   `void widget_dropdown_init(widget_dropdown_t *dd, int x, int y, int w, int h, const char **items, int count);`
*   `void widget_dropdown_draw(widget_context_t *ctx, widget_dropdown_t *dd);`
*   `bool widget_dropdown_handle_mouse(widget_dropdown_t *dd, int mx, int my, bool mouse_clicked, void *user_data);`

---

## Checkbox / Radio (`widget_checkbox_t`)

Toggleable options with support for circular "Radio" style or square "Checkbox" style.

*   `void widget_checkbox_init(widget_checkbox_t *cb, int x, int y, int w, int h, const char *text, bool is_radio);`
*   `void widget_checkbox_draw(widget_context_t *ctx, widget_checkbox_t *cb);`
*   `bool widget_checkbox_handle_mouse(widget_checkbox_t *cb, int mx, int my, bool mouse_clicked, void *user_data);`

---

## Event Integration

Widgets are designed to be polled within your `libui` event loop. Most handle-mouse functions return `true` if the event was "consumed" by the widget, allowing you to stop further processing for that event.

```c
if (ui_get_event(win, &ev)) {
    bool handled = false;
    handled |= widget_button_handle_mouse(&btn, ev.arg1, ev.arg2, is_down, is_clicked, NULL);
    if (!handled) {
        // Handle global window events...
    }
}
```
