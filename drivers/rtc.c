// CMOS Real-Time Clock driver for Onken OS
// Ported from BoredOS reference (GPL v3)
#include "rtc.h"
#include "../kernel/kernel.h"
#include "../kernel/string.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static int updating_rtc(void) {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, (uint8_t)reg);
    return inb(CMOS_DATA);
}

void rtc_get_datetime(int *year, int *month, int *day, int *hour, int *minute, int *second) {
    uint8_t last_second, last_minute, last_hour, last_day, last_month, last_year;
    uint8_t registerB;

    // Wait until RTC is not updating
    while (updating_rtc());
    *second = get_rtc_register(0x00);
    *minute = get_rtc_register(0x02);
    *hour   = get_rtc_register(0x04);
    *day    = get_rtc_register(0x07);
    *month  = get_rtc_register(0x08);
    *year   = get_rtc_register(0x09);
    
    // Read twice to ensure stable values
    do {
        last_second = *second;
        last_minute = *minute;
        last_hour   = *hour;
        last_day    = *day;
        last_month  = *month;
        last_year   = *year;

        while (updating_rtc());
        *second = get_rtc_register(0x00);
        *minute = get_rtc_register(0x02);
        *hour   = get_rtc_register(0x04);
        *day    = get_rtc_register(0x07);
        *month  = get_rtc_register(0x08);
        *year   = get_rtc_register(0x09);
    } while (last_second != *second || last_minute != *minute || last_hour != *hour ||
             last_day != *day || last_month != *month || last_year != *year);

    registerB = get_rtc_register(0x0B);

    // Convert BCD to binary if needed
    if (!(registerB & 0x04)) {
        *second = (*second & 0x0F) + ((*second / 16) * 10);
        *minute = (*minute & 0x0F) + ((*minute / 16) * 10);
        *hour   = ((*hour & 0x0F) + (((*hour & 0x70) / 16) * 10)) | (*hour & 0x80);
        *day    = (*day & 0x0F) + ((*day / 16) * 10);
        *month  = (*month & 0x0F) + ((*month / 16) * 10);
        *year   = (*year & 0x0F) + ((*year / 16) * 10);
    }

    // Convert 12h to 24h if needed
    if (!(registerB & 0x02) && (*hour & 0x80)) {
        *hour = ((*hour & 0x7F) + 12) % 24;
    }

    *year += 2000;
}

void rtc_get_time_string(char* buf, int buf_size) {
    int year, month, day, hour, minute, second;
    rtc_get_datetime(&year, &month, &day, &hour, &minute, &second);
    
    // Format as HH:MM
    if (buf_size < 6) return;
    buf[0] = '0' + (hour / 10);
    buf[1] = '0' + (hour % 10);
    buf[2] = ':';
    buf[3] = '0' + (minute / 10);
    buf[4] = '0' + (minute % 10);
    buf[5] = '\0';
}

void rtc_get_date_string(char* buf, int buf_size) {
    int year, month, day, hour, minute, second;
    rtc_get_datetime(&year, &month, &day, &hour, &minute, &second);
    
    if (buf_size < 11) return;
    // Format as YYYY-MM-DD
    buf[0] = '0' + (year / 1000);
    buf[1] = '0' + ((year / 100) % 10);
    buf[2] = '0' + ((year / 10) % 10);
    buf[3] = '0' + (year % 10);
    buf[4] = '-';
    buf[5] = '0' + (month / 10);
    buf[6] = '0' + (month % 10);
    buf[7] = '-';
    buf[8] = '0' + (day / 10);
    buf[9] = '0' + (day % 10);
    buf[10] = '\0';
}
