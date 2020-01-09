/* GPLv2 (c) Airbus */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <types.h>
#include <print.h>

#define S_RST       "\x1b[0m"

#define S_NORMAL    "\x1b[0m"
#define S_BOLD      "\x1b[1m"
#define S_DIM       "\x1b[2m"
#define S_ITALIC    "\x1b[3m"
#define S_UNDERLINE "\x1b[4m"
#define S_BLINK     "\x1b[5m"
#define S_REVERSE   "\x1b[7m"
#define S_INVISIBLE "\x1b[8m"

#define C_RED       "\x1b[31m"
#define C_GREEN     "\x1b[32m"
#define C_YELLOW    "\x1b[33m"
#define C_BLUE      "\x1b[34m"
#define C_MAGENTA   "\x1b[35m"
#define C_CYAN      "\x1b[36m"

#define debug(format,...) printf(format, ## __VA_ARGS__)
void stack_trace(offset_t);

#define debug_color(color,format,...) debug(color format S_RST,  ##__VA_ARGS__)
#define debug_red(format,...)     debug_color(C_RED,     format, ##__VA_ARGS__)
#define debug_green(format,...)   debug_color(C_GREEN,   format, ##__VA_ARGS__)
#define debug_yellow(format,...)  debug_color(C_YELLOW,  format, ##__VA_ARGS__)
#define debug_blue(format,...)    debug_color(C_BLUE,    format, ##__VA_ARGS__)
#define debug_magenta(format,...) debug_color(C_MAGENTA, format, ##__VA_ARGS__)
#define debug_cyan(format,...)    debug_color(C_CYAN,    format, ##__VA_ARGS__)

#endif
