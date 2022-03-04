/*
 * WorkVisual.hpp
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


#ifndef WORKVISUAL_HPP
#define WORKVISUAL_HPP


#include "color.h"
#include "Display.hpp"

#include "../em2d_cluster/Worker.hpp" // Base class: Worker
#include "../em2d_cluster/Config.hpp"


class WorkVisual : public Worker
{
public:
    WorkVisual(Config *conf, Display *disp);

    void init_field();
    void init_resti();
    void init_stretch();
    void init_plot();
    void init_iactive();
    void init_phase();

    void draw_field(int* x, int* y);
    void draw_resti(int* x, int* y);
    void draw_stretch(int* x, int* y);
    void draw_plot(int* x, int* y);
    void draw_iactive(int* x, int* y);
    void draw_phase(int* x, int* y);

    void draw_option(int* x, int* y, int rtime);

    void updt_field();
    void updt_resti();
    void updt_stretch();
    void updt_plot();
    void updt_iactive();
    void updt_phase();

    void run_step();
    void run_video_step();

    void tog_mechanic();
    void tog_feedback();
    void clear_field();

    double get_scale();

private:

    int area_size,
        grid_span,

        log_pivo,
        log_last;

    double
        norm_size,
        ang_cos,
        ang_sin,

        log_max,
        resti_max,

        stretch_min,
        stretch_max,

        iactive_max,

        phase_min,
        phase_max;

    std::vector<vect3>      // [0] are colors, and [1] are actual lines
        vc_field[2],        // electrical field
        vc_resti[2],        // restitute field
        vc_stretch[2],      // mechanical stretch tension
        vc_grid[2],         // grid, decoration
        vc_plot[2],
        vc_iactive[2],      // what I_s results over the field
        vc_phase[2];        // Phase

    std::vector<double>
        plot_dat[3],
        sac_data;

    Display     *meDisp;
    Config      *meConf;
};

#endif // WORKVISUAL_HPP

