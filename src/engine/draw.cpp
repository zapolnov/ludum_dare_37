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

static const size_t VERTICES_PER_INDEX = 5;
static const size_t MAX_RENDERTARGETS = 3;

namespace
{
    struct ShaderInfo
    {
        GLuint handle;
        int attrPosition;
        int attrTexCoord;
        int attrColor;
        int uniformProjectionMatrix;
        int uniformTexture;
        int uniformRandomizerTexture;
        int uniformAuxTexture[MAX_RENDERTARGETS];
        int uniformViewportSize;
    };
}

static GLuint dummyTexture;
static GLuint ssaoRandomizerTexture;
static GLuint vertexBuffer;
static GLuint colorBuffer;
static GLuint indexBuffer;
static GLuint quadVertexBuffer;
static ShaderInfo shaders[ShaderCount];

static std::vector<GLfloat> vertices;
static std::vector<uint32_t> colors;
static std::vector<GLushort> indices;
static size_t vertexCount;
static size_t indexCount;
static Shader currentShader = Shader_Default;
static GLenum currentPrimitiveType;
static GLuint currentTexture;
static GLuint framebuffer;
static GLuint renderbuffer;
static GLuint renderTargets[MAX_RENDERTARGETS];
static int framebufferWidth = -1;
static int framebufferHeight = -1;
static float currentLineWidth;

static glm::mat4 projectionMatrix;
static std::vector<std::pair<glm::vec4, uint32_t>> color;
static std::vector<glm::mat4> modelViewMatrix;

static const unsigned char ssaoRandomizerPixels[] = {
    0x96, 0x7B, 0xFE, 0xFF, 0x7F, 0x03, 0x61, 0xFF, 0xA4, 0xF6, 0x63, 0xFF, 0x9B, 0xB1, 0x0E, 0xFF,
    0x36, 0x53, 0xDD, 0xFF, 0x02, 0x8E, 0x8F, 0xFF, 0x20, 0x39, 0x4F, 0xFF, 0x31, 0xA0, 0x20, 0xFF,
    0x39, 0xE8, 0x73, 0xFF, 0xB2, 0xD8, 0xCB, 0xFF, 0x46, 0xC4, 0xDA, 0xFF, 0xF1, 0xA4, 0x52, 0xFF,
    0xE1, 0x38, 0x55, 0xFF, 0xE9, 0x58, 0xBD, 0xFF, 0x90, 0x19, 0xCB, 0xFF, 0x75, 0x49, 0x0C, 0xFF,
};

static void loadShader(Shader shader, const std::string& vertex, const std::string& fragment)
{
    shaders[shader].handle = openglLoadProgram(vertex, fragment);
    shaders[shader].attrPosition = glGetAttribLocation(shaders[shader].handle, "aPosition");
    shaders[shader].attrTexCoord = glGetAttribLocation(shaders[shader].handle, "aTexCoord");
    shaders[shader].attrColor = glGetAttribLocation(shaders[shader].handle, "aColor");
    shaders[shader].uniformProjectionMatrix = glGetUniformLocation(shaders[shader].handle, "uProjectionMatrix");
    shaders[shader].uniformTexture = glGetUniformLocation(shaders[shader].handle, "uTexture");
    shaders[shader].uniformRandomizerTexture = glGetUniformLocation(shaders[shader].handle, "uRandomizerTexture");
    shaders[shader].uniformViewportSize = glGetUniformLocation(shaders[shader].handle, "uViewportSize");

    for (size_t i = 0; i < MAX_RENDERTARGETS; i++) {
        std::string name = fmt() << "uAuxTexture" << i;
        shaders[shader].uniformAuxTexture[i] = glGetUniformLocation(shaders[shader].handle, name.c_str());
    }
}

