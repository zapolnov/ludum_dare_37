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
#include "mesh.h"
#include "util.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Mesh::Object::makeMatrix() const
{
    glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
    m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::scale(m, scale);
}

void Mesh::Object::load(std::istringstream& ss)
{
    ss >> position.x >> position.y >> position.z;
    ss >> rotation.x >> rotation.y >> rotation.z;
    ss >> scale.x >> scale.y >> scale.z;
    ss >> color.r >> color.g >> color.b >> color.a;
}

void Mesh::Object::save(std::stringstream& ss) const
{
    ss << typeString() << std::endl;
    ss << position.x << ' ' << position.y << ' ' << position.z << std::endl;
    ss << rotation.x << ' ' << rotation.y << ' ' << rotation.z << std::endl;
    ss << scale.x << ' ' << scale.y << ' ' << scale.z << std::endl;
    ss << color.r << ' ' << color.g << ' ' << color.b << ' ' << color.a << std::endl;
}

const glm::vec3 cubeVertices[36] = {
    { -1.0f, -1.0f, -1.0f },
    { -1.0f, -1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    {  1.0f,  1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f,  1.0f },
    { -1.0f, -1.0f,  1.0f },
    {  1.0f, -1.0f,  1.0f },
    {  1.0f,  1.0f,  1.0f },
    {  1.0f, -1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f },
    {  1.0f,  1.0f,  1.0f },
    {  1.0f, -1.0f,  1.0f },
    {  1.0f,  1.0f,  1.0f },
    {  1.0f,  1.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f },
    {  1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f, -1.0f },
    { -1.0f,  1.0f,  1.0f },
    {  1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    {  1.0f, -1.0f,  1.0f },
};

void Mesh::Cube::bake(std::vector<Vertex>& vertices)
{
    uint32_t c = toUInt32(color);
    auto m = makeMatrix();

    for (const auto& v : cubeVertices) {
        glm::vec3 p;
        p.x = (v.x < 0.0f ? p1.x : p2.x);
        p.y = (v.y < 0.0f ? p1.y : p2.y);
        p.z = (v.z < 0.0f ? p1.z : p2.z);
        p = glm::vec3(m * glm::vec4(p, 1.0f));
        vertices.emplace_back(Vertex{ p, c });
    }
}

void Mesh::Cube::load(std::istringstream& ss)
{
    Object::load(ss);
    ss >> p1.x >> p1.y >> p1.z;
    ss >> p2.x >> p2.y >> p2.z;
}

void Mesh::Cube::save(std::stringstream& ss) const
{
    Object::save(ss);
    ss << p1.x << ' ' << p1.y << ' ' << p1.z << std::endl;
    ss << p2.x << ' ' << p2.y << ' ' << p2.z << std::endl;
}

void Mesh::load(const std::string& file)
{
    std::string data = loadFile(file);
    std::istringstream ss(data);

    size_t n = 0;
    ss >> n;

    objects.clear();
    objects.reserve(n);

    while (n--) {
        std::string objectType;
        ss >> objectType;

        if (objectType == Cube::staticTypeString()) {
            auto object = std::unique_ptr<Cube>(new Cube);
            object->load(ss);
            objects.emplace_back(std::move(object));
        } else
            fatalExit(fmt() << "Unsupported object type \"" << objectType << "\" in file \"" << file << "\".");
    }

    bake();
}

void Mesh::save(const std::string& file) const
{
    std::stringstream ss;

    ss << objects.size() << std::endl;
    for (const auto& object : objects)
        object->save(ss);

    saveFile(file, ss.str());
}

void Mesh::bake()
{
    vertices.clear();

    for (const auto& object : objects)
        object->bake(vertices);

    if (vertices.empty()) {
        bboxMin = glm::vec3(0.0f);
        bboxMax = glm::vec3(0.0f);
        bboxCenter = glm::vec3(0.0f);
        bboxSize = glm::vec3(0.0f);
    } else {
        bboxMin = vertices[0].position;
        bboxMax = vertices[0].position;
        for (const auto& v : vertices) {
            bboxMin = glm::min(bboxMin, v.position);
            bboxMax = glm::max(bboxMax, v.position);
        }
        bboxCenter = (bboxMin + bboxMax) * 0.5f;
        bboxSize = (bboxMax - bboxMin);
    }
}
