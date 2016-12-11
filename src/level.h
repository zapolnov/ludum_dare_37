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

#include "engine/sprite.h"
#include "engine/mesh.h"
#include "menu/gamescreen.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Level : public GameScreen
{
public:
    struct Sector;

    struct Point
    {
        glm::vec2 pos;
        float minZ;
        float maxZ;
        std::weak_ptr<Sector> adjacentSector;
        std::weak_ptr<Point> adjacentPoint;
        int extraWallTex = -1;

        Point()
            : pos(0.0f)
            , minZ(0.0f)
            , maxZ(0.0f)
        {
        }

        Point(const glm::vec2& p, float mnZ, float mxZ)
            : pos(p)
            , minZ(mnZ)
            , maxZ(mxZ)
        {
        }

        Point(const glm::vec2& p, float mnZ, float mxZ, const std::shared_ptr<Sector>& as, const std::shared_ptr<Point>& ap)
            : pos(p)
            , minZ(mnZ)
            , maxZ(mxZ)
            , adjacentSector(as)
            , adjacentPoint(ap)
        {
        }
    };

    struct Sector
    {
        std::vector<std::shared_ptr<Point>> points;
    };

    struct StaticMesh
    {
        glm::vec3 pos{0.0f};
        glm::vec3 rot{0.0f};
        glm::vec3 scale{1.0f};
        std::string meshName;
        glm::mat4 matrix{1.0f};
        std::shared_ptr<Mesh> mesh;

        void loadMesh();
        void calcMatrix();
    };

    struct FlatSprite
    {
        glm::vec3 pos;
        Sprite sprite;
    };

    std::vector<std::shared_ptr<Sector>> sectors;
    std::vector<std::shared_ptr<StaticMesh>> meshes;
    std::vector<std::shared_ptr<FlatSprite>> sprites;

    Level();
    ~Level();

    static void loadResources();
    static void unloadResources();

    void draw3D() const;

    void load(const std::string& file);
    void save(const std::string& file) const;

    void run(double time, int width, int height) override;

private:
    void drawContents3D() const;
    void drawSprites() const;
};

extern bool ssaoEnabled;

#endif
