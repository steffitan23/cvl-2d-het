/*
 * Config.hpp
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


#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <iostream>
#include <fstream>
#include <string>

class Config
{
public:
    Config(int argc, char **argv);
    ~Config();

    void prepare(const char *put_path);
    void show();

    int
    // [Viewer]
        scr_sizew,
        scr_sizeh,
        plot_last,

    // [Basic]
        area_size,
        area_pad,

        area_mech,
        area_edge,          // which mode? 0 not fixed, 1 corner fixed, 2 edge fixed
        area_sac,

        area_aniso,         // Enable mechanic anisotropy?
        elec_aniso,         // Electric anisotropy also?

        wait_mech,
        wait_quit,

        shot_do,
        shot_intval,
        shot_beg,
        shot_end,

        stim_do,
        stim_intval,

        log_ptx,
        log_pty,

        tea_set,
        log_set,
        sac_set,
        act_scan,
        act_end,
        seed,
        comp_time,
        
        video_beg,
        video_end,
        video_int,
        
        AP_model,

        log_step,
        log_span[2],

        sac_span,           // up until how many tsp?
        sac_maxnum,         // how many hills should be detected until close?
        
        tea_time,
        mech_int,
        mech_int_time,
        save_run,
        
        tea_beg,
        tea_end,
        tea_cmin,

        term_quiet,

        init_sigx,
        init_sigy,
        init_mode,
        
        stim_px,
        stim_py;
        
        

    double
        elec_lap[8],

        area_home,
        area_angle,         // the angle of the anisotropy

        sac_thresh;


    double
    // [Electrophysiology]
        val_a,
        val_k,
        val_diff,
        val_e0,
        val_mu1,
        val_mu2,
        val_kT,
        val_Gs,
        val_Es,

    // [Mechanics]
        val_c,
        val_d,
        val_kap,

    // [Integration]
        val_dt,
        val_dx,
        val_mt;

    int conf_loaded;

private:
    std::string clean_str(std::string put_str);
    std::string infopath;   // path as string to info file
};

#endif // CONFIG_HPP


