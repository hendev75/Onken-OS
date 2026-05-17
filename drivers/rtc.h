#pragma once
#include <stdint.h>

void rtc_get_datetime(int *year, int *month, int *day, int *hour, int *minute, int *second);
void rtc_get_time_string(char* buf, int buf_size);
void rtc_get_date_string(char* buf, int buf_size);
