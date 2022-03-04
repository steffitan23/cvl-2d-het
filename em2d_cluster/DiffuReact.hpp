/*
 * DiffuReact.hpp
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


#ifndef DIFFUREACT_HPP
#define DIFFUREACT_HPP


#include <vector>
#include <fstream>
#include <sstream>

#include "matr.h"
#include "Config.hpp"



struct Cell
{
    double u;           // excitation
    double v;           // recovery
    double ta;          // dat Ding halt
    double is;          // das andere Ding

    double area;        // surface area

    // Jan Lebert: local electrophysiological parameters
    double val_a;
    double val_k;
    //double val_e0;
    //double val_mu1, val_mu2;

    //
    // Order of neightbors
    //
    //     0   1   2
    //
    //     7   m   3
    //
    //     6   5   4
    //

    vect2 orig_pos;     // initial position at the beginning, stays const. after setting once
    vect2 curr_pos;     // current position, calculated by force
    vect2 prev_pos;     // position from previous iteration

    vect2 curr_vel;     // current velocity, calculated after position shift
    vect2 force[8];     // forces in the 8 main vertices

    Cell(): u(0), v(0), ta(0), is(0), area(0) {}
};

struct State_cell
{
    double u;
    double v; 
    double ta;
    vect2 curr_pos;
    vect2 prev_pos;
    vect2 curr_vel;
    
    State_cell(): u(0), v(0), ta(0){}
    void set_dou(const double pu, const double pv, const double pta) {
        u = pu;
        v = pv;
        ta = pta;
    };
};

class DiffuReact
{
public:
    DiffuReact(Config *put_conf);
    ~DiffuReact();

    Cell *getMass(int x, int y);
    double getLap(int x, int y);

    void make_step();

    void updt_Variables(int lx, int ly);
    void updt_Lap();
    
    void rewrite_elec_Lap(float angle);

    void updt_Spring(int lx, int ly);
    void updt_Area(int lx, int ly);
    void updt_Position(int lx, int ly);

    void clearEdge();
    void clearField();
    void clear_field();

    void toogle_mechanic(int mode);
    void toogle_feedback(int mode);

    int getRuntime();
    double get_tdt();
    
    // for loading and writing stuff
    void state_load(const char* put_path);
    void state_save(int rtime);

    void play_data();
private:
    Config  *meConf;

    std::vector<Cell>
        field;          // the mass points itself

    std::vector<double>
        mLap[2];        // running Laplace data


    int area_size,
        field_sum,
        fprev, fnext,
        iterate_mech,

        area_mech,
        area_aniso,
        elec_aniso,
        area_sac,
        run_time,
        area_pad;

    double
        elec_lap[8],
        elec_sub,
        area_home,

        ang_ceta,       // ratio of force for rotation

        currU;


    // ===== the variables field
    double
        val_c,
        val_d,
        val_kap,

        val_mt,
        val_r0,

        zw_r0d,         // average r0 multiplied with square root 2, diagonally
        zw_r0b2;        // in other words (1/r0)^2, to have a normalizer


    // ===== the variables reaction diffusion
    double
        //val_a,
        //val_k,
        val_kT,
        val_diff,          //Diffusionskonstante
        val_e0,
        val_mu1,
        val_mu2,

        val_Gs,
        val_Es,

        val_dt,

        zw_1odt;
        
    void step_thread(std::vector<std::pair<int, int>> points, int thread_id);
};

#endif // DIFFUREACT_HPP