void drawInit()
{
    vertexBuffer = openglCreateBuffer();
    colorBuffer = openglCreateBuffer();
    indexBuffer = openglCreateBuffer();

    quadVertexBuffer = openglCreateBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    static const GLfloat quadData[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);

    const uint8_t whitePixel = 0xFF;
    dummyTexture = openglCreateTexture(NoRepeat, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, dummyTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &whitePixel);

    ssaoRandomizerTexture = openglCreateTexture(RepeatXY, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, ssaoRandomizerTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, ssaoRandomizerPixels);

    framebuffer = openglCreateFramebuffer();
    renderbuffer = openglCreateRenderbuffer();
    for (size_t i = 0; i < MAX_RENDERTARGETS; i++)
        renderTargets[i] = openglCreateTexture(NoRepeat, GL_NEAREST);

    loadShader(Shader_Default, "DrawDefaultV.glsl", "DrawDefaultF.glsl");
    loadShader(Shader_Depth, "DrawDepthV.glsl", "DrawDepthF.glsl");
    loadShader(Shader_SSAO, "DrawSsaoV.glsl", "DrawSsaoF.glsl");
    loadShader(Shader_Blur, "DrawBlurV.glsl", "DrawBlurF.glsl");
    loadShader(Shader_FromFramebuffer, "DrawFromFramebufferV.glsl", "DrawFromFramebufferF.glsl");
}

