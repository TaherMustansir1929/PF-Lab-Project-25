#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

/*
 * ANSI Escape Codes for Terminal Color and Style
 *
 * Usage:
 * printf(ANSI_FG_RED "This is red text." ANSI_RESET "\n");
 * printf(ANSI_BOLD ANSI_BG_BLUE "Bold text on blue background." ANSI_RESET "\n");
 */

// --- 1. Reset/Mode Constants ---
// Resets all colors and styles to the terminal default. ESSENTIAL to end a color sequence.
#define ANSI_RESET          "\x1b[0m"

// Text Styles
#define ANSI_BOLD           "\x1b[1m"   // Bold or High Intensity
#define ANSI_DIM            "\x1b[2m"   // Dim or Low Intensity
#define ANSI_UNDERLINE      "\x1b[4m"
#define ANSI_BLINK          "\x1b[5m"
#define ANSI_INVERT         "\x1b[7m"   // Invert foreground and background
#define ANSI_HIDDEN         "\x1b[8m"   // Conceal text


// --- 2. Standard Foreground Colors (Text Color) ---
#define ANSI_FG_BLACK       "\x1b[30m"
#define ANSI_FG_RED         "\x1b[31m"
#define ANSI_FG_GREEN       "\x1b[32m"
#define ANSI_FG_YELLOW      "\x1b[33m"
#define ANSI_FG_BLUE        "\x1b[34m"
#define ANSI_FG_MAGENTA     "\x1b[35m"
#define ANSI_FG_CYAN        "\x1b[36m"
#define ANSI_FG_WHITE       "\x1b[37m"


// --- 3. Bright Foreground Colors (High Intensity Text) ---
#define ANSI_BRIGHT_FG_BLACK    "\x1b[90m"
#define ANSI_BRIGHT_FG_RED      "\x1b[91m"
#define ANSI_BRIGHT_FG_GREEN    "\x1b[92m"
#define ANSI_BRIGHT_FG_YELLOW   "\x1b[93m"
#define ANSI_BRIGHT_FG_BLUE     "\x1b[94m"
#define ANSI_BRIGHT_FG_MAGENTA  "\x1b[95m"
#define ANSI_BRIGHT_FG_CYAN     "\x1b[96m"
#define ANSI_BRIGHT_FG_WHITE    "\x1b[97m"


// --- 4. Standard Background Colors ---
#define ANSI_BG_BLACK       "\x1b[40m"
#define ANSI_BG_RED         "\x1b[41m"
#define ANSI_BG_GREEN       "\x1b[42m"
#define ANSI_BG_YELLOW      "\x1b[43m"
#define ANSI_BG_BLUE        "\x1b[44m"
#define ANSI_BG_MAGENTA     "\x1b[45m"
#define ANSI_BG_CYAN        "\x1b[46m"
#define ANSI_BG_WHITE       "\x1b[47m"


// --- 5. Bright Background Colors (High Intensity Background) ---
#define ANSI_BRIGHT_BG_BLACK    "\x1b[100m"
#define ANSI_BRIGHT_BG_RED      "\x1b[101m"
#define ANSI_BRIGHT_BG_GREEN    "\x1b[102m"
#define ANSI_BRIGHT_BG_YELLOW   "\x1b[103m"
#define ANSI_BRIGHT_BG_BLUE     "\x1b[104m"
#define ANSI_BRIGHT_BG_MAGENTA  "\x1b[105m"
#define ANSI_BRIGHT_BG_CYAN     "\x1b[106m"
#define ANSI_BRIGHT_BG_WHITE    "\x1b[107m"

#endif // ANSI_COLORS_H
