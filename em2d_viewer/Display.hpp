/*
 * Display.hpp
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


#ifndef DISPLAY_HPP
#define DISPLAY_HPP


#include <iostream>

#include <GL/freeglut.h>

#include <vector>
#include <fstream>

#include "../em2d_cluster/matr.h"
#include "../em2d_cluster/Config.hpp"


class Display
{
public:
    Display(Config *conf);
    ~Display();

    void init(std::string put_title, int argc, char **argv);

    int getWidth();
    int getHeight();

    void resizeWindow(int width, int height);

    void printText(float x, float y, std::string put_str);
    void printText3D(float x, float y, float z, std::string put_str);

    // == Camera ==

    void setView();

    void frontViewPersp();
    void frontViewOrtho();

    void selfCamRota(double xrot, double yrot);
    void objViewShift(double xrot, double yrot);

    void viewTranslate(double x, double y, double z);
    void viewScale(double sc);

    void objTranslate(double x, double y, double z);
    void objScale(double sc);
    void objRotate(double angle, double x, double y, double z);

    matr4 get_objModel();
    void set_objModel(matr4 put_view);

    void grabScreen(std::string put_name, int single);

private:
    int scrWidth, scrHeight;

    // == Camera ==

    int camHeadup;

    double
        camSelfXrota,
        camSelfYrota,

        camSelfXpos,    camSelfYpos,    camSelfZpos,
        camTargXpos,    camTargYpos,    camTargZpos;

    // model object thingies

    matr4
        infProject,
        infView,
        infModel;

    Config          *meConf;

    // main graphic stuffs

    std::string     scrTitle;

    void ins4byte(std::vector<char> &dest, size_t pos, const uint32_t data);
    void push4byte(std::vector<char> &dest, const uint32_t data);
    void push2byte(std::vector<char> &dest, const uint16_t data);
};

#endif /* DISPLAY_HPP */
