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
#include "opengl.h"
#include "util.h"
#include <vector>

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

GLuint openglLoadTexture(const std::string& file, int repeat, GLenum filter)
{
    std::string fileData = loadFile(file);

    int w = 0;
    int h = 0;
    int c = 0;
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(fileData.data()), int(fileData.size()), &w, &h, &c, 0);
    if (!pixels)
        fatalExit(fmt() << "Unable to decode image file \"" << file << "\": " << stbi_failure_reason());

    GLint internalFormat;
    GLenum type, format;
    switch (c) {
        case 1:
            internalFormat = GL_LUMINANCE;
            format = GL_LUMINANCE;
            type = GL_UNSIGNED_BYTE;
            break;

        case 2:
            internalFormat = GL_LUMINANCE_ALPHA;
            format = GL_LUMINANCE_ALPHA;
            type = GL_UNSIGNED_BYTE;
            break;

        case 3:
            internalFormat = GL_RGB;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;

        case 4:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;

        default:
            fatalExit(fmt() << "Image \"" << file << "\" has unsupported format.");
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0)
        fatalExit("Unable to create texture.");

    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, pixels);

    stbi_image_free(pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeat & RepeatX) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeat & RepeatY) ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    switch (filter) {
        case GL_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case GL_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case GL_NEAREST_MIPMAP_LINEAR:
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case GL_LINEAR_MIPMAP_NEAREST:
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;

        case GL_LINEAR_MIPMAP_LINEAR:
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
    }

    return texture;
}

void openglDestroyTexture(GLuint handle)
{
    glDeleteTextures(1, &handle);
}

GLuint openglCreateProgram()
{
    GLuint program = glCreateProgram();
    if (program == 0)
        fatalExit("Unable to create program.");
    return program;
}

GLuint openglLoadProgram(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
{
    GLuint program = openglCreateProgram();
    openglLoadAttachShader(program, GL_VERTEX_SHADER, vertexShaderFile);
    openglLoadAttachShader(program, GL_FRAGMENT_SHADER, fragmentShaderFile);
    openglLinkProgram(program);
    return program;
}

GLuint openglLoadShader(GLenum type, const std::string& file)
{
    std::string source = loadFile(file);

    GLuint shader = glCreateShader(type);
    if (shader == 0)
        fatalExit("Unable to create shader.");

    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint infoLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

        auto buffer = std::vector<char>(size_t(infoLength));
        glGetShaderInfoLog(shader, infoLength, nullptr, buffer.data());
        glDeleteShader(shader);

        fatalExit(fmt() << "Unable to compile shader \"" << file << "\":\n" << buffer.data());
    }

    return shader;
}

void openglLoadAttachShader(GLuint program, GLenum type, const std::string& file)
{
    GLuint shader = openglLoadShader(type, file);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void openglLinkProgram(GLuint program)
{
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint infoLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);

        auto buffer = std::vector<char>(size_t(infoLength));
        glGetProgramInfoLog(program, infoLength, nullptr, buffer.data());

        fatalExit(fmt() << "Unable to link program:\n" << buffer.data());
    }
}
