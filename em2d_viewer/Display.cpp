/*
 * Display.cpp
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


#include "Display.hpp"
#include <ctime>

Display::Display(Config *conf)
{
    meConf = conf;

    scrWidth    = meConf->scr_sizew;
    scrHeight   = meConf->scr_sizeh;
}


Display::~Display()
{
    // nothing for now
}


void Display::init(std::string put_title, int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH) - scrWidth) * 0.5,
        (glutGet(GLUT_SCREEN_HEIGHT) - scrHeight) * 0.5);
    glutInitWindowSize(scrWidth, scrHeight);
    glutCreateWindow(put_title.c_str());

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutIgnoreKeyRepeat(1);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.86, 0.85, 0.85, 1.0);
}

int Display::getWidth()
{
    return scrWidth;
}

int Display::getHeight()
{
    return scrHeight;
}

void Display::resizeWindow(int width, int height)
{
    scrHeight = height;
    scrWidth = width;

    glViewport(0, 0, scrWidth, scrHeight);
}

void Display::printText(float x, float y, std::string put_str)
{
    glRasterPos2f(x, y);
    for (unsigned int i = 0; i < put_str.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, put_str[i]);
}

void Display::printText3D(float x, float y, float z, std::string put_str)
{
    glRasterPos3f(x, y, z);
//    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ' ');
    for (unsigned int i = 0; i < put_str.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, put_str[i]);
}





// =================================================================================================
// view and camera control - display


void Display::setView()
{
    camSelfXpos = std::sin(mat_radians(camSelfXrota)) * std::cos(mat_radians(camSelfYrota));
    camSelfYpos = std::sin(mat_radians(camSelfYrota));
    camSelfZpos = std::cos(mat_radians(camSelfXrota)) * std::cos(mat_radians(camSelfYrota));

    infView.lookAt(
        // where the camera is
        vect3(camSelfXpos, camSelfYpos, camSelfZpos),
        // where the camera look at
        vect3(0, 0, 0),
        // how the camera is rotated
        vect3(0, camHeadup, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    infProject.translate(camTargXpos * 0.025, camTargYpos * 0.025, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::frontViewPersp()
{
    infProject = setPerspective(45.0, (double)scrWidth / (double)scrHeight, 0.1, 200.0);

    infModel = matr4();

    setView();
}

void Display::frontViewOrtho()
{
    infProject = setOrtho(0.0, (double)scrWidth, 0.0, (double)scrHeight, -10.0, 10.0);

    infView = matr4();
    infModel = matr4();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::selfCamRota(double xrot, double yrot)
{
    camSelfXrota = xrot;
    camSelfYrota = yrot;

    camHeadup = (camSelfYrota >= 90 || camSelfYrota <= -90) ? -1 : 1;
    setView();
}

void Display::objViewShift(double xshift, double yshift)
{
    camTargXpos = -xshift;
    camTargYpos = -yshift;
    setView();
}

void Display::viewTranslate(double x, double y, double z)
{
    infView.translate(x, y, z);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::viewScale(double sc)
{
    infView.scale(sc);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::objTranslate(double x, double y, double z)
{
    infModel.translate(x, y, z);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::objScale(double sc)
{
    infModel.scale(sc);
    glLoadMatrixf((infProject * infView * infModel).get());
}

void Display::objRotate(double angle, double x, double y, double z)
{
    infModel.rotate(angle, x, y, z);
    glLoadMatrixf((infProject * infView * infModel).get());
}

matr4 Display::get_objModel()
{
    return infModel;
}

void Display::set_objModel(matr4 put_view)
{
    infModel = put_view;
    glLoadMatrixf((infProject * infView * infModel).get());
}





// =================================================================================================
// Routine for saving Images of current object, only a screenshot

void Display::ins4byte(std::vector<char> &dest, size_t pos, const uint32_t data)
{
    dest[pos] = data & 0xFF;
    dest[pos + 1] = (data >> 8) & 0xFF;
    dest[pos + 2] = (data >> 16) & 0xFF;
    dest[pos + 3] = (data >> 24) & 0xFF;
}

void Display::push4byte(std::vector<char> &dest, const uint32_t data)
{
    dest.push_back(data & 0xFF);
    dest.push_back((data >> 8) & 0xFF);
    dest.push_back((data >> 16) & 0xFF);
    dest.push_back((data >> 24) & 0xFF);
}

void Display::push2byte(std::vector<char> &dest, const uint16_t data)
{
    dest.push_back(data & 0xFF);
    dest.push_back((data >> 8) & 0xFF);
}

void Display::grabScreen(std::string put_name, int single)
{
    unsigned char * pixels = new unsigned char[scrWidth * scrHeight * 3]; // 3 bytes for RGB
    glReadPixels(0, 0, scrWidth, scrHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // prepare bmp stuff

    std::vector<char> bmdat;

    // BMP Header
    bmdat.push_back('B');
    bmdat.push_back('M');
    push4byte(bmdat, 0);            // # total size, change later
    push2byte(bmdat, 0);
    push2byte(bmdat, 0);
    push4byte(bmdat, 54);

    // DIB Header
    push4byte(bmdat, 40);
    push4byte(bmdat, scrWidth);     // width
    push4byte(bmdat, scrHeight);    // height
    push2byte(bmdat, 1);
    push2byte(bmdat, 24);
    push4byte(bmdat, 0);            // no compression
    push4byte(bmdat, 0);            // # raw size, change later
    push4byte(bmdat, 2835);
    push4byte(bmdat, 2835);
    push4byte(bmdat, 0);
    push4byte(bmdat, 0);

    for (int y = 0; y < scrHeight; y++) {
        for (int x = 0; x < scrWidth; x++) {
            bmdat.push_back(pixels[(y * scrWidth + x) * 3 + 2]);
            bmdat.push_back(pixels[(y * scrWidth + x) * 3 + 1]);
            bmdat.push_back(pixels[(y * scrWidth + x) * 3 + 0]);
        }
        while ((bmdat.size() - 54) % 4)
            bmdat.push_back(0);
    }

    ins4byte(bmdat, 0x02, bmdat.size());
    ins4byte(bmdat, 0x22, bmdat.size() - 54);

    delete [] pixels;

    // now write data

    char buffer[256];
    if (single) {
        time_t t = time(0);
        struct tm *now = localtime(& t);
        snprintf(buffer, 100, "img_%s_%i%i%i_%02i%02i%02i.bmp",
                 put_name.c_str(), (now->tm_year + 1900), (now->tm_mon + 1),
                 now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec
                );
    } else {
        snprintf(buffer, 100, "%s.bmp", put_name.c_str());
    }

    std::ofstream file_output;

    file_output.open(buffer);
    file_output.write((const char*)&bmdat[0], bmdat.size());
    file_output.close();

    bmdat.clear();

    std::cout << "** save screenshot '" << buffer << "'" << std::endl;
}
