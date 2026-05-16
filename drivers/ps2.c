#include "ps2.h"
#include "../kernel/kernel.h"
#include "../gui/fb.h"
#include "../kernel/interrupts/idt.h"

int32_t mouse_x = 512;
int32_t mouse_y = 384;
uint8_t mouse_b_left = 0;
uint8_t mouse_b_right = 0;

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];
static char last_key = 0;
static int alt_pressed = 0;
static int ctrl_pressed = 0;


static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',  0,  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0, '-',   0,   0,   0, '+',   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0
};

static int ps2_wait_write() {
    uint32_t timeout = 100000;
    while (timeout--) {
        if ((inb(0x64) & 2) == 0) return 0;
    }
    return -1;
}

static int ps2_wait_read() {
    uint32_t timeout = 100000;
    while (timeout--) {
        if (inb(0x64) & 1) return 0;
    }
    return -1;
}

static void mouse_write(uint8_t a_write) {
    ps2_wait_write();
    outb(0x64, 0xD4);
    ps2_wait_write();
    outb(0x60, a_write);
}

static uint8_t mouse_read() {
    if (ps2_wait_read() != 0) return 0;
    return inb(0x60);
}

void ps2_init(void) {
    // Enable Second PS/2 port
    ps2_wait_write();
    outb(0x64, 0xA8);

    // Get Compaq Status
    ps2_wait_write();
    outb(0x64, 0x20);
    ps2_wait_read();
    uint8_t status = inb(0x60);
    
    // Enable IRQ12, IRQ1
    status |= (1 << 1);
    status &= ~(1 << 5);
    
    ps2_wait_write();
    outb(0x64, 0x60);
    ps2_wait_write();
    outb(0x60, status);

    // Tell mouse to use default settings
    mouse_write(0xF6);
    mouse_read();

    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();
}

static void handle_mouse(uint8_t data) {
    switch (mouse_cycle) {
        case 0:
            if (data & 0x08) {
                mouse_byte[0] = data;
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            
            // Apply movement
            int dx = mouse_byte[1];
            int dy = mouse_byte[2];
            
            if (mouse_byte[0] & 0x10) dx |= ~0xFF;
            if (mouse_byte[0] & 0x20) dy |= ~0xFF;
            
            mouse_x += dx;
            mouse_y -= dy;
            
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > (int32_t)fb_width - 1) mouse_x = fb_width - 1;
            if (mouse_y > (int32_t)fb_height - 1) mouse_y = fb_height - 1;
            
            mouse_b_left = mouse_byte[0] & 0x01;
            mouse_b_right = mouse_byte[0] & 0x02;
            
            mouse_cycle = 0;
            break;
    }
}

static void handle_kbd(uint8_t data) {
    if (data == 0x38) {
        alt_pressed = 1;
    } else if (data == 0xB8) {
        alt_pressed = 0;
    } else if (data == 0x1D) {
        ctrl_pressed = 1;
    } else if (data == 0x9D) {
        ctrl_pressed = 0;
    } else if (data < 128) {
        last_key = kbd_us[data];
    }
}

void ps2_poll(void) {
    // No-op! Fully handled by hardware interrupts (irq1_handler and irq12_handler) now.
}

char ps2_get_last_key(void) {
    char k = last_key;
    last_key = 0; // Clear it after reading
    return k;
}

int ps2_is_alt_pressed(void) {
    return alt_pressed;
}

int ps2_is_ctrl_pressed(void) {
    return ctrl_pressed;
}

// C Interrupt Handler for Keyboard (IRQ 1)
void irq1_handler(void* stack_frame) {
    (void)stack_frame;
    uint8_t status = inb(0x64);
    if (status & 1) {
        uint8_t data = inb(0x60);
        if (!(status & 0x20)) {
            handle_kbd(data);
        }
    }
    // Send EOI to PIC (IRQ 1)
    pic_send_eoi(1);
}

// C Interrupt Handler for Mouse (IRQ 12)
void irq12_handler(void* stack_frame) {
    (void)stack_frame;
    uint8_t status = inb(0x64);
    if (status & 1) {
        uint8_t data = inb(0x60);
        if (status & 0x20) {
            handle_mouse(data);
        }
    }
    // Send EOI to PIC (IRQ 12 on slave PIC)
    pic_send_eoi(12);
}

