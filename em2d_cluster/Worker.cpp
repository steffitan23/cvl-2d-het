/*
 * Worker.cpp
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


#include "Worker.hpp"
#include "stdlib.h"
#include "time.h"
#include "fstream"
#include "sys/stat.h"


Worker::Worker(Config* conf)
{
    meConf      = conf;
    meModel     = new DiffuReact(meConf);

    area_size   = meConf->area_size;
    area_pad    = meConf->area_pad;
    plan_size   = area_size * area_size;

    spi_do      = 0;
    stim_do     = 0;
    cho_do      = 0;
    ran_do      = 0;
    state_do    = 0;

    sac_end     = meConf->sac_span;
    sac_thresh  = meConf->sac_thresh;
    sac_maxnum  = meConf->sac_maxnum;

    tea_beg     = meConf->tea_beg;
    tea_end     = meConf->tea_end;
    tea_cmin    = meConf->tea_cmin;

    log_beg     = meConf->log_span[0];
    log_end     = meConf->log_span[1];

    log_ptx     = meConf->log_ptx;
    log_pty     = meConf->log_pty;
    log_step    = meConf->log_step;

    tea_set     = meConf->tea_set;
    sac_set     = meConf->sac_set;
    log_set     = meConf->log_set;
    act_scan    = meConf->act_scan;
    act_end     = meConf->act_end;
    tea_time    = meConf->tea_time;
    mech_int    = meConf->mech_int;
    mech_int_time=meConf->mech_int_time;
    save_run    = meConf->save_run;
    seed        = meConf->seed;
    mech        = (meConf->area_mech==1) ? 0 : 1;
    comp_time   = meConf->comp_time;

    tea_run     = 0;
    sac_run     = 0;
    log_run     = 0;
    act_run     = 0;
    act_count   = 0;
    tea_count   = 0;
    tea_max     = 0;
    tea_mean    = 0;
    
    
    if (sac_set) sac_prep();
    if (log_set) log_prep();
    if (tea_set) tea_prep();

    mech_wait   = -1;
    sac_wait    = -1;
    init_mode   = meConf->init_mode;
    sac_wait_int=-1;
    
    video_beg   = meConf->video_beg;
    video_end   = meConf->video_end;
    video_int   = meConf->video_int;
}


Worker::~Worker()
{
    if (sac_set) sac_stop();
    if (log_set) log_stop();
    if (tea_set) tea_stop();
    if (act_scan) act_stop(); // What does this do???

    delete meModel;
}


void Worker::run_batch()
{
    int rtime;

    if (meConf->init_sigx > 0)
        switch (init_mode) {
        case 5:
            make_stimulation();
            if (meConf->term_quiet <= 1)
                std::cout << "# do stimulation" << std::endl;
            break;
        case 4:
            make_stateload();
            if (meConf->term_quiet <= 1)
                std::cout << "# do stateload" << std::endl;
            break;
        case 3:
            make_random();
            if (meConf->term_quiet <= 1)
                std::cout << "# do random" << std::endl;
            break;
        case 2:
            make_spirale(meConf->init_sigx, meConf->init_sigy);
            if (meConf->term_quiet <= 1)
                std::cout << "# do spirale" << std::endl;
            break;
        case 1:
        default:
            //make_dot(meConf->init_sigx, meConf->init_sigy);
            make_dot(meConf->stim_px, meConf->stim_py);
            //make_dot(120, 150);
            if (meConf->term_quiet <= 1)
                std::cout << "# do stimulus" << std::endl;
            break;
        }
    do {       
        
        rtime = meModel->getRuntime();
        
        if (log_run) log_tap(rtime);
        if (sac_run) sac_tap(rtime);
        if (tea_run) tea_tap(rtime);
        if (act_run) act_tap(rtime);
        if (save_run) save_data(rtime);
        if (mech_int) interval(rtime);
        
        if (meConf->term_quiet <= 1 && rtime % 1000 == 0)
            std::cout << "." << std::flush;

        hold_spirale();
        hold_chaos();
        hold_random(rtime);
        hold_stateload();
        hold_stimulation(rtime);
        
        
        meModel->make_step();
    }
    while (sac_run || log_run || tea_run|| act_run || ran_do);

    if (meConf->term_quiet <= 1)
        std::cout << std::endl;

    if (sac_set) sac_stop();
    if (log_set) log_stop();
    if (tea_set) tea_stop();
    if (act_run) act_stop();
}


void Worker::make_dot(int x, int y)
{
    //mech_wait = 0;
    if (x >= 2 && x < area_size-2 && y >= 2 && y < area_size-2) {

        double dot_size = 4; //oneo2s2 = 1.0 / (std::sqrt(2 * dot_size * dot_size));

        for (int i = -dot_size; i <= dot_size; i++){
            for (int j = -dot_size; j <= dot_size; j++){
                if(std::sqrt(i*i + j*j)<=dot_size){
                    meModel->getMass(x+i, y+j)->u = 0.3;  //- std::sqrt(i*i + j*j) * oneo2s2;
                }
            }
        }
    } else if (meConf->term_quiet <= 1) {
        std::cout << "- uhm, " << x << "_" << y << " is out of bounds?" << std::endl;
    }
}

void Worker::make_vdot(int x, int y)
{
    if (x >= 2 && x < area_size-2 && y >= 2 && y < area_size-2) {

        double dot_size = 4; //oneo2s2 = 1.0 / (std::sqrt(2 * dot_size * dot_size));

        for (int i = -dot_size; i <= dot_size; i++){
            for (int j = -dot_size; j <= dot_size; j++){
                if(1){
                    meModel->getMass(x+i, y+j)->v = 1;  
                }
            }
        }
    } else if (meConf->term_quiet <= 1) {
        std::cout << "- uhm, " << x << "_" << y << " is out of bounds?" << std::endl;
    }
}

void Worker::make_stimulation()
{
    stim_do = 1;
    make_dot(area_size/2,area_size/2);
}

void Worker::hold_stimulation(int rtime)
{
    if (!stim_do)
        return;
    if(rtime%20000==0){    
        make_dot(area_size/2,area_size/2);
    }
}

void Worker::make_spirale(int x, int y)
{
    
    //cc Bereich im Uhrzeigersinn
    if(meConf->AP_model==1996){
        for(int i=0; i<area_size/2; i++){
            for(int j=area_size/2-2*area_size/10+59; j<area_size/2+59; j++){
                meModel->getMass(i, j)->v = 2;
            }
        }
        for(int i=0; i<area_size/2; i++){
            for(int j=area_size/2+2*area_size/10+59; j>area_size/2+59; j--){
                meModel->getMass(i, j)->u = 0.8;
            }
        }
        spi_wait=800;
    }
    
    //cc Bereich gegen Uhrzeigersinn
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2; i++){
//            for(int j=area_size/2-30; j<area_size/2-10; j++){
//                meModel->getMass(i, j)->u = 0.8;
//            }
//        }
//        for(int i=0; i<area_size/2; i++){
//            for(int j=area_size/2+10; j>area_size/2-15; j--){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//        spi_wait=800;
//    }
    
    //ll Bereich im Uhrzeigersinn
//        if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2+2*area_size/10; i++){
//            for(int j=area_size/2; j<area_size/2+2*area_size/10; j++){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//        for(int i=0; i<area_size/2+area_size/10; i++){
//            for(int j=area_size/2+3*area_size/10; j>area_size/2+2*area_size/10; j--){
//                meModel->getMass(i, j)->u = 0.8;
//            }
//        }
//        spi_wait=2000;
//    }
    
    //ll Bereich gegen Uhrzeigersinn
//        if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2+2*area_size/10; i++){
//            for(int j=area_size/2-4*area_size/10; j<area_size/2-2*area_size/10; j++){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//        for(int i=0; i<area_size/2+area_size/10; i++){
//            for(int j=area_size/2-4*area_size/10; j>area_size/2-5*area_size/10; j--){
//                meModel->getMass(i, j)->u = 0.8;
//            }
//        }
//        spi_wait=2000;
//    }
    
    //kl Bereich gegen Uhrzeigersinn
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2-20; i++){
//            for(int j=area_size/2-35; j<area_size/2-15; j++){
//                meModel->getMass(i, j)->u = 0.8;
//            }
//        }
//        for(int i=0; i<area_size/2-20; i++){
//            for(int j=area_size/2+20; j>area_size/2-20; j--){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//        spi_wait=800;
//    }
    
    //kl Bereich
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2+2*area_size/10; i++){
//            for(int j=area_size/2-2*area_size/10; j<area_size/2; j++){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//        for(int i=0; i<area_size/2+area_size/10; i++){
//            for(int j=area_size/2+area_size/10; j>area_size/2-area_size/10; j--){
//                meModel->getMass(i, j)->u = 0.8;
//            }
//        }
//        spi_wait=2400;
//    }
    
    else{
        spi_px = x;
        spi_py = y;

        spi_wait = 6.5 / static_cast<double>(meModel->get_tdt());

        if (meConf->term_quiet <= 1)
            std::cout << "spi wait " << spi_wait << std::endl;
        
        for (int i = x; i < area_size; i++)
        for (int j = y; j < y+3; j++)
            meModel->getMass(i, j)->u = 0.8;
    }

    mech_wait = meConf->wait_mech;
    sac_wait = mech_wait + 1000;
    meModel->toogle_mechanic(0);
    spi_do = 1;
}


void Worker::hold_spirale()
{
    if (!spi_do)
        return;

     if (meConf->area_mech == 1 && mech_wait >= 0)
        if (mech_wait-- == 0)
            meModel->toogle_mechanic(1);
    
    if (meConf->area_sac == 1 && sac_wait >= 0)
        if (sac_wait-- == 0)
            meModel->toogle_feedback(1);

   if (!spi_wait)
       return;

    spi_wait--;
    
    //ccBereich im Uhrzeigersinn
    if(meConf->AP_model==1996){
        for(int i=0; i<area_size/2; i++){
            for(int j=area_size/2-2*area_size/10+59; j<area_size/2+59; j++){
                meModel->getMass(i, j)->v = 2;
            }
        }
    }
    
        //ccBereich gegen Uhrzeigersinn
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2; i++){
//            for(int j=area_size/2+10; j>area_size/2-10; j--){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//    }
    
    
    
    //ll Bereich im Uhrzeigersinn
//      if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2+2*area_size/10; i++){
//            for(int j=area_size/2+area_size/10; j<area_size/2+3*area_size/10; j++){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//    }
    
    //ll Bereich gegen Uhrzeigersinn
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2+2*area_size/10; i++){
//            for(int j=area_size/2-4*area_size/10; j<area_size/2-2*area_size/10; j++){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//    }
  
    //kl Bereich gegen Uhrzeigersinn
//    if(meConf->AP_model==1996){
//        for(int i=0; i<area_size/2-20; i++){
//            for(int j=area_size/2+20; j>area_size/2-15; j--){
//                meModel->getMass(i, j)->v = 2;
//            }
//        }
//    }
  
    //kl Bereich
//    if(meConf->AP_model==1996){
//        if(spi_wait>1200){
//            for(int i=0; i<area_size/2+2*area_size/10; i++){
//                for(int j=area_size/2+area_size/10; j>area_size/2-area_size/10; j--){
//                    meModel->getMass(i, j)->v = 2;
//                }
//            }
//        }
//        else{
//            for(int i=4*area_size/10; i<6*area_size/10; i++){
//                for(int j=4*area_size/10; j<6*area_size/10; j++){
//                    meModel->getMass(i, j)->v = 2;
//                }
//            }
//        }
//    }
    
    else{
        for (int i = spi_px-1; i < area_size; i++)
        for (int j = spi_py-1; j >  spi_py-5; j--)
            meModel->getMass(i, j)->u = 0.0;
    }
}


void Worker::make_chaos()
{
    cho_wait = 4.5 / meModel->get_tdt();

    if (meConf->term_quiet <= 1)
        std::cout << "cho wait " << cho_wait << std::endl;

    mech_wait = meConf->wait_mech;
    meModel->toogle_mechanic(0);

    cho_list.push_back(int4(static_cast<int>(area_size * 0.3), static_cast<int>(area_size * 0.8),  0, 20));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.7), static_cast<int>(area_size * 0.6), 30,  0));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.6), static_cast<int>(area_size * 0.3), 20,  0));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.2), static_cast<int>(area_size * 0.7),  0, 10));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.3), static_cast<int>(area_size * 0.4),  0, 10));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.2), static_cast<int>(area_size * 0.2), 10,  0));
    cho_list.push_back(int4(static_cast<int>(area_size * 0.5), static_cast<int>(area_size * 0.9),  0,  5));

    cho_do = 1;

//    std::srand(std::time(nullptr));

//    ran_list.resize(area_size * area_size, 0.0);
//    for (int i = 0; i < area_size; i++)
//    for (int j = 0; j < area_size; j++)
//        ran_list[area_size * i + j] = 2.0 / (double) (std::rand() % 30 + 1);

    for (auto elem : cho_list)
        for (int i = elem.a-3; i < elem.a+3; i++)
        for (int j = elem.b-3; j < elem.b+3; j++)
            meModel->getMass(i, j)->u = 0.8;
}


void Worker::hold_chaos()
{
    if (!cho_do)
        return;

    if (mech_wait >= 0)
        if (mech_wait-- == 0)
            meModel->toogle_mechanic(1);

    if (!cho_wait)
        return;

    cho_wait--;

//    for (int i = 0; i < area_size; i++)
//    for (int j = 0; j < area_size; j++)
//        meModel->getMass(i, j)->v =
//            (meModel->getMass(i, j)->v < ran_list[area_size * i + j]) ? ran_list[area_size * i + j] : meModel->getMass(i, j)->v;

    for (auto elem : cho_list)
        for (int i = elem.a-elem.c; i < elem.a+elem.d; i++)
        for (int j = elem.b-elem.c; j < elem.b+elem.d; j++)
            meModel->getMass(i, j)->v = 4.0;
//            meModel->getMass(i, j)->u = 0.0;
}

void Worker::make_random()
{
    
    //einzelne Stimuli versetzt initialiesiert
    
    srand(seed);
    ran_do=1; 
    
    mech_wait = meConf->wait_mech;
    sac_wait  = mech_wait+1000;
    meModel->toogle_mechanic(0);
    meModel->toogle_feedback(0);
    
}

void Worker::hold_random(int rtime)
{
    if(ran_do==0)
        return;
    
    if (meConf->area_mech == 1 && mech_wait >= 0)
        if (mech_wait-- == 0)
            meModel->toogle_mechanic(1);
    
    if (meConf->area_sac == 1 && sac_wait >= 0)
        if (sac_wait-- == 0)
            meModel->toogle_feedback(1);
    
    if(rtime==meConf->wait_mech+11000)
        ran_do=0;
    
    if(rtime>=10000)
        return;
        
    
            
    double p=(double)rand()/RAND_MAX*100;
    if(p<=0.75){
        int x_u=rand()%(area_size-8)+4;
        int y_u=rand()%(area_size-8)+4;
        int x_v=rand()%(area_size-8)+4;
        int y_v=rand()%(area_size-8)+4;
        
        //v Blöcke in der Nähe von Punktstimuli setzen:
//        if(x_u>=area_size-12)
//            x_v=x_u-5;
//        if(x_u<=11)
//            x_v=x_u+5;
//        if(x_u>11 && x_u<area_size-12)
//            x_v=x_u+5*( (rand()%2==0) ? (-1.0) : 1.0 );
//            
//        if(y_u>=area_size-12)
//            y_v=y_u-5;
//        if(y_u<=11)
//            y_v=y_u+5;
//        if(y_u>11 && y_u<area_size-12)
//            y_v=y_u+5*( (rand()%2==0) ? (-1.0) : 1.0 );
        
        
        make_dot(x_u,y_u);
        make_vdot(x_v,y_v);
        act_count++;
    }
    if(act_count==1 && act_scan==1){
        act_prep();
        act_count++;
    }
}

void Worker::make_stateload()
{
    meModel->state_load("stateshot_now.bin");
    
    if (meConf->area_sac == 1){
        state_do = 1;
        sac_wait = 1000;
        meModel->toogle_feedback(0);
    }
}

void Worker::hold_stateload()
{
    if(state_do==0)
        return;
    
    if (meConf->area_sac == 1 && sac_wait >= 0)
        if (sac_wait-- == 0){
            meModel->toogle_feedback(1);
            state_do=0;
        }
}

void Worker::interval(int nowtime)
{
    if(sac_wait_int >= 0){
        if(sac_wait_int-- == 0){
            meModel->toogle_feedback(mech);
            mech=0;
        }
    }
    if(nowtime % mech_int_time==0 && nowtime > tea_beg){
        meModel->toogle_mechanic(mech);
        meModel->clear_field();
            if(mech==1){
                sac_wait_int=1000;
            }
            else{
                meModel->toogle_feedback(mech);
                mech=1;
            }
        if (meConf->term_quiet <= 1)
            std::cout << "# switched mechanic mode " << std::endl;
    }
}


// ==========================================================================================
// Logging area


void Worker::log_prep()
{
    log_data.clear();
    log_run = 1;

    if (meConf->term_quiet <= 1)
        std::cout << "# begin log, planned until " << log_end << std::endl;
}


void Worker::log_tap(int nowtime)
{
    if (nowtime < log_beg)
        return;

    if (nowtime >= log_end)
        log_stop();

    if (nowtime % log_step != 0)
        return;

    log_data.push_back(meModel->getRuntime());
    log_data.push_back(meModel->getMass(log_ptx, log_pty)->u);
    log_data.push_back(meModel->getMass(log_ptx, log_pty)->v);
    log_data.push_back(meModel->getMass(log_ptx, log_pty)->area - 1.0);
}


void Worker::log_stop()
{
    if (!log_run)
        return;
    log_run = 0;

    time_t now  = std::time(0);
    std::tm *ltm = localtime(&now);

    std::stringstream strs;
    strs << "log_f" << meConf->area_size << "_d" << meConf->val_d << "_ah" << meConf->area_home << "_" <<
        1900+ltm->tm_year <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_mon <<
        std::setfill('0') << std::setw(2) << ltm->tm_mday << "-" <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_hour <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_min <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_sec << ".txt";

    std::ofstream outfile;
    outfile.open(strs.str().c_str());
    for (unsigned int i = 0; i < log_data.size(); i++) {
        outfile << log_data[i++] << "\t";
        outfile << log_data[i++] << "\t";
        outfile << log_data[i++] << "\t";
        outfile << log_data[i] << "\n";

    }
    outfile.close();

    log_data.clear();

    if (meConf->term_quiet <= 1)
        std::cout << "# end scan log, file " << strs.str() << " written" << std::endl;
}



// ==========================================================================================
// Activity detection area, it shall detect if there is electrical activity (only for mode random)


void Worker::act_prep()
{
    act_run = 1;

    if (meConf->term_quiet <= 1)
        std::cout << "# begin activity detection" << std::endl;
    
    if(comp_time==1){
        std::stringstream strs;
        strs << "random_a" << meConf->val_a << "_k" << meConf->val_k << "_seed" << seed << ".txt";
        auto T = time(NULL);
        std::ofstream outfile;
        outfile.open(strs.str().c_str(), std::ios::app);
        outfile << "Beginning" << "\t" << ctime(&T) << "\n";
        outfile.close();        
    }
}

void Worker::act_tap(int nowtime)
{
    if (nowtime < 10000)
        return;
        
    if (nowtime % 100 != 0)     
        return;
    
    if (nowtime >= act_end)
        act_stop();
    for(int i=area_pad;i<area_size-area_pad;i++){
        for(int j=area_pad;j<area_size-area_pad;j++){
            double u_min=0.15;
            double u=meModel->getMass(i,j)->u;
            
            if (u>u_min)
                return;
        }
    }
    act_stop();
}

void Worker::act_stop()
{
    if (!act_run)
        return;
    act_run = 0;
    ran_do  = 0;
    
    std::stringstream strs;
    strs << "random_a" << meConf->val_a << "_k" << meConf->val_k << "_Gs" << meConf->val_Gs << "_seed" << seed << ".txt";
    std::ofstream outfile;
    outfile.open(strs.str().c_str(), std::ios::app);
    
    if(comp_time==1){
        auto T = time(NULL);
        outfile << meConf->val_a << "\t" << meConf->val_k << "\t" << meModel->getRuntime() << "\t" << tea_count << "\t" << tea_max << "\n" << "Ending" << "\t" << ctime(&T);
    }
    else{
        outfile << meConf->val_a << "\t" << meConf->val_k << "\t" << meModel->getRuntime() << "\t" << tea_count << "\t" << tea_max;
    }
    outfile.close();
    
    if (meConf->term_quiet <= 1)
        std::cout << "# end scan, file written to " << strs.str() << std::endl;
}

// ==========================================================================================
// SAC detect area, it shall detect and measure the following waves after first excitation


void Worker::sac_prep()
{
    sac_data.clear();
    sac_run = 1;

    sac_trig    = 0;
    sac_prev    = 0.0;
    sac_min     = 0.0;

    if (meConf->term_quiet <= 1)
        std::cout << "# begin sac, planned until " << sac_end << std::endl;
}


void Worker::sac_tap(int nowtime)
{
    if (nowtime >= sac_end)
        sac_stop();

    double tmp = meModel->getMass(log_ptx, log_pty)->u;

    if (sac_trig && tmp > sac_prev && tmp > sac_min + sac_thresh) {
        sac_trig = 0;
    }

    if (sac_trig)
        sac_min = (tmp < sac_min) ? tmp : sac_min;

    if (!sac_trig && tmp < sac_prev) {
        sac_data.push_back(meModel->getRuntime());
        sac_data.push_back(sac_prev);

        sac_trig = 1;
    }

    sac_prev = tmp;

    if ((int)sac_data.size()/2 >= sac_maxnum)
        sac_stop();
}


void Worker::sac_stop()
{
    if (!sac_run)
        return;
    sac_run = 0;

    std::stringstream strs;
    strs << "sac_kT" << meConf->val_kT << "_d" << meConf->val_d << ".txt";

    std::ofstream outfile;
    outfile.open(strs.str().c_str());
    outfile << meConf->val_kT << "\t" << meConf->val_d;

    for (unsigned long i = 0; i < sac_data.size(); i++) {
        outfile << "\t" << sac_data[i++];
        outfile << "\t" << sac_data[i];
    }

    for (unsigned long i = sac_data.size(); i < 10; i++) {
        outfile << "\t" << 0;
    }

    outfile.close();

    if (meConf->term_quiet <= 1)
        std::cout << "# end scan sac, file written to " << strs.str() << std::endl;
}





// ==========================================================================================
// Tear up detection, it shall detect any beginnings of tear ups of preferrently spirals
// for this, the centered area has to be ignored


void Worker::tea_prep()
{
    tea_data.clear();
    tea_phas.clear();
    tea_phas2.clear();
    tea_run = 1;

    tea_tau     = 8;     // tsp, approximately 1/4 rotation of spiral
    tea_pivot   = 0;

    tea_data.resize(area_size * area_size * tea_tau, 0);
    tea_phas.resize(area_size * area_size, 0);
    tea_phas2.resize(area_size * area_size, 0);

    if (meConf->term_quiet <= 1)
        std::cout << "# begin break up detect, planned until " << tea_end << std::endl;
    
    if (tea_time == 1 && comp_time==1){
        std::stringstream strs;
        strs << "tea_time_kT" << meConf->val_kT << "_Gs" << meConf->val_Gs << "_" << seed << ".txt";
        std::ofstream outfile;
        outfile.open(strs.str().c_str(), std::ios::app);
        auto T = time(NULL);
        outfile << "Beginning" << "\t" << ctime(&T) << "\n";
        outfile.close();
    }
}


double Worker::tea_phadiff(double a, double b)
{
    if (std::abs(a-b) <= M_PI)
        return a-b;
    else if (a-b > 0)
        return a - b - 2 * M_PI;
    else
        return a - b + 2 * M_PI;
}


void Worker::tea_tap(int nowtime)
{
    if (nowtime < tea_beg)
        return;

    if (nowtime >= tea_end)
        tea_stop();

    //if (nowtime % 20 != 0)     // every 20th time step to reduce memory usage.  Für AP 1996 muss jeder zeitschritt berücksichtigt werden
    //    return;

    tea_pivot = (tea_pivot+1) % tea_tau;

    double tmpu = 0.0, phas_now;

    for (int dx = 9; dx < area_size-9; dx++)
    for (int dy = 9; dy < area_size-9; dy++) {
        tmpu = meModel->getMass(dx, dy)->u;
        phas_now = std::atan2(
            tmpu - 0.5,
            tea_data[area_size * (area_size * tea_pivot + dy) + dx] - 0.5
        );
        tea_data[area_size * (area_size * tea_pivot + dy) + dx] = tmpu;
        tea_phas[area_size * dy + dx] = phas_now;
    }

    double k_sum = 0.0;

    for (int dx = 10; dx < area_size-10; dx++)
    for (int dy = 10; dy < area_size-10; dy++) {
        k_sum  = tea_phadiff(tea_phas[area_size * (dy-1) + dx],   tea_phas[area_size * (dy-1) + dx+1]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy+1) + dx],   tea_phas[area_size * (dy+1) + dx-1]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy)   + dx+1], tea_phas[area_size * (dy+1) + dx+1]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy)   + dx-1], tea_phas[area_size * (dy-1) + dx-1]);

        k_sum += tea_phadiff(tea_phas[area_size * (dy-1) + dx-1], tea_phas[area_size * (dy-1) + dx]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy-1) + dx+1], tea_phas[area_size * (dy)   + dx+1]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy+1) + dx+1], tea_phas[area_size * (dy+1) + dx]);
        k_sum += tea_phadiff(tea_phas[area_size * (dy+1) + dx-1], tea_phas[area_size * (dy)   + dx-1]);

        tea_phas2[area_size * dy + dx] = (k_sum >= 2 * M_PI || k_sum <= -2 * M_PI);
    }

    tea_count = 0;
    for (int dx = 10; dx < area_size-10; dx++)
    for (int dy = 10; dy < area_size-10; dy++) {

        if (tea_phas2[area_size * dy + dx] == 1) {
            
            for (int ndx = dx-2; ndx < dx+2; ndx++)
            for (int ndy = dy-2; ndy < dy+2; ndy++)
                tea_phas2[area_size * ndy + ndx] = 0;

            for (int ndx = dx-2; ndx < dx+2; ndx++)
            for (int ndy = dy-2; ndy < dy+2; ndy++)
                tea_phas2[area_size * ndy + ndx] = 2;
            tea_count++;
            tea_count++;
            
            //Speichern der Singularitätenpositionen
//            std::stringstream strs;
//            strs << "tea_positions" << "_Gs" << meConf->val_Gs << ".txt";
//
//            std::ofstream outfile;
//            outfile.open(strs.str().c_str(), std::ios::app);
//            outfile << nowtime << "\t" << dx << "," << dy << "\n";
//            outfile.close();
        }
    }

    if(tea_count > tea_max)
        tea_max = tea_count;
    
    tea_mean+=tea_count/(static_cast<double>(tea_end-tea_beg));
    
    if(tea_time == 1 && nowtime % 100 == 0){
        std::stringstream strs;
        strs << "tea_time_kT" << meConf->val_kT << "_Gs" << meConf->val_Gs << "_" << seed << ".txt";

        std::ofstream outfile;
        outfile.open(strs.str().c_str(), std::ios::app);
        outfile << nowtime << "\t" << tea_count << "\n";
        outfile.close();
    }
        
    if (tea_count >= tea_cmin)
        tea_stop();
}


void Worker::tea_stop()
{
    if (!tea_run)
        return;
    tea_run = 0;

    std::stringstream strs;
    strs << "breakup_kT" << meConf->val_kT << "_Gs" << meConf->val_Gs << ".txt";

    std::ofstream outfile;
    outfile.open(strs.str().c_str());
    outfile << meConf->val_kT << "\t" << meConf->val_Gs << "\t" << meModel->getRuntime() << "\t" << tea_count << "\t" << tea_mean;

    outfile.close();

    if (meConf->term_quiet <= 1)
        std::cout << "# end scan, file written to " << strs.str() << std::endl;
    
    if (tea_time == 1 && comp_time==1){
        std::stringstream strs;
        strs << "tea_time_kT" << meConf->val_kT << "_Gs" << meConf->val_Gs << "_" << seed << ".txt";
        auto T = time(NULL);
        std::ofstream outfile;
        outfile.open(strs.str().c_str(), std::ios::app);
        outfile << "Ending" << "\t" << ctime(&T) << "\n";
        outfile.close();
    }
}


// ==========================================================================================
//Data saving area

void Worker::save_data(int rtime) // make all 25tsp a snapshot
{
    if(rtime % 25 != 0)
        return;
    
    meModel->state_save(rtime);    
}
