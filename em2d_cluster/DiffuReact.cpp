/*
 * DiffuReact.cpp
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

#include <filesystem>

#include "npy.hpp"

#include "DiffuReact.hpp"
#include "threading.hpp"
#include "sys/stat.h"


/*
 * To avoid putting (and including more headers) variables in the  DiffuReact
 * class, the threading uses some global variables in a anonymous namespace.
 *
 * This assumes that there is only one DiffuReact instance. If could be more
 * than one instance, put all this inside the DiffuReact and remove `static`.
 */
namespace {
namespace threadpool {
    // the number of worker threads we are using
    static unsigned thread_count;

    static std::vector<std::thread> threads;

    // thread synchronization barrier
    static ThreadBarrier* barrier;

    // semaphore, call with .signal(num_threads) for one time step using all threads
    static Semaphore run_semaphore;

    // callback when to the main thread once the worker threads have completed one time step
    static AutoResetEvent run_completed;

    // signal threads to terminate, run_semaphore->signal() must be called after setting it to true
    static std::atomic<bool> kill_threads;
}
}

DiffuReact::DiffuReact(Config* put_conf)
{
    meConf = put_conf;

    fprev = fnext = 0;
    run_time = 0;

    // Electrophysiology
    //val_a = meConf->val_a;
    //val_k = meConf->val_k;
    val_diff = meConf->val_diff;
    val_e0 = meConf->val_e0;
    val_mu1 = meConf->val_mu1;
    val_mu2 = meConf->val_mu2;

    // Mechanics p1
    val_kT = meConf->val_kT;

    // Electromechanical feedback
    val_Gs = meConf->val_Gs;
    val_Es = meConf->val_Es;

    val_dt = meConf->val_dt;

    area_size   = meConf->area_size;

    field_sum   = area_size * area_size;

    area_mech   = meConf->area_mech;
    area_sac    = meConf->area_sac;

    area_aniso  = meConf->area_aniso;
    elec_aniso  = meConf->elec_aniso;

    area_home   = meConf->area_home;

    if(meConf->AP_model==1996){
        iterate_mech=50;
    }
    else{
        if(meConf->AP_model==2005){
            iterate_mech=1;
        }
        else{
            std::cout << "# wrong AP model" << std::endl;
        }
    }

    // =====================


    ang_ceta    = std::tan( mat_radians(meConf->area_angle) );
//    ang_ceta    = meConf->area_angle / 45.0;

    // calculate laplace and rotate it correctly
    if (elec_aniso) {

        elec_lap[0] = meConf->elec_lap[0] * (1 - ang_ceta) + (meConf->elec_lap[7] * ang_ceta) * 2;
        elec_lap[1] = meConf->elec_lap[1] * (1 - ang_ceta) + (meConf->elec_lap[0] * ang_ceta) / 2;
        elec_lap[2] = meConf->elec_lap[2] * (1 - ang_ceta) + (meConf->elec_lap[1] * ang_ceta) * 2;
        elec_lap[3] = meConf->elec_lap[3] * (1 - ang_ceta) + (meConf->elec_lap[2] * ang_ceta) / 2;
        elec_lap[4] = meConf->elec_lap[4] * (1 - ang_ceta) + (meConf->elec_lap[3] * ang_ceta) * 2;
        elec_lap[5] = meConf->elec_lap[5] * (1 - ang_ceta) + (meConf->elec_lap[4] * ang_ceta) / 2;
        elec_lap[6] = meConf->elec_lap[6] * (1 - ang_ceta) + (meConf->elec_lap[5] * ang_ceta) * 2;
        elec_lap[7] = meConf->elec_lap[7] * (1 - ang_ceta) + (meConf->elec_lap[6] * ang_ceta) / 2;

    } else {
        for (int i = 0; i < 8; i++)
            elec_lap[i] = meConf->elec_lap[i];
    }

    elec_sub = 0;
    for (int i = 0; i < 8; i++)
        elec_sub += elec_lap[i];

    std::cout << "Laplace\t\t";
    for (int i = 0; i < 8; i++)
         std::cout << elec_lap[i] << " ";
    std::cout << "\n";

    // =====================


    // Mechanics p2
    val_c = meConf->val_c;
    val_d = meConf->val_d * meConf->val_dx;

    val_kap = meConf->val_kap;

    val_mt = meConf->val_mt;
    val_r0 = 1.0;
    zw_r0d = val_r0 * std::sqrt(2.0);

    // This pad is (maybe) to prevent strange feedback at the edge
    area_pad = meConf->area_pad;

    clearField();

    threadpool::thread_count = get_thread_count();
    threadpool::barrier = new ThreadBarrier(threadpool::thread_count);
    threadpool::kill_threads = false;

    using Point = std::pair<int, int>;
    std::vector<Point> indexes;
    indexes.reserve(area_size * area_size);
    for (int lx = 0; lx < area_size; lx++) {
      for (int ly = 0; ly < area_size; ly++) {
        indexes.emplace_back(lx, ly);
      }
    }

    auto chunks = make_chunks(indexes, threadpool::thread_count);

    for (unsigned i = 0; i < threadpool::thread_count; i++) {
      auto chunk = chunks[i];
      std::vector<Point> points(chunk.first, chunk.second);
      threadpool::threads.emplace_back(&DiffuReact::step_thread, this, points, i);
    }
}

