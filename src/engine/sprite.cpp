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
#include "sprite.h"
#include "util.h"
#include <sstream>

Sprite spriteLoad(const std::string& file, GLenum filter)
{
    std::string data = loadFile(file);

    std::stringstream ss(data);
    std::string textureFile;
    glm::vec2 anchor;
    ss >> textureFile >> anchor.x >> anchor.y;

    int width = 0;
    int height = 0;
    GLuint texture = openglLoadTextureEx(textureFile, &width, &height, NoRepeat, filter);

    Sprite sprite;
    sprite.texture = texture;
    sprite.size = glm::vec2(float(width), float(height));
    sprite.anchor = anchor;

    return sprite;
}

void spriteDelete(Sprite& sprite)
{
    openglDeleteTexture(sprite.texture);
}
