#include "player.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../drivers/sound.h"

int player_selected_btn = -1;

static app_entry_t player_app = {
    .name = "player",
    .title = "Sound Player",
    .description = "PC Speaker Chiptunes",
    .init = player_init,
    .launch = player_launch,
    .draw = player_draw,
    .handle_key = 0
};

void player_init(void) {
    app_register(&player_app);
}

void player_launch(const char* args) {
    (void)args;
    wm_add_window(100, 200, 420, 240, "Sound Player", player_draw);
    sys_create_task("player", 1);
}

void player_draw(void* self) {
    window_t* w = (window_t*)self;
    
    fb_print("Retro Sound Console", w->x + 20, w->y + 40, 0x882200, 0xC0C0C0);
    
    // Play Chiptune button
    draw_retro_3d_panel(w->x + 20, w->y + 70, 180, 30, player_selected_btn == 0);
    fb_print("Play Chiptune", w->x + 40, w->y + 81, 0x000000, 0xC0C0C0);
    
    // Success Alert button
    draw_retro_3d_panel(w->x + 220, w->y + 70, 180, 30, player_selected_btn == 1);
    fb_print("Success Alert", w->x + 240, w->y + 81, 0x000000, 0xC0C0C0);
    
    // Mute button
    draw_retro_3d_panel(w->x + 20, w->y + 120, 380, 30, player_selected_btn == 2);
    fb_print("Mute Speaker / Stop Sound", w->x + 80, w->y + 131, 0x000000, 0xC0C0C0);
    
    // Instructions
    fb_print("Click a button above to play authentic retro tones!", w->x + 20, w->y + 175, 0x222222, 0xC0C0C0);
    fb_print("Uses motherboard timer chip Channel 2 (8253 PIT).", w->x + 20, w->y + 195, 0x222222, 0xC0C0C0);
}
