#pragma once

#include <stdint.h>

#define FONT8X8_FIRST 0x20 /* ' ' */
#define FONT8X8_LAST  0x7E /* '~' */

/** Printable ASCII, 8 bytes per glyph, one byte per row (bit 0 = leftmost pixel). */
extern const uint8_t font8x8_basic[FONT8X8_LAST - FONT8X8_FIRST + 1][8];
