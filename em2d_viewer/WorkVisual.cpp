/*
 * WorkVisual.cpp
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


#include "WorkVisual.hpp"

WorkVisual::WorkVisual(Config *conf, Display *disp) : Worker(conf)
{
    meConf = conf;
    meDisp = disp;

    area_size = meConf->area_size;

    ang_cos = std::cos( mat_radians(meConf->area_angle) );
    ang_sin = std::sin( mat_radians(meConf->area_angle) );


    grid_span = 10;
    norm_size = 450.0;

    init_field();
    init_stretch();
    init_plot();
}




// ==========================================================================================
// Init area


void WorkVisual::init_field()
{
    vc_field[0].resize(area_size * area_size * 3);
    vc_field[1].resize(area_size * area_size * 3);
}


void WorkVisual::init_resti()
{
    vc_resti[0].resize(area_size * area_size * 3);
    vc_resti[1].resize(area_size * area_size * 3);
}


void WorkVisual::init_stretch()
{
    vc_stretch[0].resize(area_size * area_size * 3);
    vc_stretch[1].resize(area_size * area_size * 3);
}


void WorkVisual::init_plot()
{
    log_ptx = meConf->log_ptx;
    log_pty = meConf->log_pty;

    log_pivo = 0;
    log_last = meConf->plot_last;

    plot_dat[0].resize(log_last);
    plot_dat[1].resize(log_last);
    plot_dat[2].resize(log_last);
}


void WorkVisual::init_iactive()
{
    vc_iactive[0].resize(area_size * area_size * 3);
    vc_iactive[1].resize(area_size * area_size * 3);
}


void WorkVisual::init_phase()
{
    vc_phase[0].resize(area_size * area_size * 3);
    vc_phase[1].resize(area_size * area_size * 3);
}





// ==========================================================================================
// Draw area


void WorkVisual::draw_field(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_field[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_field[1][0]);
    glDrawArrays(GL_TRIANGLES, 0, vc_field[0].size());

    glColorPointer(3, GL_FLOAT, 0, &vc_grid[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_grid[1][0]);
    glDrawArrays(GL_LINES, 0, vc_grid[0].size());

    *x = norm_size;
    *y = norm_size;
}


void WorkVisual::draw_resti(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_resti[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_resti[1][0]);
    glDrawArrays(GL_TRIANGLES, 0, vc_resti[0].size());

    glColorPointer(3, GL_FLOAT, 0, &vc_grid[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_grid[1][0]);
    glDrawArrays(GL_LINES, 0, vc_grid[0].size());

    *x = norm_size;
    *y = norm_size;
}


void WorkVisual::draw_stretch(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_stretch[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_stretch[1][0]);
    glDrawArrays(GL_TRIANGLES, 0, vc_stretch[0].size());


    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 0.0f, 0.2f);

    std::ostringstream outstream;

    outstream.clear();
    outstream.str("");
    outstream << std::setprecision(3) << -stretch_min;
    meDisp->printText(norm_size + 20, 5, outstream.str());

    outstream.clear();
    outstream.str("");
    outstream << std::setprecision(3) << stretch_max;
    meDisp->printText(norm_size + 20, norm_size - 15, outstream.str());

    glEnable(GL_DEPTH_TEST);


    *x = norm_size + 50;
    *y = norm_size;
}


void WorkVisual::draw_plot(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_plot[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_plot[1][0]);
    glDrawArrays(GL_LINES, 0, vc_plot[0].size());
    
    *x = norm_size;
    *y = norm_size * 0.5;

    std::ostringstream outstream;
    outstream.clear();
    outstream.str("");
    outstream << log_max;
    meDisp->printText(10, *y-16, outstream.str());
}


void WorkVisual::draw_iactive(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_iactive[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_iactive[1][0]);
    glDrawArrays(GL_TRIANGLES, 0, vc_iactive[0].size());

    glColorPointer(3, GL_FLOAT, 0, &vc_grid[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_grid[1][0]);
    glDrawArrays(GL_LINES, 0, vc_grid[0].size());
    
    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 0.0f, 0.2f);

    glEnable(GL_DEPTH_TEST);

    *x = norm_size + 50;
    *y = norm_size;
}


void WorkVisual::draw_option(int* x, int* y, int rtime)
{
    int row = norm_size-40;
    int col = 40;

    std::ostringstream outstream;
    outstream.clear();
    outstream.str("");
    outstream << "time       " << rtime << " tsp";
    meDisp->printText(col, row, outstream.str());
    row -= 30;

    outstream.clear();
    outstream.str("");
    outstream << "max v      " << resti_max;
    meDisp->printText(col, row, outstream.str());
    row -= 30;

//    outstream.clear();
//    outstream.str("");
//    outstream << "max I_s    " << std::setprecision(3) << iactive_max;
//    meDisp->printText(col, row, outstream.str());
//    row -= 30;

    if (tea_set) {
        outstream.clear();
        outstream.str("");
        outstream << "Tear ups   " << std::setprecision(3) << tea_count;
        meDisp->printText(col, row, outstream.str());
    }



    *x = norm_size;
    *y = norm_size;
}


void WorkVisual::draw_phase(int* x, int* y)
{
    glColorPointer(3, GL_FLOAT, 0, &vc_phase[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_phase[1][0]);
    glDrawArrays(GL_TRIANGLES, 0, vc_phase[0].size());

    glColorPointer(3, GL_FLOAT, 0, &vc_grid[0][0]);
    glVertexPointer(3, GL_FLOAT, 0, &vc_grid[1][0]);
    glDrawArrays(GL_LINES, 0, vc_grid[0].size());

    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 0.0f, 0.2f);

    std::ostringstream outstream;

    outstream.clear();
    outstream.str("");
    outstream << std::setprecision(3) << -phase_min;
    meDisp->printText(norm_size + 20, 5, outstream.str());

    outstream.clear();
    outstream.str("");
    outstream << std::setprecision(3) << phase_max;
    meDisp->printText(norm_size + 20, norm_size - 15, outstream.str());

    glEnable(GL_DEPTH_TEST);

    *x = norm_size + 50;
    *y = norm_size;
}




// ==========================================================================================
// Update area


void WorkVisual::updt_field()
{
    vc_field[0].clear();
    vc_field[1].clear();
    
    double r_scale = norm_size / (double) area_size;
    
    // Electrophysiologic excitation
    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    double l_colex[] = {1.0, 1.0, 1.0};

    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {   
        color_elec_bright(l_colex, meModel->getMass(ida, idb)->u);
                        
        for (int i = 0; i < 6; i++) {
            vc_field[0].push_back(vect3(l_colex[0], l_colex[1], l_colex[2]));
            vc_field[1].push_back(vect3(
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
        }
    }


    vc_grid[0].clear();
    vc_grid[1].clear();

    // Grid for field
    int parts = area_size / grid_span;
    double l_colgr[] = {0.75, 0.75, 0.55};

    for (int ida = parts; ida < area_size-1; ida += parts)
    for (int idb = 2; idb < area_size; idb++)
    {
        for (int i = 0; i < 4; i++)
            vc_grid[0].push_back(vect3(l_colgr[0], l_colgr[1], l_colgr[2]));

        vc_grid[1].push_back(vect3(meModel->getMass(ida, idb-1)->curr_pos.a * r_scale, meModel->getMass(ida, idb-1)->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(ida, idb  )->curr_pos.a * r_scale, meModel->getMass(ida, idb  )->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(idb-1, ida)->curr_pos.a * r_scale, meModel->getMass(idb-1, ida)->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(idb,   ida)->curr_pos.a * r_scale, meModel->getMass(idb,   ida)->curr_pos.b * r_scale, 0.25));
    }

    for (int i = 0; i < 2; i++)
        vc_grid[0].push_back(vect3(0.25, 0.45, 0.55));

    vc_grid[1].push_back(vect3((area_size / 2 + ang_sin * (area_size / 2)) * r_scale, (area_size / 2 + ang_cos * (area_size / 2)) * r_scale, 0.35));
    vc_grid[1].push_back(vect3((area_size / 2 - ang_sin * (area_size / 2)) * r_scale, (area_size / 2 - ang_cos * (area_size / 2)) * r_scale, 0.35));
    
    
    
}


void WorkVisual::updt_resti()
{
    vc_resti[0].clear();
    vc_resti[1].clear();

    double curris;

    resti_max = 0.0;
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        curris = meModel->getMass(ida, idb)->v;
        resti_max = (curris > resti_max) ? curris : resti_max;
    }

    resti_max = (resti_max <= 0.0) ? 1.0 : resti_max;

    double r_scale = norm_size / (double) area_size, kehr_max = 1.0 / resti_max;

    // Electrophysiologic excitation
    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    double l_colex[] = {1.0, 1.0, 1.0};

    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        curris = meModel->getMass(ida, idb)->v;

        l_colex[0] = 1.0 - curris * kehr_max;
        l_colex[1] = 1.0 - curris * kehr_max * 0.5;
        l_colex[2] = 1.0;
        
        for (int i = 0; i < 6; i++) {
            vc_resti[0].push_back(vect3(l_colex[0], l_colex[1], l_colex[2]));
            vc_resti[1].push_back(vect3(
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
        }
    }
}


void WorkVisual::updt_stretch()
{
    vc_stretch[0].clear();
    vc_stretch[1].clear();

    double currarea;

    stretch_min = 1.0, stretch_max = 1.0;
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        currarea = meModel->getMass(ida, idb)->area;

        stretch_max = (currarea > stretch_max) ? currarea : stretch_max;
        stretch_min = (currarea < stretch_min) ? currarea : stretch_min;
    }

    stretch_max = stretch_max - 1.0;
    stretch_min = 1.0 - stretch_min;


    double r_scale = norm_size / (double) area_size, kehr_min = 1.0 / stretch_min, kehr_max = 1.0 / stretch_max;

    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    double l_colst[] = {1.0, 1.0, 1.0};

    // Mechanical deformation
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        currarea = meModel->getMass(ida, idb)->area;

        if (currarea == 1.0)
            l_colst[0] = l_colst[1] = l_colst[2] = 1.0;

        if (currarea < 1.0) {
            l_colst[1] = l_colst[0] = 1.0 - (1.0 - currarea) * kehr_min;
            l_colst[2] = 1.0;
        }

        if (currarea > 1.0) {
            l_colst[1] = l_colst[2] = 1.0 - (currarea - 1.0) * kehr_max;
            l_colst[0] = 1.0;
        }

        for (int i = 0; i < 6; i++) {
            vc_stretch[0].push_back(vect3(l_colst[0], l_colst[1], l_colst[2]));
            vc_stretch[1].push_back(vect3(
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
        }
    }

    // the colorbar
    double whpos = ((stretch_max == stretch_min) >= 1e-09) ? 0.5 : 0.9 / (stretch_max + stretch_min) * stretch_min + 0.05;

    double sidb[] = {
        0,0.05, 1,0.05, 1,whpos, 0,0.05, 1,whpos, 0,whpos,
        0,whpos, 1,whpos, 1,0.95, 0,whpos, 1,0.95, 0,0.95 };

    double sidc[] = {
        0,0,1, 0,0,1, 1,1,1, 0,0,1, 1,1,1, 1,1,1,
        1,1,1, 1,1,1, 1,0,0, 1,1,1, 1,0,0, 1,0,0 };

    for (int i = 0; i < 12; i++) {
        vc_stretch[0].push_back(vect3(sidc[i*3], sidc[i*3+1], sidc[i*3+2]));
        vc_stretch[1].push_back(vect3(
            norm_size + 20 + 30 * sidb[i*2],
            norm_size * sidb[i*2+1], 0));
    }
}


void WorkVisual::updt_plot()
{
    vc_plot[0].clear();
    vc_plot[1].clear();

    if (meModel->getRuntime() % log_step == 0) {
        plot_dat[0][log_pivo] = meModel->getMass(log_ptx, log_pty)->u;
        plot_dat[1][log_pivo] = meModel->getMass(log_ptx, log_pty)->v;
        plot_dat[2][log_pivo] = meModel->getMass(log_ptx, log_pty)->area - 1.0;
        log_pivo = (log_pivo+1) % log_last;
    }

    double h_scale, r_scale = norm_size / (double) log_last;

    log_max = 0.5;

    for (int i = 1; i < log_last-1; i++) {
        log_max = (plot_dat[0][i] < log_max) ? log_max : plot_dat[0][i];
        log_max = (plot_dat[1][i] < log_max) ? log_max : plot_dat[1][i];
    }
    h_scale = norm_size * 0.5 / log_max;


    // main plot data
    int now = log_pivo, next = now + 1;
    for (int i = 0; i < log_last-1; i++) {
        vc_plot[0].push_back(vect3(0.3, 0.6, 0.5));
        vc_plot[0].push_back(vect3(0.3, 0.6, 0.5));
        vc_plot[1].push_back(vect3(r_scale * i, plot_dat[0][now] * h_scale, 0.25));
        vc_plot[1].push_back(vect3(r_scale * (i+1), plot_dat[0][next] * h_scale, 0.25));

        vc_plot[0].push_back(vect3(0.3, 0.2, 0.9));
        vc_plot[0].push_back(vect3(0.3, 0.2, 0.9));
        vc_plot[1].push_back(vect3(r_scale * i, plot_dat[1][now] * h_scale, 0.25));
        vc_plot[1].push_back(vect3(r_scale * (i+1), plot_dat[1][next] * h_scale, 0.25));

        now  = (now+1) % log_last;
        next = (now+1) % log_last;
    }

    // frame
    for (double f = 0.0; f <= log_max; f += 0.5) {

        vc_plot[0].push_back(vect3(0.55, 0.65, 0.65));
        vc_plot[0].push_back(vect3(0.55, 0.65, 0.65));

        vc_plot[1].push_back(vect3(0, f * h_scale, 0));
        vc_plot[1].push_back(vect3(norm_size, f * h_scale, 0));
    }
}


void WorkVisual::updt_iactive()
{
    vc_iactive[0].clear();
    vc_iactive[1].clear();
    
    double curris;

    iactive_max = 0.0;
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        //curris = meModel->getMass(ida, idb)->is;
        curris = - meConf->val_Gs * (std::sqrt((meModel->getMass(ida,idb)->area >= 1.0) ? meModel->getMass(ida,idb)->area : 1.0) - 1.0) * (meModel->getMass(ida,idb)->u - meConf->val_Es) * meConf->area_sac;
        iactive_max = (curris > iactive_max) ? curris : iactive_max;
    }

    iactive_max = (iactive_max <= 0.0) ? 1.0 : iactive_max;
    
    double r_scale = norm_size / (double) area_size, kehr_max = 1.0 / iactive_max;
    
    // Electrophysiologic excitation
    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    double l_colex[] = {1.0, 1.0, 1.0};

    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {   
        
        //curris = meModel->getMass(ida, idb)->is;
        curris = - meConf->val_Gs * (std::sqrt((meModel->getMass(ida,idb)->area >= 1.0) ? meModel->getMass(ida,idb)->area : 1.0) - 1.0) * (meModel->getMass(ida,idb)->u - meConf->val_Es) * meConf->area_sac;
        
        if(curris > 0 && meModel->getMass(ida, idb)->u < 0.15){
            l_colex[0] = 1.0 - curris * kehr_max * 0.5;
            l_colex[1] = 1.0 - curris * kehr_max;
            l_colex[2] = 1.0;
        }
        else{
            color_elec_bright(l_colex, meModel->getMass(ida, idb)->u);
        }
                
        for (int i = 0; i < 6; i++) {
            vc_iactive[0].push_back(vect3(l_colex[0], l_colex[1], l_colex[2]));
            vc_iactive[1].push_back(vect3(
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
        }
    }


    vc_grid[0].clear();
    vc_grid[1].clear();

    // Grid for field
    int parts = area_size / grid_span;
    double l_colgr[] = {0.75, 0.75, 0.55};

    for (int ida = parts; ida < area_size-1; ida += parts)
    for (int idb = 2; idb < area_size; idb++)
    {
        for (int i = 0; i < 4; i++)
            vc_grid[0].push_back(vect3(l_colgr[0], l_colgr[1], l_colgr[2]));

        vc_grid[1].push_back(vect3(meModel->getMass(ida, idb-1)->curr_pos.a * r_scale, meModel->getMass(ida, idb-1)->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(ida, idb  )->curr_pos.a * r_scale, meModel->getMass(ida, idb  )->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(idb-1, ida)->curr_pos.a * r_scale, meModel->getMass(idb-1, ida)->curr_pos.b * r_scale, 0.25));
        vc_grid[1].push_back(vect3(meModel->getMass(idb,   ida)->curr_pos.a * r_scale, meModel->getMass(idb,   ida)->curr_pos.b * r_scale, 0.25));
    }

    for (int i = 0; i < 2; i++)
        vc_grid[0].push_back(vect3(0.25, 0.45, 0.55));

    vc_grid[1].push_back(vect3((area_size / 2 + ang_sin * (area_size / 2)) * r_scale, (area_size / 2 + ang_cos * (area_size / 2)) * r_scale, 0.35));
    vc_grid[1].push_back(vect3((area_size / 2 - ang_sin * (area_size / 2)) * r_scale, (area_size / 2 - ang_cos * (area_size / 2)) * r_scale, 0.35));
    
    
    
    
//    double curris;
//
//    iactive_max = 0.0;
//    for (int ida = 1; ida < area_size-1; ida++)
//    for (int idb = 1; idb < area_size-1; idb++)
//    {
//        curris = meModel->getMass(ida, idb)->is;
//        //curris = - meConf->val_Gs * (std::sqrt((meModel->getMass(ida,idb)->area >= 1.0) ? meModel->getMass(ida,idb)->area : 1.0) - 1.0) * (meModel->getMass(ida,idb)->u - meConf->val_Es) * meConf->area_sac;
//        iactive_max = (curris > iactive_max) ? curris : iactive_max;
//    }
//
//    iactive_max = (iactive_max <= 0.0) ? 1.0 : iactive_max;
//
//    double r_scale = norm_size / (double) area_size, kehr_max = 1.0 / iactive_max;
//
//    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
//    double l_colst[] = {1.0, 1.0, 1.0};
//
//    // Mechanical deformation
//    for (int ida = 1; ida < area_size-1; ida++)
//    for (int idb = 1; idb < area_size-1; idb++)
//    {
//        curris = meModel->getMass(ida, idb)->is;
//        //curris = - meConf->val_Gs * (std::sqrt((meModel->getMass(ida,idb)->area >= 1.0) ? meModel->getMass(ida,idb)->area : 1.0) - 1.0) * (meModel->getMass(ida,idb)->u - meConf->val_Es) * meConf->area_sac;
//        
//        l_colst[0] = 1.0 - curris * kehr_max * 0.5;
//        l_colst[1] = 1.0 - curris * kehr_max;
//        l_colst[2] = 1.0;
//        for (int i = 0; i < 6; i++) {
//            vc_iactive[0].push_back(vect3(l_colst[0], l_colst[1], l_colst[2]));
//            vc_iactive[1].push_back(vect3(
//                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
//                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
//        }
//    }
}


void WorkVisual::updt_phase()
{
    vc_phase[0].clear();
    vc_phase[1].clear();

    if (!tea_set)
        return;

    double currarea;

    phase_min = 0.0, phase_max = 0.0;
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        currarea = tea_phas2[area_size * idb + ida];

        phase_max = (currarea > phase_max) ? currarea : phase_max;
        phase_min = (currarea < phase_min) ? currarea : phase_min;
    }

    double r_scale = norm_size / (double) area_size, kehr_min = 1.0 / phase_min, kehr_max = 1.0 / phase_max;

    int sidv[] = { 0,0, 1,0, 1,1, 0,0, 1,1, 0,1 };
    double l_colst[] = {1.0, 1.0, 1.0};

    // Mechanical deformation
    for (int ida = 1; ida < area_size-1; ida++)
    for (int idb = 1; idb < area_size-1; idb++)
    {
        currarea = tea_phas2[area_size * idb + ida];

        if (currarea == 0.0)
            l_colst[0] = l_colst[1] = l_colst[2] = 1.0;

        if (currarea < 0.0) {
            l_colst[1] = l_colst[0] = 1.0 + currarea * kehr_min;
            l_colst[2] = 1.0;
        }

        if (currarea > 0.0) {
            l_colst[1] = l_colst[2] = 1.0 - currarea * kehr_max;
            l_colst[0] = 1.0;
//            std::stringstream strs;
//            strs << "../tea_positions.txt";
//
//            std::ofstream outfile;
//            outfile.open(strs.str().c_str(), std::ios::app);
//            outfile << rtime << "\t" << ida << "," << idb << "\n";
//            outfile.close();
        }

        for (int i = 0; i < 6; i++) {
            vc_phase[0].push_back(vect3(l_colst[0], l_colst[1], l_colst[2]));
            vc_phase[1].push_back(vect3(
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.a * r_scale,
                meModel->getMass(ida + sidv[i*2], idb + sidv[i*2+1])->curr_pos.b * r_scale, 0));
        }
    }
}





// ==========================================================================================
// Working area


void WorkVisual::run_step()
{
    meModel->make_step();

    if (log_run) log_tap(meModel->getRuntime());
    if (sac_run) sac_tap(meModel->getRuntime());
    if (tea_run) tea_tap(meModel->getRuntime());
    if (act_run) act_tap(meModel->getRuntime());
    if (mech_int) interval(meModel->getRuntime());
    
    hold_spirale();
    hold_stimulation(meModel->getRuntime());
    hold_chaos();
    hold_random(meModel->getRuntime());
    hold_stateload();
}

void WorkVisual::run_video_step()
{
    if (tea_run) tea_tap(meModel->getRuntime());
}

void WorkVisual::tog_mechanic()
{
    meModel->toogle_mechanic(2);
}


void WorkVisual::tog_feedback()
{
    meModel->toogle_feedback(2);
}

void WorkVisual::clear_field()
{
    meModel->clear_field();
}

double WorkVisual::get_scale()
{
    return area_size / (double) norm_size;
}

