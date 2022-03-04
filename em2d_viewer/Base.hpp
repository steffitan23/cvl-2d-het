/*
 * Base.hpp
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


#ifndef BASE_HPP
#define BASE_HPP


#include <vector>
#include <iomanip>

#include "../em2d_cluster/Config.hpp"
#include "Display.hpp"
#include "WorkVisual.hpp"


class Base
{
public:
    Base();
    ~Base();

    void prepare_disp(Config *conf, Display *disp);
    void loop_disp();

    void show_main();

    void togglePlay();
    void toggleVideo();

    void increase_wait();
    void decrease_wait();

    void doinit_dot();
    void doinit_spirale();
    void doinit_stimulation();
    void doinit_chaos();
    void doinit_random();
    void doinit_stateload();
    
    void play_data();

    Display *meDisp;
    WorkVisual *meDiffRe;

    int mouse_state,
        mouse_beg[2];

    double
        now_shift[2],
        pre_shift[2],

        fcoord[5];

private:
    Config  *meConf;

    int quit, count,
        act_play,
        act_pace,   // initial value to control update speed
        act_wait,   // running variable to control update speed

        shot_curr,
        shot_do,
        shot_intval,
        shot_beg,
        shot_end,
        
        rtime,
        video_play,
        
        video_beg,
        video_end,
        video_int,

        wait_quit,

        stim_curr,
        stim_do,
        stim_intval;
};

extern Base lBase;

void timer_routine(int t);

void show_disp();
void resize_disp(int w, int h);

void key_down(unsigned char key, int x, int y);
void key_up(unsigned char key, int x, int y);

void mouse_down(int button, int state, int x, int y);
void mouse_move(int x, int y);

void joystick(unsigned int buttonmask, int x, int y, int z);

#endif // BASE_HPP

