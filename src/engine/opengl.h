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
#ifndef OPENGL_H
#define OPENGL_H

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <string>

enum GLRepeatFlags
{
    NoRepeat = 0,
    RepeatX = 0x0001,
    RepeatY = 0x0002,
    RepeatXY = RepeatX | RepeatY,
};

GLuint openglCreateTexture(int repeat = NoRepeat, GLenum filter = GL_LINEAR);
GLuint openglLoadTexture(const std::string& file, int repeat = NoRepeat, GLenum filter = GL_LINEAR);
GLuint openglLoadTextureEx(const std::string& file, int* width, int* height, int repeat = NoRepeat, GLenum filter = GL_LINEAR);
void openglDeleteTexture(GLuint handle);

GLuint openglCreateBuffer();
void openglDeleteBuffer(GLuint handle);

GLuint openglCreateProgram();
GLuint openglLoadProgram(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
GLuint openglLoadShader(GLenum type, const std::string& file);
void openglLoadAttachShader(GLuint program, GLenum type, const std::string& file);
void openglLinkProgram(GLuint program);

GLuint openglCreateFramebuffer();
GLuint openglCreateRenderbuffer();
void openglDeleteFramebuffer(GLuint handle);
void openglDeleteRenderbuffer(GLuint handle);

#endif