void DiffuReact::rewrite_elec_Lap(float angle){

    elec_lap[0] = meConf->elec_lap[0] * (1 - angle/45.0) + (meConf->elec_lap[7] * angle/45.0) * 2;
    elec_lap[1] = meConf->elec_lap[1] * (1 - angle/45.0) + (meConf->elec_lap[0] * angle/45.0) / 2;
    elec_lap[2] = meConf->elec_lap[2] * (1 - angle/45.0) + (meConf->elec_lap[1] * angle/45.0) * 2;
    elec_lap[3] = meConf->elec_lap[3] * (1 - angle/45.0) + (meConf->elec_lap[2] * angle/45.0) / 2;
    elec_lap[4] = meConf->elec_lap[4] * (1 - angle/45.0) + (meConf->elec_lap[3] * angle/45.0) * 2;
    elec_lap[5] = meConf->elec_lap[5] * (1 - angle/45.0) + (meConf->elec_lap[4] * angle/45.0) / 2;
    elec_lap[6] = meConf->elec_lap[6] * (1 - angle/45.0) + (meConf->elec_lap[5] * angle/45.0) * 2;
    elec_lap[7] = meConf->elec_lap[7] * (1 - angle/45.0) + (meConf->elec_lap[6] * angle/45.0) / 2;

    elec_sub = 0;
    for (int i = 0; i < 8; i++)
        elec_sub += elec_lap[i];
}

DiffuReact::~DiffuReact()
{
    field.clear();      // clear cell field

    mLap[0].clear();    // clear the laplace fields
    mLap[1].clear();    //       both of them

    threadpool::kill_threads = true;
    threadpool::run_semaphore.signal(threadpool::thread_count);
    for (auto &t : threadpool::threads) {
        t.join();
    }
    delete threadpool::barrier;
}


Cell* DiffuReact::getMass(int x, int y)
{
    return &field[area_size * y + x];
}


double DiffuReact::getLap(int x, int y)
{
    return mLap[fprev][area_size * y + x];
}


void DiffuReact::step_thread(std::vector<std::pair<int, int>> points, int thread_id)
{
    while (true) {
      threadpool::run_semaphore.wait();  // wait until main thread calls run_semaphore.signal()

      if (threadpool::kill_threads) return;

      for (const auto& p : points) {
          updt_Variables(p.first, p.second);
      }

      threadpool::barrier->arrive_and_wait(); // synchronize threads

      // laplacian computation remains single-threaded for now
      if (thread_id == 0) {
          updt_Lap();
      }

      threadpool::barrier->arrive_and_wait(); // synchronize threads

      if (area_mech){

           for(int k=0; k<iterate_mech; k++) {

               for (const auto& p : points) {
                   updt_Spring(p.first, p.second);
               }

               threadpool::barrier->arrive_and_wait(); // synchronize threads

               for (const auto& p : points) {
                   updt_Position(p.first, p.second);
               }

               threadpool::barrier->arrive_and_wait(); // synchronize threads

               for (const auto& p : points) {
                   updt_Area(p.first, p.second);
               }

               threadpool::barrier->arrive_and_wait();  // synchronize threads

           }


      }


      if (thread_id == 0) { // prevent thread race
          threadpool::run_completed.signal();  // notify main thread that timestep was completed
      }
    }
}

