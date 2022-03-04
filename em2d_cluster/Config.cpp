/*
 * Config.cpp
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


#include "Config.hpp"

#include <cmath>

Config::Config(int argc, char **argv)
{
    conf_loaded = 0;

    // ===== Magic numbers, the first instance, if there are missing to no parameters in the following data.

    // [Viewer]
    scr_sizew   = 1500;
    scr_sizeh   = 970;

    plot_last   = 400;

    // [Basic]
    area_size   = 100;
    area_edge   = 0;
    area_home   = 1.0;
    area_angle  = 0.0;

    term_quiet  = 0;

    log_ptx     = 40;
    log_pty     = 50;

    log_step    = 10;

    log_set     = 0;
    sac_set     = 0;
    tea_set     = 0;

    log_span[0] = 0;
    log_span[1] = 500;

    sac_span    = 1000;
    sac_thresh  = 0.05;

    tea_beg     = 10;
    tea_end     = 1000;
    tea_cmin    = 3;

    area_pad    = 2;
    area_mech   = 1;
    area_sac    = 1;

    area_aniso  = 0;
    area_angle  = 0.0;
    elec_aniso  = 0;
    
    wait_mech   = 1;
    wait_quit   = 999999;

    shot_do     = 0;
    shot_intval = 2000;

    init_sigx   = 50;
    init_sigy   = 50;
    init_mode   = 1;            // 1, which is a simple stimulus, 2 would be a spiral

    // [Electrophysiology]
    val_a   = 0.05;
    val_k   = 8.0;
    val_kT  = 1.5;
    val_Gs  = 2.0;
    val_Es  = 1.0;
    
    val_diff = 1.0;

    // [Mechanics]
    val_c   = 1.0;
    val_d   = 10.0;
    val_kap = 0.5;

    // [Integration]
    val_mt  = 0.01;
    val_dt  = 0.001;
    val_dx  = 0.3;
    
    // [Stimulation]
    stim_px = 100;
    stim_py = 100;

    // ===== Second and third instance, the hightest priority, it overwrites everything previously set
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--stim_px")  stim_px  = std::stoi(argv[++i]);
        if (std::string(argv[i]) == "--stim_py")  stim_py  = std::stoi(argv[++i]);
        if (std::string(argv[i]) == "--quiet")  term_quiet  = std::stoi(argv[++i]);
        if (std::string(argv[i]) == "--val_kT") val_kT      = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_k")  val_k       = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_diff")  val_diff       = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_a")  val_a       = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_e0") val_e0      = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_mu1")val_mu1     = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_d")  val_d       = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--val_Gs") val_Gs      = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--a_home") area_home   = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--a_ang")  area_angle  = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--seed")   seed  = std::stod(argv[++i]);
        if (std::string(argv[i]) == "--load") {
            prepare(argv[++i]);
            conf_loaded = 1;
        }
    }

    // Laplace
    if (elec_aniso) {
        elec_lap[0] = elec_lap[4] = 5.5;
        elec_lap[2] = elec_lap[6] = 1.0;
        elec_lap[1] = elec_lap[3] = elec_lap[5] = elec_lap[7] = 0.25;
    } else {
        elec_lap[0] = elec_lap[2] = elec_lap[4] = elec_lap[6] = 1.0;
        elec_lap[1] = elec_lap[3] = elec_lap[5] = elec_lap[7] = 1.0;
    }

    if (term_quiet <= 1) {
        if (conf_loaded)
            std::cout << "+ load info data '" << infopath << "'" << std::endl;
        else
            std::cout << "- no config loaded, use default values" << std::endl;
    }
}

Config::~Config()
{
}


/**
 * @brief load and prepare information out of the file from a given path
 * @param put_path string of full path to info file
 * @return 0 on errer
 */
