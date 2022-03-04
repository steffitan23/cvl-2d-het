/*
 * main.cpp
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


#include "../em2d_cluster/Config.hpp"
#include "Display.hpp"
#include "Base.hpp"


int main(int argc, char **argv)
{
    Config locConf = Config(argc, argv);
    locConf.show();

    std::string n_title = "Simul Viewer vT - 0.3";

    Display locDisp = Display(&locConf);
    locDisp.init(n_title, argc, argv);

    lBase.prepare_disp(&locConf, &locDisp);
    lBase.loop_disp();

    return 0;
}