void DiffuReact::make_step()
{
    fprev = fnext;
    fnext = 1-fnext;

    threadpool::run_semaphore.signal(threadpool::thread_count);
    threadpool::run_completed.wait();

    clearEdge();

    run_time++;
}

void DiffuReact::updt_Variables(int lx, int ly)
{
    if (lx < 1 || lx >= area_size-1)
        return;
    if (ly < 1 || ly >= area_size-1)
        return;

    Cell *ctmp;
    double tmpIs, nU, nV, nTa, tU, tV, tTa;

    ctmp = getMass(lx, ly);

    nU = ctmp->u;
    nV = ctmp->v;
    nTa = ctmp->ta;

    tmpIs = val_Gs * (std::sqrt((ctmp->area >= 1.0) ? ctmp->area : 1.0) - 1.0) * (nU - val_Es) * area_sac;
    ctmp->is = -tmpIs;

    tU = (ctmp->val_k * nU * (1.0 - nU) * (nU - ctmp->val_a) - nU * nV - tmpIs) * val_dt + getLap(lx, ly) * val_dt * val_diff;


    //tV = (val_k * nU - nV) * ( (nU < 0.05) ? 1.0 : 0.1 ) * val_dt;                                                //2005
    tV = (val_e0 + (val_mu1 * nV)/(nU + val_mu2)) * (ctmp->val_k * nU * (ctmp->val_a + 1.0 - nU) - nV) * val_dt;                //1996


    tTa = (val_kT * nU - nTa) * ( (nU < 0.05) ? 1.0 : 0.1 ) * val_dt; //True:False                                //2005

    //tTa = (val_e0 + (val_mu1 * nTa)/(nU + val_mu2)) * (val_k * nU * (cmtp->val_a + 1.0 - nU) - nTa) * val_dt;           //1996

    nU += tU;
    nV += tV;
    nTa += tTa;

    nU = (nU > 1.0) ? 1.0 : nU;
    nTa = (nTa > 1.0) ? 1.0 : nTa;

    ctmp->u = nU;
    ctmp->v = nV;
    ctmp->ta = nTa;
}


void DiffuReact::updt_Lap()
{
    for (int lx = 1; lx < area_size-1; lx++)
    for (int ly = 1; ly < area_size-1; ly++) {
        currU = field[area_size * ly + lx].u;

        //Test zur gekurvten Anisotropie
//        if(ly == 1){
//            rewrite_elec_Lap(0);
//        }
//        if(ly == area_size/2){
//            rewrite_elec_Lap(5);
//        }
//        if(ly == area_size/2+area_size/20){
//            rewrite_elec_Lap(10);
//        }

//        if(ly==1)
//            rewrite_elec_Lap(0);
//        if(ly>area_size/2){
//            rewrite_elec_Lap((ly-area_size*0.5)/(area_size*0.5)*45.0);
//        }

        mLap[fnext][area_size * (ly+1) + lx+1]  += currU * elec_lap[1];
        mLap[fnext][area_size * (ly-1) + lx+1]  += currU * elec_lap[3];
        mLap[fnext][area_size * (ly-1) + lx-1]  += currU * elec_lap[5];
        mLap[fnext][area_size * (ly+1) + lx-1]  += currU * elec_lap[7];
        mLap[fnext][area_size * (ly+1) + lx  ]  += currU * elec_lap[0];
        mLap[fnext][area_size * ly     + lx+1]  += currU * elec_lap[2];
        mLap[fnext][area_size * (ly-1) + lx  ]  += currU * elec_lap[4];
        mLap[fnext][area_size * ly     + lx-1]  += currU * elec_lap[6];

        mLap[fnext][area_size * ly     + lx  ]  -= currU * elec_sub;

        mLap[fprev][area_size * ly + lx] = 0.0;
    }
}




/**
 * @brief This instance function assumes a range of lx and ly in [0, area_size) for updating the springs
 * @param lx
 * @param ly
 */
