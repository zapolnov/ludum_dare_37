/*
 * Copyright (c) 2016 Nikolay Zapolnov (zapolnov@gmail.com)
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef LEVEL_H
#define LEVEL_H

#include "menu/gamescreen.h"
#include <glm/glm.hpp>
#include <vector>

struct Point
{
    glm::vec2 pos;
    float minZ;
    float maxZ;
};

struct Sector
{
    std::vector<Point> points;
};

class Level : public GameScreen
{
public:
    Level();
    ~Level();

    static void loadResources();
    static void unloadResources();

    void draw2D() const;
    void draw3D() const;

    void run(double time, int width, int height) override;

private:
    std::vector<Sector> mSectors;
};

#endif