void drawShutdown()
{
    openglDeleteFramebuffer(framebuffer);
    openglDeleteRenderbuffer(renderbuffer);
    for (size_t i = 0; i < MAX_RENDERTARGETS; i++)
        openglDeleteTexture(renderTargets[i]);

    openglDeleteTexture(dummyTexture);
    openglDeleteTexture(ssaoRandomizerTexture);

    openglDeleteBuffer(quadVertexBuffer);
    openglDeleteBuffer(vertexBuffer);
    openglDeleteBuffer(colorBuffer);
    openglDeleteBuffer(indexBuffer);

    glDeleteProgram(shaders[Shader_Default].handle);
    glDeleteProgram(shaders[Shader_Depth].handle);
    glDeleteProgram(shaders[Shader_SSAO].handle);
    glDeleteProgram(shaders[Shader_Blur].handle);
    glDeleteProgram(shaders[Shader_FromFramebuffer].handle);
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
    currentLineWidth = 1.0f;

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

void drawBeginRenderToTexture(int n, bool clearDepth)
{
    drawFlush();

    GLint viewport[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    if (framebufferWidth != width || framebufferHeight != height) {
        framebufferWidth = width;
        framebufferHeight = height;

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    glBindTexture(GL_TEXTURE_2D, renderTargets[n]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[n], 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    glClear(GL_COLOR_BUFFER_BIT | (clearDepth ? GL_DEPTH_BUFFER_BIT : 0));
}

void drawEndRenderToTexture()
{
    drawFlush();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawSetShader(Shader shader)
{
    if (shader != currentShader) {
        drawFlush();
        currentShader = shader;
    }
}

void drawSetTexture(GLuint texture)
{
    if (texture != currentTexture) {
        drawFlush();
        currentTexture = texture;
    }
}

void drawSetLineWidth(float width)
{
    if (width != currentLineWidth) {
        drawFlush();
        currentLineWidth = width;
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

void drawMesh(const Mesh& mesh)
{
    drawSetTexture(0);
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

static void setupUniforms(const ShaderInfo* shader)
{
    glUseProgram(shader->handle);

    glLineWidth(currentLineWidth);

    int textureIndex = 0;
    for (size_t i = 0; i < MAX_RENDERTARGETS; i++) {
        if (shader->uniformAuxTexture[i] >= 0) {
            glActiveTexture(GL_TEXTURE0 + textureIndex);
            glBindTexture(GL_TEXTURE_2D, renderTargets[i]);
            glUniform1i(shader->uniformAuxTexture[i], textureIndex);
            ++textureIndex;
        }
    }

    if (shader->uniformTexture >= 0) {
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        glBindTexture(GL_TEXTURE_2D, currentTexture != 0 ? currentTexture : dummyTexture);
        glUniform1i(shader->uniformTexture, textureIndex);
        ++textureIndex;
    }

    if (shader->uniformRandomizerTexture >= 0) {
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        glBindTexture(GL_TEXTURE_2D, ssaoRandomizerTexture);
        glUniform1i(shader->uniformRandomizerTexture, textureIndex);
        ++textureIndex;
    }

    glActiveTexture(GL_TEXTURE0);

    if (shader->uniformViewportSize >= 0) {
        GLint viewport[4] = { 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, viewport);
        glUniform2f(shader->uniformViewportSize, float(viewport[2]), float(viewport[3]));
    }

    if (shader->uniformProjectionMatrix >= 0)
        glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void drawFlush()
{
    if (vertexCount > 0 || indexCount > 0) {
        const ShaderInfo* shader = &shaders[currentShader];
        setupUniforms(shader);

        if (shader->attrPosition >= 0 || shader->attrTexCoord >= 0) {
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER,
                vertexCount * sizeof(GLfloat) * VERTICES_PER_INDEX, vertices.data(), GL_STREAM_DRAW);

            if (shader->attrPosition >= 0) {
                glVertexAttribPointer(shader->attrPosition, 3, GL_FLOAT, GL_FALSE,
                    sizeof(GLfloat) * VERTICES_PER_INDEX, (void*)(sizeof(GLfloat) * 0));
                glEnableVertexAttribArray(shader->attrPosition);
            }

            if (shader->attrTexCoord >= 0) {
                glVertexAttribPointer(shader->attrTexCoord, 2, GL_FLOAT, GL_FALSE,
                    sizeof(GLfloat) * VERTICES_PER_INDEX, (void*)(sizeof(GLfloat) * 3));
                glEnableVertexAttribArray(shader->attrTexCoord);
            }
        }

        if (shader->attrColor >= 0) {
            glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(uint32_t), colors.data(), GL_STREAM_DRAW);
            glVertexAttribPointer(shader->attrColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
            glEnableVertexAttribArray(shader->attrColor);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices.data(), GL_STREAM_DRAW);
        glDrawElements(currentPrimitiveType, indexCount, GL_UNSIGNED_SHORT, NULL);

        if (shader->attrPosition >= 0)
            glDisableVertexAttribArray(shader->attrPosition);
        if (shader->attrTexCoord >= 0)
            glDisableVertexAttribArray(shader->attrTexCoord);
        if (shader->attrColor >= 0)
            glDisableVertexAttribArray(shader->attrColor);

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

static void drawFullscreenQuad(Shader shaderId)
{
    drawFlush();

    const ShaderInfo* shader = &shaders[shaderId];
    setupUniforms(shader);

    if (shader->attrPosition >= 0 || shader->attrTexCoord >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);

        if (shader->attrPosition >= 0) {
            glVertexAttribPointer(shader->attrPosition, 2, GL_FLOAT, GL_FALSE,
                sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 0));
            glEnableVertexAttribArray(shader->attrPosition);
        }

        if (shader->attrTexCoord >= 0) {
            glVertexAttribPointer(shader->attrTexCoord, 2, GL_FLOAT, GL_FALSE,
                sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 2));
            glEnableVertexAttribArray(shader->attrTexCoord);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (shader->attrPosition >= 0)
            glDisableVertexAttribArray(shader->attrPosition);
        if (shader->attrTexCoord >= 0)
            glDisableVertexAttribArray(shader->attrTexCoord);
    }
}

void drawSsao()
{
    drawFullscreenQuad(Shader_SSAO);
}

void drawBlur()
{
    drawFullscreenQuad(Shader_Blur);
}

void drawFromFramebuffer(int n)
{
    drawSetTexture(renderTargets[n]);
    drawFullscreenQuad(Shader_FromFramebuffer);
}