void DiffuReact::updt_Spring(int lx, int ly)
{
    vect2 tpos, tvel;
    double tmabs, para, parb, parc;

    // ========== horizontals ==========

    if (lx < area_size-1) {
        tpos.set( getMass(lx+1, ly)->curr_pos - getMass(lx, ly)->curr_pos );
        tvel.set( getMass(lx+1, ly)->curr_vel - getMass(lx, ly)->curr_vel );
        tmabs = tpos.norm();

        para = val_c * (tmabs - val_r0) / val_r0;
        parb = val_d * (tvel.a * tpos.a + tvel.b * tpos.b) / tmabs;

        parc = (!area_aniso) ? (getMass(lx+1, ly)->ta + getMass(lx, ly)->ta) * 0.5 * (1 - ang_ceta) : 0;

        getMass(lx, ly)->force[2].set( (tpos / tmabs) * (para - parb + parc) );
        getMass(lx+1, ly)->force[6].set( -getMass(lx, ly)->force[2] );
    }

    // ========== verticals ==========
    if (ly < area_size-1) {
        tpos.set( getMass(lx, ly+1)->curr_pos - getMass(lx, ly)->curr_pos );
        tvel.set( getMass(lx, ly+1)->curr_vel - getMass(lx, ly)->curr_vel );
        tmabs = tpos.norm();

        para = val_c * (tmabs - val_r0) / val_r0;
        parb = val_d * (tvel.a * tpos.a + tvel.b * tpos.b) / tmabs;

        parc = (getMass(lx, ly)->ta + getMass(lx, ly+1)->ta) * 0.5 * (1 - ang_ceta);

        getMass(lx, ly)->force[4].set( (tpos / tmabs) * (para - parb + parc) );
        getMass(lx, ly+1)->force[0].set( -getMass(lx, ly)->force[4] );
    }

    // ========== diagonal left and right
    if (lx < area_size-1 && ly < area_size-1) {
        // ====== left ==
        tpos.set( getMass(lx+1, ly+1)->curr_pos - getMass(lx, ly)->curr_pos );
        tvel.set( getMass(lx+1, ly+1)->curr_vel - getMass(lx, ly)->curr_vel );
        tmabs = tpos.norm();

        para = val_c * (tmabs - zw_r0d) / zw_r0d;
        parb = val_d * (tvel.a * tpos.a + tvel.b * tpos.b) / tmabs;

        parc = (getMass(lx+1, ly+1)->ta + getMass(lx, ly)->ta) * 0.5 * ang_ceta;

        getMass(lx, ly)->force[3].set( (tpos / tmabs) * (para - parb + parc) );
        getMass(lx+1, ly+1)->force[7].set( -getMass(lx, ly)->force[3] );

        // ====== right ==
        tpos.set( getMass(lx+1, ly)->curr_pos - getMass(lx, ly+1)->curr_pos );
        tvel.set( getMass(lx+1, ly)->curr_vel - getMass(lx, ly+1)->curr_vel );
        tmabs = tpos.norm();

        para = val_c * (tmabs - zw_r0d) / zw_r0d;
        parb = val_d * (tvel.a * tpos.a + tvel.b * tpos.b) / tmabs;

        parc = (!area_aniso) ? (getMass(lx+1, ly)->ta + getMass(lx, ly+1)->ta) * 0.5 * ang_ceta : 0;

        getMass(lx, ly+1)->force[1].set( (tpos / tmabs) * (para - parb + parc) );
        getMass(lx+1, ly)->force[5].set( -getMass(lx, ly+1)->force[1] );
    }
}


/**
 * @brief This instance function assumes a range of lx and ly in [0, area_size) for updating the areas
 * @param lx
 * @param ly
 */
void DiffuReact::updt_Area(int lx, int ly)
{
    if (lx >= area_pad && lx < area_size-area_pad && ly >= area_pad && ly < area_size-area_pad)
        getMass(lx, ly)->area =
            (getMass(lx+1, ly)->curr_pos - getMass(lx-1, ly)->curr_pos).norm() *
            (getMass(lx, ly+1)->curr_pos - getMass(lx, ly-1)->curr_pos).norm() * 0.25;
}


/**
 * @brief This instance function assumes a range of lx and ly in [0, area_size) for updating the position of each cell
 * @param lx
 * @param ly
 */
