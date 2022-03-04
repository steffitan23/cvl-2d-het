/*
 * color.cpp
 *
 * Copyright 2018 Tung Thai <>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include "color.h"


/**
 * @brief A customized colormap which goes from black to green, then yellow and then red, from 0 to 1
 * @param v_col [red, green, blue]
 * @param value
 */
void color_elec_dark(double v_col[], double value)
{
    if (value <= 0.1) {
        v_col[0] = 0;
        v_col[1] = value * 10.0;
    }
    else if (value > 0.1 && value <= 0.9) {
        v_col[0] = (value-0.1) * 1.25;
        v_col[1] = 1;
    }
    else {
        v_col[1] = 1.0 - (value-0.9) * 10.0;
    }
    v_col[2] = 0;
}


/**
 * @brief A customized colormap which goes from white to green, then yellow and then red, from 0 to 1
 * @param v_col [red, green, blue]
 * @param value
 */
void color_elec_bright (double v_col[], double value)
{
    if (value <= 0.25) {
        v_col[1] = 1.0;
        v_col[0] = v_col[2] = 1.0 - value * 4.0;
    }
    else if (value > 0.25 && value <= 0.75) {
        v_col[0] = (value-0.25) * 2.0;
        v_col[1] = 1.0;
        v_col[2] = 0;
    }
    else {
        v_col[0] = 1.0;
        v_col[1] = 1.0 - (value-0.75) * 4.0;
        v_col[2] = 0;
    }
}

