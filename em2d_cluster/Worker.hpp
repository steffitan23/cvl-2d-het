/*
 * Worker.hpp
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


#ifndef WORKER_HPP
#define WORKER_HPP

#include <cstdlib>
#include <ctime>

#include <vector>

#include <fstream>
#include <ctime>
#include <iomanip>

#include <sstream>
#include <string>

#include "Config.hpp"
#include "DiffuReact.hpp"


class Worker
{
public:
    Worker(Config* conf);
    ~Worker();

    void run_batch();

    void make_dot(int x, int y);
    void make_vdot(int x, int y);
    
    void make_stimulation();
    void hold_stimulation(int rtime);

    void make_spirale(int x, int y);
    void hold_spirale();

    void make_chaos();
    void hold_chaos();
    
    void make_random();
    void hold_random(int rtime);
    
    void make_stateload();
    void hold_stateload();
    
    void interval(int nowtime);
    
    void make_load_and_calc();
    
    void save_data(int rtime);
    
    DiffuReact  *meModel;

protected:
    void log_prep();
    void log_tap(int nowtime);
    void log_stop();

    void sac_prep();
    void sac_tap(int nowtime);
    void sac_stop();

    void tea_prep();
    void tea_tap(int nowtime);
    void tea_stop();
    
    void act_prep();
    void act_tap(int nowtime);
    void act_stop();
    
    double tea_phadiff(double a, double b);

    int area_size,
        area_pad,
        plan_size,

        spi_do,
        stim_do,
        cho_do,
        ran_do,
        state_do,
        seed,
        
        tea_set,
        sac_set,
        log_set,
        act_scan,
        tea_time,
        mech_int,
        mech_int_time,
        save_run,
        comp_time,
        
        tea_run,
        sac_run,
        log_run,
        act_run,
        act_count,
        act_end,

        tea_beg,
        tea_end,
        tea_cmin,
        tea_tau,
        tea_pivot,
        tea_count,
        tea_max,
        mech,
        sac_wait_int,

        sac_end,
        sac_maxnum,
        sac_trig,

        log_ptx,
        log_pty,
        log_end,

        log_beg,
        log_step,

        spi_px,
        spi_py,
        spi_wait,

        cho_wait,

        mech_wait,
        sac_wait,
        init_mode,
        
        video_beg,
        video_end,
        video_int;

    double
        sac_thresh,
        sac_prev,
        tea_mean,
        sac_min;
        

    std::vector<int4>
        cho_list;

    std::vector<int>
        tea_phas2;

    std::vector<double>
        log_data,
        sac_data,
        tea_data,
        tea_phas;

    Config      *meConf;
};

#endif // WORKER_HPP
