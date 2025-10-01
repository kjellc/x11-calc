/*
 * x11-calc-cpu.h - RPN (Reverse Polish) calculator simulator.
 *
 * Copyright(C) 2024  Kjellc
 *
 * Implements the card reader for HP67
 * The file chooser part uses the GTK libraries
 *
 */

#ifndef X11_CALC_CARD_67_H
#define X11_CALC_CARD_67_H

#include <stdio.h>
#include "x11-calc-cpu.h"

void v_card_read_write_record(oprocessor* h_processor);
void v_card_open_file(oprocessor* h_processor);
void v_card_close_file(oprocessor* h_processor);

#endif // X11_CALC_CARD_67_H
