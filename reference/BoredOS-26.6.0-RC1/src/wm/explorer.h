// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#ifndef EXPLORER_H
#define EXPLORER_H

#include "wm.h"
#include "fat32.h"
#include <stddef.h>
#include "libwidget.h"

// External windows references (for opening other apps)
extern Window win_explorer;
extern Window win_editor;
extern Window win_notepad;
extern Window win_markdown;

#define EXPLORER_INITIAL_CAPACITY 256
#define DIALOG_INPUT_MAX 256

typedef struct {
    char name[FAT32_MAX_FILENAME];
    bool is_directory;
    uint32_t size;
    uint32_t color;
} ExplorerItem;

typedef struct {
    ExplorerItem *items;
    int items_capacity;
    int item_count;
    int selected_item;
    char current_path[FAT32_MAX_PATH];
    int last_clicked_item;
    uint32_t last_click_time;
    int explorer_scroll_row;

    // Dialog state
    int dialog_state;
    char dialog_input[DIALOG_INPUT_MAX];
    int dialog_input_cursor;
    char dialog_target_path[FAT32_MAX_PATH];
    bool dialog_target_is_dir;
    char dialog_dest_dir[FAT32_MAX_PATH];
    char dialog_creation_path[FAT32_MAX_PATH];
    char dialog_move_src[FAT32_MAX_PATH];

    // Dropdown menu state
    bool dropdown_menu_visible;
    bool drive_menu_visible;
    
    // File context menu state
    bool file_context_menu_visible;
    int file_context_menu_x;
    int file_context_menu_y;
    int file_context_menu_item;

    // GUI widgets
    widget_button_t btn_primary;
    widget_button_t btn_secondary;
    widget_button_t btn_dropdown;
    widget_button_t btn_back;
    widget_button_t btn_up;
    widget_button_t btn_fwd;

    widget_textbox_t dialog_textbox;

} ExplorerState;

void explorer_init(void);
void explorer_reset(void);
void explorer_open_directory(const char *path);
void explorer_open_target(const char *path); 


bool explorer_get_file_at(int screen_x, int screen_y, char *out_path, bool *is_dir);
void explorer_import_file(Window *win, const char *source_path); // To focused or default
void explorer_import_file_to(Window *win, const char *source_path, const char *dest_dir);
void explorer_refresh(Window *win);
void explorer_refresh_all(void);
void explorer_clear_click_state(Window *win);

size_t explorer_strlen(const char *str);
void explorer_strcpy(char *dest, const char *src);
void explorer_strcat(char *dest, const char *src);

void explorer_clipboard_copy(const char *path);
void explorer_clipboard_cut(const char *path);
void explorer_clipboard_paste(Window *win, const char *dest_dir);
bool explorer_clipboard_has_content(void);

bool explorer_delete_permanently(const char *path);
bool explorer_delete_recursive(const char *path);
void explorer_create_shortcut(Window *win, const char *target_path);

#endif