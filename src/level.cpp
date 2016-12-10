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

static const float COEFF = 32.0f;
static Sprite man1Sprite;
static GLuint wallpaperTexture;
static GLuint floorTexture;

Level::Level()
{
    Sector sector;
    sector.points.emplace_back(Point{ { -100.0f, -100.0f }, 0.0f, 60.0f });
    sector.points.emplace_back(Point{ {  100.0f, -100.0f }, 0.0f, 60.0f });
    sector.points.emplace_back(Point{ {  100.0f,  100.0f }, 0.0f, 60.0f });
    sector.points.emplace_back(Point{ { -100.0f,  100.0f }, 0.0f, 60.0f });
    mSectors.emplace_back(sector);

    Object2D object;
    object.pos = glm::vec3(0.0f);
    object.sprite = man1Sprite;
    mObjects.emplace_back(object);
}

Level::~Level()
{
}

void Level::loadResources()
{
    man1Sprite = spriteLoad("man1.sprite");
    wallpaperTexture = openglLoadTexture("wallpaper.png", RepeatXY, GL_NEAREST);
    floorTexture = openglLoadTexture("floor.png", RepeatXY, GL_NEAREST);
}

void Level::unloadResources()
{
    spriteDelete(man1Sprite);
    openglDeleteTexture(floorTexture);
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
    // Draw walls
    drawSetTexture(wallpaperTexture);
    drawBeginPrimitive(GL_TRIANGLES);
    for (const auto& sector : mSectors) {
        size_t n = sector.points.size();
        float prev = 0.0f;
        for (size_t i = 0; i < n; i++) {
            const auto& p1 = sector.points[i];
            const auto& p2 = sector.points[(i + 1) % n];

            float length = glm::length(p2.pos - p1.pos) / COEFF;
            float next = prev + length;

            drawVertex3D(glm::vec3(p1.pos, p1.minZ), glm::vec2(prev, 0.0f));
            auto v1 = drawVertex3D(glm::vec3(p1.pos, p1.maxZ), glm::vec2(prev, 1.0f));
            auto v2 = drawVertex3D(glm::vec3(p2.pos, p2.minZ), glm::vec2(next, 0.0f));

            drawIndex(v2);
            drawIndex(v1);
            drawVertex3D(glm::vec3(p2.pos, p2.maxZ), glm::vec2(next, 1.0f));

            prev = next;
        }
    }
    drawEndPrimitive();

    // Draw floor
    drawSetTexture(floorTexture);
    drawBeginPrimitive(GL_TRIANGLES);
    for (const auto& sector : mSectors) {
        size_t n = sector.points.size();
        for (size_t i = 2; i < n; i++) {
            const auto& p1 = sector.points[0];
            const auto& p2 = sector.points[i - 1];
            const auto& p3 = sector.points[i];

            drawVertex3D(glm::vec3(p1.pos, p1.minZ), p1.pos / COEFF);
            drawVertex3D(glm::vec3(p2.pos, p2.minZ), p2.pos / COEFF);
            drawVertex3D(glm::vec3(p3.pos, p3.minZ), p3.pos / COEFF);
        }
    }
    drawEndPrimitive();

    // Draw objects
    for (const auto& object : mObjects)
        drawBillboard(object.pos, object.sprite);
}

void Level::run(double time, int width, int height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));
    drawPushMatrix(glm::lookAt(glm::vec3(80.0f, 80.0f, 80.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
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
