/*
 * Base.cpp
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



#include "Base.hpp"

Base::Base()
{
    now_shift[0] = now_shift[1] = 20;
    pre_shift[0] = pre_shift[1] = 20;

    act_wait = 0;
    act_pace = 0;
    act_play = 0;
}


Base::~Base()
{
    delete meDiffRe;
}

void Base::prepare_disp(Config* conf, Display* disp)
{
    meConf = conf;
    meDisp = disp;

    meDisp->objViewShift(0, 0);
    meDisp->selfCamRota(0, 0);


    glutDisplayFunc(show_disp);
    glutReshapeFunc(resize_disp);

    glutKeyboardFunc(key_down);
    glutKeyboardUpFunc(key_up);

    glutMouseFunc(mouse_down);
    glutMotionFunc(mouse_move);


    meDiffRe = new WorkVisual(meConf, meDisp);

    meDiffRe->updt_field();
    meDiffRe->updt_resti();
    meDiffRe->updt_plot();
    meDiffRe->updt_stretch();
    meDiffRe->updt_iactive();
    meDiffRe->updt_phase();

    shot_do     = meConf->shot_do;
    shot_intval = meConf->shot_intval;
    shot_curr   = shot_intval;
    shot_beg    = meConf->shot_beg;
    shot_end    = meConf->shot_end;
    
    video_beg   = meConf->video_beg;
    video_end   = meConf->video_end;
    video_int   = meConf->video_int;
    video_play  = 0;
    rtime       = video_beg;
    
    stim_do     = meConf->stim_do;
    stim_intval = meConf->stim_intval;
    stim_curr   = 0;

    wait_quit   = meConf->wait_quit;

    fcoord[4]   = meDiffRe->get_scale();

    glutTimerFunc(1000/20, timer_routine, 10);
    //if (meConf->tea_set) meDiffRe->tea_prep();
}


void Base::show_main()
{
    if (act_play) {
        if (act_wait-- <= 0) {
            act_wait = act_pace;
            
            meDiffRe->run_step();
            meDiffRe->run_step();
            meDiffRe->run_step();

            meDiffRe->run_step();
            meDiffRe->run_step();
            meDiffRe->run_step();

            meDiffRe->updt_resti();
            meDiffRe->updt_plot();

            meDiffRe->updt_field();
            meDiffRe->updt_stretch();
            //meDiffRe->updt_iactive();
            //meDiffRe->updt_phase();
            
            rtime = meDiffRe->meModel->getRuntime();
        }
    }
    
//    if(video_play){
//        
//        std::stringstream strs;
//        strs << "./data_t" << rtime << ".bin";
//        
//        meDiffRe->meModel->state_load(strs.str().c_str());
//        meDiffRe->run_video_step();
//        meDiffRe->updt_resti();
//        meDiffRe->updt_plot();
//        meDiffRe->updt_field();
//        meDiffRe->updt_stretch();
//        meDiffRe->updt_iactive();
//        meDiffRe->updt_phase();
//        
//        
//        rtime += video_int;
//        
//        if(rtime>=video_end){
//            rtime=video_beg;
//            toggleVideo();
//        }
//        
//    }
    
    meDisp->frontViewOrtho();
    meDisp->viewTranslate(now_shift[0], now_shift[1], 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);


    int x, y, tempx = 0, tempy = 0;
    
    meDiffRe->draw_field(&x, &y);
    
    meDisp->objTranslate(x+30, 0, 0);
    tempx += x+30;
    tempy = (y < tempy) ? tempy : y;

    meDiffRe->draw_stretch(&x, &y);
    tempx += x+30;

    meDisp->objTranslate(x+30, 0, 0);
    meDiffRe->draw_option(&x, &y, rtime);

    meDisp->objTranslate(-tempx, tempy+30, 0);
    meDiffRe->draw_resti(&x, &y);

    meDisp->objTranslate(x+30, 0, 0);
    //meDiffRe->draw_iactive(&x, &y);
    //meDiffRe->draw_phase(&x, &y);

    meDisp->objTranslate(x, 0, 0);
    meDiffRe->draw_plot(&x, &y);


    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);


    int currT = meDiffRe->meModel->getRuntime();

    if (shot_do && currT >= shot_curr && currT >= shot_beg && currT<= shot_end) {
        std::ostringstream outstream;
        outstream.clear();
        outstream.str("");
        outstream << "sim_" << std::setfill('0') << std::setw(6) << currT;

        meDisp->grabScreen(outstream.str(), 1);
        shot_curr += shot_intval;
    }

    if (stim_do && currT >= stim_curr) {
        meDiffRe->make_dot(meConf->init_sigx, meConf->init_sigy);
        stim_curr += stim_intval;
    }

    if (currT >= wait_quit) {
        std::cout << "# Automatic quit at " << currT << std::endl;
        glutLeaveMainLoop();
    }

}


void Base::togglePlay()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# animate = " << (act_play = 1 - act_play) << std::endl;
    return;
}

void Base::toggleVideo()
{
    video_play = 1 - video_play;
    if (meConf->term_quiet <= 1)
        std::cout << "# video animate = " << video_play << std::endl;
    return;
}

void Base::loop_disp()
{
    glutMainLoop();
}


void Base::increase_wait()
{
    wait_quit += 10000;
    std::cout << "# wait to quit extended to " << wait_quit << std::endl;
}


void Base::decrease_wait()
{
    wait_quit -= 10000;
    std::cout << "# wait to quit decreased to " << wait_quit << std::endl;
}


void Base::doinit_dot()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# dot at " << meConf->init_sigx << " " << meConf->init_sigy << std::endl;
    meDiffRe->make_dot(meConf->init_sigx, meConf->init_sigy);
}

void Base::doinit_spirale()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# spirale at " << meConf->init_sigx << " " << meConf->init_sigy << std::endl;
    meDiffRe->make_spirale(meConf->init_sigx, meConf->init_sigy);
}

void Base::doinit_stimulation()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# do stimulation" << std::endl;
    meDiffRe->make_stimulation();
}

void Base::doinit_chaos()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# start chaos at designated coordinates" << std::endl;
    meDiffRe->make_chaos();
}

void Base::doinit_random()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# random stimuli " << std::endl;
    meDiffRe->make_random();
}

void Base::doinit_stateload()
{
    if (meConf->term_quiet <= 1)
        std::cout << "# state loaded " << std::endl;
    meDiffRe->make_stateload();
}
//void Base::play_data(){
//    for(int i = video_beg; i <= video_end; i += video_int){
//        while(video_play==0){
//            show_disp();
//        }
//        
//        std::stringstream strs;
//        strs << "./Videodaten/data_t" << i << ".bin";
//        
//        meDiffRe->meModel->state_load(strs.str().c_str());
//        meDiffRe->updt_resti();
//        meDiffRe->updt_plot();
//        meDiffRe->updt_field();
//        meDiffRe->updt_stretch();
//        meDiffRe->updt_phase();
//        
//        rtime = i;
//        show_disp();
//    }
//}

Base lBase;


void show_disp()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lBase.show_main();

    glutSwapBuffers();
}


void resize_disp(int w, int h)
{

    lBase.meDisp->resizeWindow(w, h);
}


void timer_routine(int t)
{
    show_disp();
    glutPostRedisplay();

    glutTimerFunc(2, timer_routine, t);
}









// =================================================================================================
// key and mouse control - base


void key_down(unsigned char key, int x, int y)
{
    switch (key) {
    case 'p': case 'P':
        lBase.meDisp->grabScreen("simul", 1);
        break;
    case 'f': case 'F':
        glutFullScreenToggle();
        break;
    case 'c': case 'C':
        lBase.doinit_chaos();
        break;
    case 'd': case 'D':
        lBase.doinit_dot();
        break;
    case 's': case 'S':
        lBase.doinit_spirale();
        break;
    case 't': case 'T':
        lBase.doinit_stimulation();
        break;
    case 'r': case 'R':
        lBase.doinit_random();
        break;
    case 'l': case 'L':
        lBase.increase_wait();
        break;
    case 'k': case 'K':
        lBase.decrease_wait();
        break;
    case 'm': case 'M':
        lBase.meDiffRe->tog_mechanic();
        lBase.meDiffRe->tog_feedback();
        lBase.meDiffRe->clear_field();
        break;
    case 'n': case 'N':
        lBase.meDiffRe->tog_feedback();
        break;
        
    case 'v': case 'V':
        lBase.toggleVideo();
        break;
        
// experimental save and load function, use them wisely
    case '5':
        lBase.meDiffRe->meModel->state_save(0);
        break;
    case '4':
        lBase.doinit_stateload();
        break;
    case 'q': case 'Q': case 27:
        glutLeaveMainLoop();
        break;
    case ' ':
        lBase.togglePlay();
        glutPostRedisplay();
        break;
    default:
        break;
    }
}


void key_up(unsigned char key, int x, int y)
{
    // Noch leer

    glutPostRedisplay();
}


void mouse_down(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {

        switch (button) {
        case GLUT_RIGHT_BUTTON:
            break;
        case GLUT_LEFT_BUTTON:

            lBase.mouse_state = button;
            lBase.mouse_beg[0] = x;
            lBase.mouse_beg[1] = y;

            break;
        case GLUT_MIDDLE_BUTTON:
            break;
        case 3:     // mouse wheel up
    //        nbase.base_objScale += 0.25;
            break;
        case 4:     // mouse wheel down
    //        nbase.base_objScale = (nbase.base_objScale <= 1) ? nbase.base_objScale : nbase.base_objScale-0.25;
            break;
        default: break;
        }

    } else {

        switch (button) {
        case GLUT_RIGHT_BUTTON:
            break;
        case GLUT_LEFT_BUTTON:

            lBase.mouse_state = -1;
//            lBase.pre_shift[0] = lBase.now_shift[0];
//            lBase.pre_shift[1] = lBase.now_shift[1];

            break;
        default: break;
        }
    }
    glutPostRedisplay();
}


void mouse_move(int x, int y)
{
    switch (lBase.mouse_state) {
    case GLUT_LEFT_BUTTON:

//        std::cout << "draw " << x << " " << y << std::endl;

        lBase.meDiffRe->make_dot(
            lBase.fcoord[4] * (x - lBase.now_shift[0]),
            lBase.fcoord[4] * (lBase.meDisp->getHeight() - lBase.now_shift[1] - y));
//        if (x >= lBase.now_shift[0] && x <= lBase.now_shift[0] + 450 &&
//            y >= lBase.now_shift[1] && y <= lBase.now_shift[1] + 450) {

//        }

//        lBase.now_shift[0] = lBase.pre_shift[0] + (x - lBase.mouse_beg[0]);
//        lBase.now_shift[1] = lBase.pre_shift[1] + (lBase.mouse_beg[1] - y);

        break;
    default: break;
    }
    glutPostRedisplay();
}

