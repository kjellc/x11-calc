/*
 * x11-calc-display.h - RPN (Reverse Polish) calculator simulator.
 *
 * Copyright(C) 2013   MT
 *
 * 10 Digit digit display functions.
 *
 * Contains  the functions needed to create and display a 10 digit 7 seven
 * segment display element.
 *
 * This  program is free software: you can redistribute it and/or modify it
 * under  the terms of the GNU General Public License as published  by  the
 * Free  Software Foundation, either version 3 of the License, or (at  your
 * option) any later version.
 *
 * This  program  is distributed in the hope that it will  be  useful,  but
 * WITHOUT   ANY   WARRANTY;   without even   the   implied   warranty   of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You  should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 14 Jul 13         - Initial version - MT
 * 17 Aug 13         - Added constants for mask values - MT
 * 10 Mar 14         - Changed  names  of display masks to highlight  their
 *                     association with the display module - MT
 * 30 Aug 20         - Base  the number of display segments on the  maximum
 *                     number of digits in the display - MT
 * 08 Aug 21         - Tidied up spelling errors in the comments - MT
 * 20 Dec 21         - Updated display for HP67 - MT
 * 29 Jan 22         - Added an optional bezel to the display - MT
 * 01 Mar 22         - Only define constants that are required - MT
 * 12 Mar 22         - Added display annunciators - MT
 * 11 Dec 22         - Renamed models with continuous memory and added HP25
 *                     HP33E, and HP38E - MT
 * 26 Oct 23         - Added an enabled property to the display - MT
 * 09 Apr 24         - Removed some unused parameters from display-update()
 *                     function - MT
 *                   - Changed display data structure to store the size and
 *                     position of the bezel and display using a predefined
 *                     XRectangle structure - MT
 *                   - Display structure stores both the original  geometry
 *                     and the current position of the display - MT
 *                   - Finally renamed x11-calc-segment to the more correct
 *                     x11-calc-digit - MT
 * 24 Jun 25         - Fixed storage overflow error display - MT
 * 29 Jun 25         - Added support for European display formats for SPICE
 *                     series - MT
 * 23 Aug 25         - Deleted 'Enabled' property as it isn't needed - MT
 *
 */

#include "x11-calc-cpu.h"

#define DISPLAY_SPACE      0x0000

#if !(defined(HP10c) || defined(HP11c) || defined(HP12c) || defined(HP15c) || defined(HP16c))

#if 1 // kjc: 0 for testing alt font

#define DISPLAY_ONE        0x0030
#define DISPLAY_TWO        0x006d
#define DISPLAY_THREE      0x0079
#define DISPLAY_FOUR       0x0072
#define DISPLAY_FIVE       0x005b
#define DISPLAY_SIX        0x005f
#define DISPLAY_SEVEN      0x0031
#define DISPLAY_EIGHT      0x007f
#define DISPLAY_NINE       0x0073
#define DISPLAY_ZERO       0x003f
#define DISPLAY_MINUS      0x0040
#define DISPLAY_DECIMAL    0x0080
#define DISPLAY_COMMA      0x0100
#define DISPLAY_COLON      0x0200
#define DISPLAY_r          0x0044
#define DISPLAY_c          0x004c
#define DISPLAY_C          0x000f
#define DISPLAY_o          0x005c
#define DISPLAY_d          0x007c
#define DISPLAY_P          0x0067
#define DISPLAY_E          0x004f
#define DISPLAY_F          0x0047
#define DISPLAY_TEST       0x01ff

#else

#include "../temp/seven-seg-ascii.c"

// character testing

#define DISPLAY_ZERO       0x3f
#define DISPLAY_ONE        DISPLAY_x
#define DISPLAY_TWO        DISPLAY_y
#define DISPLAY_THREE      DISPLAY_QM
#define DISPLAY_FOUR       DISPLAY_SPACE
#define DISPLAY_FIVE       0xc8
#define DISPLAY_SIX        0x23
#define DISPLAY_SEVEN      0x6d
#define DISPLAY_EIGHT      DISPLAY_d
#define DISPLAY_NINE       DISPLAY_d

#define DISPLAY_MINUS      0x0040
#define DISPLAY_DECIMAL    0  // 0x0080

#define DISPLAY_COMMA      0x0100
#define DISPLAY_COLON      0x0200

#endif

#if 0
#define DISPLAY_r          DISPLAY_R  //0x0044
#define DISPLAY_c          0x004c
#define DISPLAY_C          DISPLAY_Q  //0x000f
#define DISPLAY_o          0x003f  //0x005c
#define DISPLAY_d          DISPLAY_QM //0x007c
#define DISPLAY_P          0x0067
#define DISPLAY_E          0x004f
#define DISPLAY_TEST       0x01ff

#define DISPLAY_A          0x37
#define DISPLAY_w          0x9c
#define DISPLAY_m          0xd4
#define DISPLAY_x          0x94
#define DISPLAY_k          0x96
#define DISPLAY_z          0xc8
#define DISPLAY_Q          0xbf
#define DISPLAY_V          0xa2
#define DISPLAY_Y          0xe2
#define DISPLAY_QM         0xe1
#define DISPLAY_R          0xe7

#endif

#endif


struct odisplay/* Calculator display structure. */
{
   int index;
   XRectangle bezel_position;    /* Current bezel position */
   XRectangle display_position;  /* Current display position */
   XRectangle bezel_geometry;    /* Original bezel position */
   XRectangle display_geometry;  /* Original display position */
#if defined(HP31e) || defined(HP32e) || defined(HP33e) || defined(HP33c) || defined(HP34c) || defined(HP37e) || defined(HP38e) || defined(HP38c)
   char euro;
#endif
   unsigned int foreground;
   unsigned int background;
   unsigned int fill;
   unsigned int border;
   struct odigit* digit[DIGITS];
#if defined(INDECATORS)
   struct olabel* label[INDECATORS];
#endif
#if defined(HP67)
   struct olabel* label_mnemonic;
#endif
   unsigned char last_a_nibble[REG_SIZE];
} ;

struct odisplay *h_display_create(int i_index,
   int i_left, int i_top, int i_width, int i_height,
   int i_display_left, int i_display_top, int i_display_width, int i_display_height,
   unsigned int i_foreground, unsigned int i_background, unsigned int i_fill, unsigned int i_border);

int i_display_draw(Display *x_display, int x_application_window, int i_screen, struct odisplay *h_display);

int i_display_resize(struct odisplay *h_display, float f_scale);

int i_display_update(struct odisplay *h_display, oprocessor *h_processor);
