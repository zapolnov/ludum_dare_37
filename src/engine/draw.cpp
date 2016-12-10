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
#include "draw.h"
#include "util.h"
#include <cassert>
#include <cstdint>
#include <vector>

static GLuint shader;
static GLuint dummyTexture;
static GLuint vertexBuffer;
static GLuint colorBuffer;
static GLuint indexBuffer;

static int attrPosition;
static int attrTexCoord;
static int attrColor;
static int uniformProjectionMatrix;
static int uniformTexture;

static const size_t VERTICES_PER_INDEX = 5;
static std::vector<GLfloat> vertices;
static std::vector<uint32_t> colors;
static std::vector<GLushort> indices;
static size_t vertexCount;
static size_t indexCount;
static GLenum currentPrimitiveType;
static GLuint currentTexture;

static glm::mat4 projectionMatrix;
static std::vector<std::pair<glm::vec4, uint32_t>> color;
static std::vector<glm::mat4> modelViewMatrix;

void drawInit()
{
    vertexBuffer = openglCreateBuffer();
    colorBuffer = openglCreateBuffer();
    indexBuffer = openglCreateBuffer();

    const uint8_t whitePixel = 0xFF;
    dummyTexture = openglCreateTexture(NoRepeat, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, dummyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &whitePixel);

    shader = openglLoadProgram("DrawV.glsl", "DrawF.glsl");
    attrPosition = glGetAttribLocation(shader, "aPosition");
    attrTexCoord = glGetAttribLocation(shader, "aTexCoord");
    attrColor = glGetAttribLocation(shader, "aColor");
    uniformProjectionMatrix = glGetUniformLocation(shader, "uProjectionMatrix");
    uniformTexture = glGetUniformLocation(shader, "uTexture");
}

void drawShutdown()
{
    openglDeleteTexture(dummyTexture);
    openglDeleteBuffer(vertexBuffer);
    openglDeleteBuffer(colorBuffer);
    openglDeleteBuffer(indexBuffer);
    glDeleteProgram(shader);
}

void drawBegin(const glm::mat4& projMatrix)
{
    vertices.clear();
    colors.clear();
    indices.clear();
    vertexCount = 0;
    indexCount = 0;
    currentPrimitiveType = 0;
    currentTexture = 0;

    projectionMatrix = projMatrix;

    modelViewMatrix.resize(1);
    modelViewMatrix[0] = glm::mat4(1.0f);

    color.resize(1);
    color[0] = std::make_pair(glm::vec4(1.0f), 0xFFFFFFFF);
}

void drawEnd()
{
    drawFlush();
}

const glm::mat4& drawGetMatrix()
{
    return modelViewMatrix.back();
}

void drawPushMatrix(const glm::mat4& matrix)
{
    modelViewMatrix.push_back(matrix);
}

void drawPopMatrix()
{
    assert(modelViewMatrix.size() > 1);
    if (modelViewMatrix.size() > 1)
        modelViewMatrix.pop_back();
}

const glm::vec4& drawGetColor()
{
    return color.back().first;
}

void drawPushColor(const glm::vec4& c)
{
    color.emplace_back(c, toUInt32(c));
}

void drawSetColor(const glm::vec4& c)
{
    assert(color.size() > 0);
    color.back().first = c;
    color.back().second = toUInt32(c);
}

void drawPopColor()
{
    assert(color.size() > 1);
    if (color.size() > 1)
        color.pop_back();
}

void drawSetTexture(GLuint texture)
{
    if (texture != currentTexture) {
        drawFlush();
        currentTexture = texture;
    }
}

void drawSprite(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& anchor, GLuint texture)
{
    glm::vec2 p1 = pos - size * anchor;
    glm::vec2 p2 = p1 + size;

    drawSetTexture(texture);
    drawBeginPrimitive(GL_TRIANGLES);
        GLushort v1 = drawVertex(glm::vec2(p1.x, p2.y), glm::vec2(0.0f, 1.0f));
        drawVertex(glm::vec2(p1.x, p1.y), glm::vec2(0.0f, 0.0f));
        GLushort v2 = drawVertex(glm::vec2(p2.x, p1.y), glm::vec2(1.0f, 0.0f));
        drawIndex(v1);
        drawIndex(v2);
        drawVertex(glm::vec2(p2.x, p2.y), glm::vec2(1.0f, 1.0f));
    drawEndPrimitive();
}

void drawSprite(const glm::vec2& pos, const Sprite& sprite)
{
    drawSprite(pos, sprite.size, sprite.anchor, sprite.texture);
}

