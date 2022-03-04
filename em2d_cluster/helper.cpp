/*
 * helper.cpp
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

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

#include "helper.h"


std::string get_timedate_string() {
    
    time_t now  = std::time(0);
    std::tm *ltm = localtime(&now);

    std::stringstream strs;
    
    strs << 1900+ltm->tm_year <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_mon <<
        std::setfill('0') << std::setw(2) << ltm->tm_mday << "-" <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_hour <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_min <<
        std::setfill('0') << std::setw(2) << 1+ltm->tm_sec;
    
    return strs.str();
}