void DiffuReact::updt_Position(int lx, int ly)
{
    vect2 acc, new_pos;
    Cell *ctmp;

//     TODO hier wurde die Masse vernachlässig. Wenn es also benötigt wird, hier bearbeiten

    ctmp = getMass(lx, ly);

    acc.zero();
    for (int i = 0; i < 8; i++)
        acc.add(ctmp->force[i]);

    // calculate with verlet integration scheme
    new_pos.set(ctmp->curr_pos * 2 - ctmp->prev_pos + acc * val_mt * val_mt);

    if (lx == 0 || lx == area_size-1 || ly == 0 || ly == area_size-1)
        new_pos.add( (ctmp->orig_pos - ctmp->curr_pos) * area_home );

    // calculate velocity
    ctmp->curr_vel.set( -(ctmp->curr_pos - ctmp->prev_pos) / val_mt );

    // discard previous, move current to previous and set new to current
    ctmp->prev_pos.set( ctmp->curr_pos );
    ctmp->curr_pos.set( new_pos );
}


void DiffuReact::clearEdge()
{
    // This is for cleaning up the edges, nothing more
    for (int i = 0; i < area_size; i++) {
        mLap[fnext][area_size * 0 + i] = 0.0;
        mLap[fnext][area_size * i + 0] = 0.0;
        mLap[fnext][area_size * (area_size-1) + i] = 0.0;
        mLap[fnext][area_size * i + (area_size-1)] = 0.0;
    }
}


void DiffuReact::clearField()
{
    // clear and setup cell field and force field
    field.clear();          // clear cell field
    Cell *ctmp;

    field.resize(field_sum);
    for (int dx = 0; dx < area_size; dx++)
    for (int dy = 0; dy < area_size; dy++) {
        ctmp = &field[area_size * dy + dx];

        ctmp->orig_pos.set(vect2(dx, dy));
        ctmp->curr_pos.set(vect2(dx, dy));
        ctmp->prev_pos.set(vect2(dx, dy));
    }

    // TODO: Jan Lebert
    std::cout << "steffi is cool";
    // load local val_a, val_k
    std::string a_array_path = "a.npy";
    std::string k_array_path = "k.npy";

    std::vector<double> a_data;
    std::vector<unsigned long> a_shape;
    std::vector<double> k_data;
    std::vector<unsigned long> k_shape;
    bool fortran_order;
    npy::LoadArrayFromNumpy(a_array_path, a_shape, fortran_order, a_data);
    npy::LoadArrayFromNumpy(k_array_path, k_shape, fortran_order, k_data);
    assert(a_shape.size() == 2);
    assert(a_shape[0] == area_size);
    assert(a_shape[1] == area_size);
    assert(a_shape == k_shape);


    for (int dx = 0; dx < area_size; dx++) {
      for (int dy = 0; dy < area_size; dy++) {
        ctmp = &field[area_size * dy + dx];

        ctmp->val_a = a_data[area_size * dy + dx];
        ctmp->val_k = k_data[area_size * dy + dx];
      }
    }


    mLap[0].clear();        // clear the laplace fields
    mLap[1].clear();        //       both of them

    mLap[0].resize(field_sum, 0.0);
    mLap[1].resize(field_sum, 0.0);
}

//für Mechanikintervall leicht verändert
void DiffuReact::clear_field()
{
    // clear and setup cell field and force field
    //field.clear();          // clear cell field
    Cell *ctmp;

    field.resize(field_sum);
    for (int dx = 0; dx < area_size; dx++)
    for (int dy = 0; dy < area_size; dy++) {
        ctmp = &field[area_size * dy + dx];

        ctmp->orig_pos.set(vect2(dx, dy));
        ctmp->curr_pos.set(vect2(dx, dy));
        ctmp->prev_pos.set(vect2(dx, dy));
    }

    mLap[0].clear();        // clear the laplace fields
    mLap[1].clear();        //       both of them

    mLap[0].resize(field_sum, 0.0);
    mLap[1].resize(field_sum, 0.0);
}


void DiffuReact::toogle_mechanic(int mode)
{
    std::cout << "mode " << mode << std::endl;

    if (mode != 0 && mode != 1)
        area_mech = 1 - area_mech;
    else
        area_mech = mode;

    if (meConf->term_quiet <= 1)
        std::cout << "# mechanics on = " << area_mech << std::endl;
    return;std::cout << "# File written" << std::endl;
}


