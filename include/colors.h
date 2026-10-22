#define _POSIX_C_SOURCE 200809L   /* isatty, fileno */
#ifndef COLORS_H
#define COLORS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int color = 0;

#define COLOR_BOLD   "\033[1m"
#define COLOR_RESET  "\033[0m"
#define COLOR_BLACK  "\033[1;30m"
#define COLOR_RED    "\033[1;31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[1;34m"
#define COLOR_PURPLE "\033[1;35m"
#define COLOR_CYAN   "\033[1;36m"
#define COLOR_WHITE  "\033[1;37m"

#define C_BOLD   (color ? COLOR_BOLD   : "")
#define C_RESET  (color ? COLOR_RESET  : "")
#define C_BLACK  (color ? COLOR_BLACK  : "")
#define C_RED    (color ? COLOR_RED    : "")
#define C_GREEN  (color ? COLOR_GREEN  : "")
#define C_YELLOW (color ? COLOR_YELLOW : "")
#define C_BLUE   (color ? COLOR_BLUE   : "")
#define C_PURPLE (color ? COLOR_PURPLE : "")
#define C_CYAN   (color ? COLOR_CYAN   : "")
#define C_WHITE  (color ? COLOR_WHITE  : "")

static inline void use_color(void)
{
  color = isatty(fileno(stderr)) && !getenv("NO_COLOR");
}

#endif /* COLORS_H */