void Config::prepare(const char* put_path)
{
    infopath = put_path;

    // ========================================================================================
    // Now start loading the info file

    std::fstream infFile(put_path);
    if (infFile.is_open()) {
        std::string ln_tmp;
        while (infFile) {
            infFile >> ln_tmp;

            if (ln_tmp[0] == '#') {
                std::getline(infFile, ln_tmp);
                continue;
            }

            if (ln_tmp == "scr_size")   infFile >> scr_sizew >> scr_sizeh;
//            if (ln_tmp == "plot_last")  infFile >> plot_last;

            if (ln_tmp == "area_size")  infFile >> area_size;
//            if (ln_tmp == "area_edge")  infFile >> area_edge;
            if (ln_tmp == "area_home")  infFile >> area_home;

            if (ln_tmp == "term_quiet") infFile >> term_quiet;

            if (ln_tmp == "log_loca")   infFile >> log_ptx >> log_pty;
            if (ln_tmp == "log_step")   infFile >> log_step;

            if (ln_tmp == "log_set")    infFile >> log_set;
            if (ln_tmp == "log_span")   infFile >> log_span[0] >> log_span[1];

            if (ln_tmp == "sac_set")    infFile >> sac_set;
            if (ln_tmp == "sac_until")  infFile >> sac_maxnum >> sac_span;
            if (ln_tmp == "sac_thresh") infFile >> sac_thresh;
            
            if (ln_tmp == "act_scan")   infFile >> act_scan;
            if (ln_tmp == "act_end")    infFile >> act_end;
            if (ln_tmp == "seed")       infFile >> seed;
            
            if (ln_tmp == "comp_time")  infFile >> comp_time;
            
            if (ln_tmp == "tea_time")   infFile >> tea_time;
            if (ln_tmp == "mech_int")   infFile >> mech_int >> mech_int_time;
            if (ln_tmp == "save_data")  infFile >> save_run;
            
            if (ln_tmp == "video")      infFile >> video_beg >> video_end >> video_int;
            
            if (ln_tmp == "AP_model")   infFile >> AP_model;
            
            if (ln_tmp == "tea_set")    infFile >> tea_set;
            if (ln_tmp == "tea_cmin")   infFile >> tea_cmin;
            if (ln_tmp == "tea_span")   infFile >> tea_beg >> tea_end;

            if (ln_tmp == "area_pad")   infFile >> area_pad;
            if (ln_tmp == "area_mech")  infFile >> area_mech;
            if (ln_tmp == "area_sac")   infFile >> area_sac;

            if (ln_tmp == "area_aniso") infFile >> area_aniso >> area_angle;
            if (ln_tmp == "elec_aniso") infFile >> elec_aniso;

            if (ln_tmp == "elec_lap") {
                for (int i = 0; i < 8; i++)
                     infFile >> elec_lap[i];
            }

            if (ln_tmp == "shot_do")    infFile >> shot_do >> shot_intval;
            if (ln_tmp == "shot_be")    infFile >> shot_beg >> shot_end;
            
            if (ln_tmp == "stim_do")    infFile >> stim_do >> stim_intval;

            if (ln_tmp == "wait_mech")  infFile >> wait_mech;
            if (ln_tmp == "wait_quit")  infFile >> wait_quit;

            if (ln_tmp == "init_sig")   infFile >> init_sigx >> init_sigy;
            if (ln_tmp == "init_mode")  infFile >> init_mode;

            if (ln_tmp == "val_a")      infFile >> val_a;
            if (ln_tmp == "val_k")      infFile >> val_k;
            if (ln_tmp == "val_diff")   infFile >> val_diff;
            if (ln_tmp == "val_e0")     infFile >> val_e0;
            if (ln_tmp == "val_mu1")    infFile >> val_mu1;
            if (ln_tmp == "val_mu2")    infFile >> val_mu2;
            
            if (ln_tmp == "val_kT")     infFile >> val_kT;
            if (ln_tmp == "val_Gs")     infFile >> val_Gs;
            if (ln_tmp == "val_Es")     infFile >> val_Es;
            if (ln_tmp == "val_dt")     infFile >> val_dt;
            if (ln_tmp == "val_dx")     infFile >> val_dx;

            if (ln_tmp == "val_c")      infFile >> val_c;
            if (ln_tmp == "val_d")      infFile >> val_d;

            if (ln_tmp == "val_kap")    infFile >> val_kap;
            if (ln_tmp == "val_mt")     infFile >> val_mt;

            // here for more options
        }
        infFile.close();
    } else {
        throw std::runtime_error("  - cannot find or open info file, stop here.");
    }

    return;
}


