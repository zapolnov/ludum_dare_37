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
#include "level.h"
#include "game.h"
#include "engine/draw.h"
#include "engine/opengl.h"
#include "engine/gui.h"
#include <glm/gtc/matrix_transform.hpp>

static GLuint wallpaperTexture;

Level::Level()
{
    Sector sector;
    sector.points.emplace_back(Point{ { -10.0f, -10.0f }, 0.0f, 10.0f });
    sector.points.emplace_back(Point{ {  10.0f, -10.0f }, 0.0f, 10.0f });
    sector.points.emplace_back(Point{ {  10.0f,  10.0f }, 0.0f, 10.0f });
    sector.points.emplace_back(Point{ { -10.0f,  10.0f }, 0.0f, 10.0f });
    mSectors.emplace_back(sector);
}

Level::~Level()
{
}

void Level::loadResources()
{
    wallpaperTexture = openglLoadTexture("wallpaper.png", RepeatXY, GL_NEAREST);
}

void Level::unloadResources()
{
    openglDeleteTexture(wallpaperTexture);
}

void Level::draw2D() const
{
    drawBeginPrimitive(GL_LINES);

    for (const auto& sector : mSectors) {
        size_t n = sector.points.size();
        for (size_t i = 0; i < n; i++) {
            drawVertex(sector.points[i].pos);
            drawVertex(sector.points[(i + 1) % n].pos);
        }
    }

    drawEndPrimitive();
}

void Level::draw3D() const
{
    drawSetTexture(wallpaperTexture);
    drawBeginPrimitive(GL_TRIANGLES);

    for (const auto& sector : mSectors) {
        size_t n = sector.points.size();
        for (size_t i = 0; i < n; i++) {
            const auto& p1 = sector.points[i];
            const auto& p2 = sector.points[(i + 1) % n];

            drawVertex3D(glm::vec3(p1.pos, p1.minZ), glm::vec2(0.0f, 0.0f));
            drawVertex3D(glm::vec3(p1.pos, p1.maxZ), glm::vec2(0.0f, 1.0f));
            drawVertex3D(glm::vec3(p2.pos, p2.minZ), glm::vec2(1.0f, 0.0f));

            drawVertex3D(glm::vec3(p2.pos, p2.minZ), glm::vec2(1.0f, 0.0f));
            drawVertex3D(glm::vec3(p1.pos, p1.maxZ), glm::vec2(0.0f, 1.0f));
            drawVertex3D(glm::vec3(p2.pos, p2.maxZ), glm::vec2(1.0f, 1.0f));
        }
    }

    drawEndPrimitive();
}

void Level::run(double time, int width, int height)
{
    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));
    drawPushMatrix(glm::lookAt(glm::vec3(30.0f, 30.0f, 30.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    draw3D();
    drawEnd();

    /*
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    drawBegin(glm::ortho(-512.0f, 512.0f, 384.0f, -384.0f, -1.0f, 1.0f));
    draw2D();
    drawEnd();
    */
}