void DiffuReact::toogle_feedback(int mode)
{
    if (mode != 0 && mode != 1)
        area_sac = 1 - area_sac;
    else
        area_sac = mode;

    if (meConf->term_quiet <= 1)
        std::cout << "# feedback on = " << area_sac << std::endl;
    return;
}


int DiffuReact::getRuntime()
{
    return run_time;
}

double DiffuReact::get_tdt()
{
    return val_dt;
}



/**
 * @brief load the state, which was saved before
 * @param put_path
 */
void DiffuReact::state_load(const char* put_path)
{
    int info_in[2];
    std::vector<State_cell> state_field;
    state_field.resize(field_sum);

    std::ifstream in(put_path, std::ios::in | std::ios::binary);
    in.read((char *) &info_in, sizeof info_in);
    in.read((char *) &state_field[0], sizeof(State_cell) * state_field.size());
    in.close();

    clearField();

    Cell *ctmp;
    State_cell *stmp;

    for (int dx = 0; dx < area_size; dx++)
    for (int dy = 0; dy < area_size; dy++) {
        stmp = &state_field[area_size * dy + dx];
        ctmp = &field[area_size * dy + dx];

        ctmp->u = stmp->u;
        ctmp->v = stmp->v;
        ctmp->ta = stmp->ta;

        ctmp->curr_pos.set(stmp->curr_pos);
        ctmp->prev_pos.set(stmp->prev_pos);
        ctmp->curr_vel.set(stmp->curr_vel);
    }

    state_field.clear();

    for (int idx = 0; idx < area_size; idx++)
    for (int idy = 0; idy < area_size; idy++) {
        updt_Spring(idx, idy);
        updt_Position(idx, idy);
        updt_Area(idx, idy);
    }

    //std::cout << "# File loaded" << std::endl;
}


void DiffuReact::state_save(int rtime)
{
    // To save correctly, we need at least
    // u, v, ta, curr_pos, prev_pos, curr_vel
    // all those would be about 72 bytes big
    // If it works correctly, a "video" with about 300 images and an area size of 100 would results in approx. 106 MB
    // which is okay, I think

    int info_out[2] = { area_size, field_sum };
    std::vector<State_cell> state_field;
    state_field.resize(field_sum);

    Cell *ctmp;
    State_cell *stmp;

    for (int dx = 0; dx < area_size; dx++)
    for (int dy = 0; dy < area_size; dy++) {
        ctmp = &field[area_size * dy + dx];
        stmp = &state_field[area_size * dy + dx];

        stmp->set_dou(ctmp->u, ctmp->v, ctmp->ta);

        stmp->curr_pos.set(ctmp->curr_pos);
        stmp->prev_pos.set(ctmp->prev_pos);
        stmp->curr_vel.set(ctmp->curr_vel);
    }

//    std::cout << "size of State_cell " << sizeof(State_cell) << std::endl;
//    std::cout << "size of state_field " << state_field.size() << std::endl;

    std::filesystem::create_directory("Messdaten");

    std::stringstream stra;
    stra<<"Messdaten/px"<<meConf->stim_px<<"py"<<meConf->stim_py<<"_fib"<<meConf->area_angle<<"deg_k"<<meConf->val_k<<"_D"<<meConf->val_diff<<"_a"<<meConf->val_a<<"_e0"<<meConf->val_e0<<"_mu1"<<meConf->val_mu1<<"_mu2"<<meConf->val_mu2<<"_Gs"<<meConf->val_Gs<<"";
    std::filesystem::create_directory(stra.str());

    std::stringstream strs;
    strs<<"./"<<stra.str()<<"/data_t"<<rtime<<".bin";

    std::ofstream out(strs.str().c_str(), std::ios::out | std::ios::binary);
    if(!out) {
        std::cout << "Cannot open file." << std::endl;
        return;
    }
    out.write((const char*)&info_out[0], sizeof info_out);
    out.write((const char*)&state_field[0], sizeof(State_cell) * state_field.size());
    out.close();

    state_field.clear();

//    std::cout << "# File written" << std::endl;
}
