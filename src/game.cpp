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
#include "game.h"
#include "level.h"
#include "engine/opengl.h"
#include "engine/draw.h"
#include <glm/gtc/matrix_transform.hpp>

static GLuint texture;
static Level level;

void gameInit()
{
    texture = openglLoadTexture("test.png");
}

void gameShutdown()
{
    openglDeleteTexture(texture);
}

void gameRunFrame(double frameTime, int width, int height)
{
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));
    drawPushMatrix(glm::lookAt(glm::vec3(30.0f, 30.0f, 30.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    level.draw3D();
    drawEnd();

    glClear(GL_DEPTH_BUFFER_BIT);

    drawBegin(glm::ortho(-512.0f, 512.0f, 384.0f, -384.0f, -1.0f, 1.0f));
    level.draw2D();
    /*
    drawSetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawSprite(glm::vec2(-100.0f), glm::vec2(128.0f, 256.0f), glm::vec2(0.5f), texture);
    drawSetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    drawSprite(glm::vec2(100.0f), glm::vec2(128.0f, 256.0f), glm::vec2(0.5f), texture);
    */
    drawEnd();
}
