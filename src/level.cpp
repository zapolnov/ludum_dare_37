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
#include "engine/util.h"
#include <map>
#include <glm/gtc/matrix_transform.hpp>

static const float COEFF = 32.0f;
static Sprite man1Sprite;
static GLuint wallpaperTexture;
static GLuint floorTexture;

Level::Level()
{
    // FIXME
    auto object = std::make_shared<FlatSprite>();
    object->pos = glm::vec3(0.0f);
    object->sprite = man1Sprite;
    sprites.emplace_back(object);
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

void Level::draw3D() const
{
    glDisable(GL_BLEND);

    // Draw walls
    drawSetTexture(wallpaperTexture);
    drawBeginPrimitive(GL_TRIANGLES);
    for (const auto& sector : sectors) {
        size_t n = sector->points.size();
        float prev = 0.0f;
        for (size_t i = 0; i < n; i++) {
            const auto& p1 = sector->points[i];

            auto adjacentSector = p1->adjacentSector.lock();
            if (adjacentSector) {
                continue;
            }

            const auto& p2 = sector->points[(i + 1) % n];

            float length = glm::length(p2->pos - p1->pos) / COEFF;
            float next = prev + length;

            drawVertex3D(glm::vec3(p1->pos, p1->minZ), glm::vec2(prev, 0.0f));
            auto v1 = drawVertex3D(glm::vec3(p1->pos, p1->maxZ), glm::vec2(prev, 1.0f));
            auto v2 = drawVertex3D(glm::vec3(p2->pos, p2->minZ), glm::vec2(next, 0.0f));

            drawIndex(v2);
            drawIndex(v1);
            drawVertex3D(glm::vec3(p2->pos, p2->maxZ), glm::vec2(next, 1.0f));

            prev = next;
        }
    }
    drawEndPrimitive();

    // Draw floor
    drawSetTexture(floorTexture);
    drawBeginPrimitive(GL_TRIANGLES);
    for (const auto& sector : sectors) {
        size_t n = sector->points.size();
        for (size_t i = 2; i < n; i++) {
            const auto& p1 = sector->points[0];
            const auto& p2 = sector->points[i - 1];
            const auto& p3 = sector->points[i];

            drawVertex3D(glm::vec3(p1->pos, p1->minZ), p1->pos / COEFF);
            drawVertex3D(glm::vec3(p2->pos, p2->minZ), p2->pos / COEFF);
            drawVertex3D(glm::vec3(p3->pos, p3->minZ), p3->pos / COEFF);
        }
    }
    drawEndPrimitive();
    drawFlush();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw objects
    for (const auto& object : sprites)
        drawBillboard(object->pos, object->sprite);
    drawFlush();

    glDisable(GL_BLEND);
}

void Level::load(const std::string& file)
{
    std::string data = loadFile(file);
    std::istringstream ss(data);

    size_t n = 0;
    ss >> n;
    sectors.clear();
    sectors.reserve(n);

    std::map<int, std::shared_ptr<Sector>> sectorIds;
    std::map<int, std::shared_ptr<Point>> pointIds;

    size_t j = n;
    while (j--) {
        int sectorId = -1;
        size_t nn = 0;
        ss >> sectorId >> nn;

        auto sector = std::make_shared<Sector>();
        sectorIds[sectorId] = sector;

        while (nn--) {
            int pointId = -1;
            ss >> pointId;

            auto point = std::make_shared<Point>();
            pointIds[pointId] = point;
            ss >> point->pos.x >> point->pos.y >> point->minZ >> point->maxZ;
            sector->points.emplace_back(std::move(point));
        }

        sectors.emplace_back(std::move(sector));
    }

    while (n--) {
        size_t nn = 0;
        ss >> nn;

        while (nn--) {
            int pointId = -1;
            ss >> pointId;
            auto point = pointIds[pointId];
            if (!point)
                fatalExit("Level file is corrupt.");

            int adjacentPointId = -1, adjacentSectorId = -1;
            ss >> adjacentSectorId >> adjacentPointId;
            point->adjacentSector = sectorIds[adjacentSectorId];
            point->adjacentPoint = pointIds[adjacentPointId];
        }
    }

    n = 0;
    ss >> n;
    // FIXME
    //sprites.clear();
    sprites.reserve(n);

    while (n--) {
        auto sprite = std::make_shared<FlatSprite>();
        ss >> sprite->pos.x >> sprite->pos.y >> sprite->pos.z;
        // FIXME
    }
}

void Level::save(const std::string& file) const
{
    std::stringstream ss;

    std::map<Sector*, int> sectorIds;
    std::map<Point*, int> pointIds;

    sectorIds[nullptr] = -1;
    pointIds[nullptr] = -1;

    ss << sectors.size() << std::endl;
    for (const auto& sector : sectors) {
        size_t sectorId = sectorIds.size();
        sectorIds[sector.get()] = int(sectorId);
        ss << sectorId << ' ' << sector->points.size() << std::endl;
        for (const auto& point : sector->points) {
            size_t pointId = pointIds.size();
            pointIds[point.get()] = int(pointId);
            ss << pointId << ' ' << point->pos.x << ' ' << point->pos.y << ' ' << point->minZ << ' ' << point->maxZ << std::endl;
        }
    }

    for (const auto& sector : sectors) {
        ss << sector->points.size() << std::endl;
        for (const auto& point : sector->points) {
            ss << pointIds[point.get()];
            ss << ' ' << sectorIds[point->adjacentSector.lock().get()];
            ss << ' ' << pointIds[point->adjacentPoint.lock().get()];
            ss << std::endl;
        }
    }

    ss << sprites.size() << std::endl;
    for (const auto& sprite : sprites) {
        ss << sprite->pos.x << ' ' << sprite->pos.y << ' ' << sprite->pos.z << std::endl;
        // FIXME
    }

    saveFile(file, ss.str());
}

void Level::run(double time, int width, int height)
{
    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));
    drawPushMatrix(glm::lookAt(glm::vec3(80.0f, 80.0f, 80.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    draw3D();
    drawEnd();
}