/**
 * @brief This cleans up any given string by trimming whitespaces at the beginning and ending
 * @param put_str
 * @return the cleaned string
 */
std::string Config::clean_str(std::string put_str)
{
    size_t st_beg, st_end;

    // remove comment at the end, if any
    st_end = put_str.find_first_of("#");
    put_str = put_str.substr(0, st_end);

    // remove trailing whitespace
    st_beg = put_str.find_first_not_of(" \t\n\v\f\r");
    st_end = put_str.find_last_not_of(" \t\n\v\f\r");

    return put_str.substr(st_beg, st_end - st_beg + 1);
}


void Config::show()
{
    if (term_quiet >= 1)
        return;

    std::cout << std::endl;

    std::cout << "  [Viewer]\n";
    std::cout << "    Window size\t\t" << scr_sizew << " x " << scr_sizeh << "\n";
    std::cout << "    Plot last\t\t" << plot_last << "\n";
    std::cout << "    run until\t\t" << wait_quit << " tsp\n";

    std::cout << "  [Basics]\n";
    std::cout << "    Area size\t\t" << area_size << "\n";
    std::cout << "    Area padding\t" << area_pad << "\n";
    std::cout << "    Area edge\t\t" << area_edge << "\n";
    std::cout << "    Area home\t\t" << area_home << "\n";
    std::cout << "    Mechanic aniso\t" << ((area_aniso) ? "yes" : "no") << "\n";
    std::cout << "    Electrical aniso\t" << ((elec_aniso) ? "yes" : "no") << "\n";
    if (area_aniso)
        std::cout << "      Anisotropy angle\t" << area_angle << " degree" << "\n";

    std::cout << "    Area SAC\t\t" << ((area_sac) ? "yes" : "no") << "\n";

    std::cout << "    Laplace\t\t";
    for (int i = 0; i < 8; i++)
         std::cout << elec_lap[i] << " ";
    std::cout << "\n";

    std::cout << "    Logging\t\t" << ((log_set) ? "yes" : "no") << "\n";
    if (log_set) {
        std::cout << "      At\t\t" << log_ptx << " " << log_pty << "\n";
        std::cout << "      Span\t\t" << log_span[0] << " to " << log_span[1] << "\n";
        std::cout << "      Step\t\t" << log_step << "\n";
    }

    std::cout << "    Scan SAC\t\t" << ((sac_set) ? "yes" : "no") << "\n";
    if (sac_set) {
        std::cout << "      Max num\t\t" << sac_maxnum << "\n";
        std::cout << "      Span\t\tuntil " << sac_span << "\n";
        std::cout << "      Threshold\t\t" << sac_thresh << "\n";
    }

    std::cout << "    Scan Tear ups\t\t" << ((tea_set) ? "yes" : "no") << "\n";
    if (tea_set) {
        std::cout << "      Count\tuntil " << tea_cmin << "\n";
        std::cout << "      Span\t\t " << tea_beg << " to " << tea_end << "\n";
    }

    std::cout << "    Signal init\t\tat x" << init_sigx << " y" << init_sigy << "\n";

    std::cout << "  [Electrophysiology]\n";
    std::cout << "    a\t\t\t" << val_a << "\n";
    std::cout << "    k\t\t\t" << val_k << "\n";
    std::cout << "    kT\t\t\t" << val_kT << "\n";
    std::cout << "    Gs\t\t\t" << val_Gs << "\n";
    std::cout << "    Es\t\t\t" << val_Es << "\n";

    std::cout << "  [Mechanics]" << "\n";
    std::cout << "    c\t\t\t" << val_c << "\n";
    std::cout << "    d\t\t\t" << val_d << "\n";
    std::cout << "    kappa\t\t" << val_kap << "\n";

    std::cout << "  [Integration]" << "\n";
    std::cout << "    mt\t\t\t" << val_mt << "\n";
    std::cout << "    dt\t\t\t" << val_dt << "\n";
    std::cout << "    dx\t\t\t" << val_dx << "\n";

    std::cout << std::endl;
}