void drawBillboard(const glm::vec3& pos, const Sprite& sprite)
{
    const auto& vm = drawGetMatrix();
    auto right = glm::vec3(vm[0][0], vm[1][0], vm[2][0]);
    auto up = -glm::vec3(vm[0][1], vm[1][1], vm[2][1]);

    glm::vec2 d1 = -sprite.size * sprite.anchor;
    glm::vec2 d2 = d1 + sprite.size;

    glm::vec3 p1 = pos + right * d1.x + up * d1.y;
    glm::vec3 p2 = pos + right * d2.x + up * d1.y;
    glm::vec3 p3 = pos + right * d1.x + up * d2.y;
    glm::vec3 p4 = pos + right * d2.x + up * d2.y;

    drawSetTexture(sprite.texture);
    drawBeginPrimitive(GL_TRIANGLES);
        GLushort v1 = drawVertex3D(p2, glm::vec2(1.0f, 0.0f));
        drawVertex3D(p1, glm::vec2(0.0f, 0.0f));
        GLushort v2 = drawVertex3D(p3, glm::vec2(0.0f, 1.0f));
        drawIndex(v1);
        drawIndex(v2);
        drawVertex3D(p4, glm::vec2(1.0f, 1.0f));
    drawEndPrimitive();
}

void drawMesh(const glm::vec3& pos, const Mesh& mesh)
{
    drawPushColor(glm::vec4(1.0f));
    drawBeginPrimitive(GL_TRIANGLES);

    for (const auto& v : mesh.vertices) {
        color.back().second = v.color;
        drawVertex3D(v.position);
    }

    drawEndPrimitive();
    drawPopColor();
}

void drawBeginPrimitive(GLenum primitiveType)
{
    if (currentPrimitiveType != primitiveType) {
        drawFlush();
        currentPrimitiveType = primitiveType;
    }
}

void drawEndPrimitive()
{
    vertexCount = vertices.size() / VERTICES_PER_INDEX;
    indexCount = indices.size();
}

GLushort drawVertex(const glm::vec2& pos, const glm::vec2& texCoord)
{
    return drawVertex3D(glm::vec3(pos, 0.0f), texCoord);
}

GLushort drawVertex3D(const glm::vec3& pos, const glm::vec2& texCoord)
{
    if (vertices.size() + VERTICES_PER_INDEX >= 0xFFFF) {
        assert(vertexCount > 0);
        drawFlush();
    }

    GLushort index = GLushort(vertices.size()) / VERTICES_PER_INDEX;
    assert(vertices.size() == colors.size() * VERTICES_PER_INDEX);

    assert(modelViewMatrix.size() > 0);
    glm::vec4 transformedPos = modelViewMatrix.back() * glm::vec4(pos, 1.0f);

    assert(color.size() > 0);
    vertices.emplace_back(transformedPos.x);
    vertices.emplace_back(transformedPos.y);
    vertices.emplace_back(transformedPos.z);
    vertices.emplace_back(texCoord.x);
    vertices.emplace_back(texCoord.y);
    colors.emplace_back(color.back().second);
    indices.emplace_back(index);

    return index;
}

void drawIndex(GLushort index)
{
    indices.emplace_back(index);
}

void drawFlush()
{
    if (vertexCount > 0 || indexCount > 0) {
        glUseProgram(shader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture != 0 ? currentTexture : dummyTexture);
        glUniform1i(uniformTexture, 0);
        glUniformMatrix4fv(uniformProjectionMatrix, 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLfloat) * VERTICES_PER_INDEX, vertices.data(), GL_STREAM_DRAW);
        glVertexAttribPointer(attrPosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * VERTICES_PER_INDEX, (void*)(sizeof(GLfloat) * 0));
        glVertexAttribPointer(attrTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * VERTICES_PER_INDEX, (void*)(sizeof(GLfloat) * 3));

        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(uint32_t), colors.data(), GL_STREAM_DRAW);
        glVertexAttribPointer(attrColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);

        glEnableVertexAttribArray(attrPosition);
        glEnableVertexAttribArray(attrTexCoord);
        glEnableVertexAttribArray(attrColor);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices.data(), GL_STREAM_DRAW);
        glDrawElements(currentPrimitiveType, indexCount, GL_UNSIGNED_SHORT, NULL);

        glDisableVertexAttribArray(attrPosition);
        glDisableVertexAttribArray(attrTexCoord);
        glDisableVertexAttribArray(attrColor);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vertices.erase(vertices.begin(), vertices.begin() + vertexCount * VERTICES_PER_INDEX);
        colors.erase(colors.begin(), colors.begin() + vertexCount);
        indices.erase(indices.begin(), indices.begin() + indexCount);

        for (auto& index : indices)
            index -= vertexCount;

        vertexCount = 0;
        indexCount = 0;
    }
}
