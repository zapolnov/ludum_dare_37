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
#ifndef DRAW_H
#define DRAW_H

#include "engine/opengl.h"
#include "engine/sprite.h"
#include "engine/mesh.h"
#include <glm/glm.hpp>

void drawInit();
void drawShutdown();

void drawBegin(const glm::mat4& projMatrix);
void drawEnd();

const glm::mat4& drawGetMatrix();
void drawPushMatrix(const glm::mat4& matrix);
void drawPopMatrix();

const glm::vec4& drawGetColor();
void drawPushColor(const glm::vec4& c);
void drawSetColor(const glm::vec4& c);
void drawPopColor();

void drawSetTexture(GLuint texture);
void drawSetLineWidth(float width);

void drawSprite(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& anchor, GLuint texture);
void drawSprite(const glm::vec2& pos, const Sprite& sprite);
void drawBillboard(const glm::vec3& pos, const Sprite& sprite);
void drawMesh(const glm::vec3& pos, const Mesh& mesh);

void drawBeginPrimitive(GLenum primitiveType);
void drawEndPrimitive();
GLushort drawVertex(const glm::vec2& pos, const glm::vec2& texCoord = glm::vec2(0.0f));
GLushort drawVertex3D(const glm::vec3& pos, const glm::vec2& texCoord = glm::vec2(0.0f));
void drawIndex(GLushort index);

void drawFlush();

#endif
