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
#ifndef MESH_H
#define MESH_H

#include "engine/opengl.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

struct Mesh
{
    struct Vertex
    {
        glm::vec3 position;
        uint32_t color;
    };

    struct Object
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
        glm::vec4 color{1.0f};

        virtual ~Object() = default;

        virtual const char* typeString() const = 0;

        virtual Object* clone() const = 0;

        glm::mat4 makeMatrix() const;

        virtual void load(std::istringstream& ss) = 0;
        virtual void save(std::stringstream& ss) const = 0;

        virtual void bake(std::vector<Vertex>& vertices) = 0;
    };

    struct Cube : public Object
    {
        glm::vec3 p1;
        glm::vec3 p2;

        static const char* staticTypeString() { return "Cube"; }
        const char* typeString() const override { return staticTypeString(); }

        Object* clone() const override { return new Cube(*this); }

        void load(std::istringstream& ss) override;
        void save(std::stringstream& ss) const override;

        void bake(std::vector<Vertex>& vertices) override;
    };

    std::vector<std::unique_ptr<Object>> objects;
    std::vector<Vertex> vertices;
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    glm::vec3 bboxCenter;
    glm::vec3 bboxSize;

    void load(const std::string& file);
    void save(const std::string& file) const;

    void bake();
};

extern const glm::vec3 cubeVertices[36];

void meshInitCache();
void meshShutdownCache();

const std::shared_ptr<Mesh>& meshGetCached(const std::string& name);

#endif
