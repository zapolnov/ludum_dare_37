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
#include "gui.h"
#include "opengl.h"
#include "draw.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

static GLuint fontTexture;
static GLuint shader;
static GLuint vertexBuffer;
static GLuint indexBuffer;

static int attrPosition;
static int attrTexCoord;
static int attrColor;
static int uniformProjectionMatrix;
static int uniformTexture;

static void renderDrawLists(ImDrawData* drawData)
{
    ImGuiIO& io = ImGui::GetIO();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    GLint viewportSize[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_VIEWPORT, viewportSize);

    glActiveTexture(GL_TEXTURE0);
    glm::mat4 projectionMatrix = glm::ortho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, 1.0f);

    glUseProgram(shader);
    glUniform1i(uniformTexture, 0);
    glUniformMatrix4fv(uniformProjectionMatrix, 1, GL_FALSE, &projectionMatrix[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glVertexAttribPointer(attrPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, pos));
    glVertexAttribPointer(attrTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, uv));
    glVertexAttribPointer(attrColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, col));

    glEnableVertexAttribArray(attrPosition);
    glEnableVertexAttribArray(attrTexCoord);
    glEnableVertexAttribArray(attrColor);

    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        glBufferData(GL_ARRAY_BUFFER,
            cmdList->VtxBuffer.Size * sizeof(ImDrawVert),
            reinterpret_cast<void*>(cmdList->VtxBuffer.Data),
            GL_STREAM_DRAW);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            cmdList->IdxBuffer.Size * sizeof(ImDrawIdx),
            reinterpret_cast<void*>(cmdList->IdxBuffer.Data),
            GL_STREAM_DRAW);

        const ImDrawIdx* indexBufferOffset = 0;
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];
            if (pcmd->UserCallback)
                pcmd->UserCallback(cmdList, pcmd);
            else {
                glScissor(int(pcmd->ClipRect.x),
                    int(viewportSize[3] - pcmd->ClipRect.w),
                    int(pcmd->ClipRect.z - pcmd->ClipRect.x),
                    int(pcmd->ClipRect.w - pcmd->ClipRect.y));

                glBindTexture(GL_TEXTURE_2D, GLuint(ptrdiff_t(pcmd->TextureId)));
                glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, indexBufferOffset);
            }
            indexBufferOffset += pcmd->ElemCount;
        }
    }

    glDisableVertexAttribArray(attrPosition);
    glDisableVertexAttribArray(attrTexCoord);
    glDisableVertexAttribArray(attrColor);
}

void guiInit()
{
    vertexBuffer = openglCreateBuffer();
    indexBuffer = openglCreateBuffer();

    shader = openglLoadProgram("DrawV.glsl", "DrawF.glsl");
    attrPosition = glGetAttribLocation(shader, "aPosition");
    attrTexCoord = glGetAttribLocation(shader, "aTexCoord");
    attrColor = glGetAttribLocation(shader, "aColor");
    uniformProjectionMatrix = glGetUniformLocation(shader, "uProjectionMatrix");
    uniformTexture = glGetUniformLocation(shader, "uTexture");

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = renderDrawLists;
    io.SetClipboardTextFn = [](void*, const char*){};
    io.GetClipboardTextFn = [](void*){ return ""; };
    io.ClipboardUserData = nullptr;

    fontTexture = openglCreateTexture(NoRepeat, GL_LINEAR);
    unsigned char* pixels = NULL;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->TexID = (void*)ptrdiff_t(fontTexture);
}

void guiShutdown()
{
    openglDeleteTexture(fontTexture);
    openglDeleteBuffer(vertexBuffer);
    openglDeleteBuffer(indexBuffer);
    glDeleteProgram(shader);
    ImGui::GetIO().Fonts->TexID = 0;
    ImGui::Shutdown();
}

void guiBeginFrame(double frameTime, int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(width), float(height));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DeltaTime = frameTime;
    ImGui::NewFrame();
}

void guiEndFrame()
{
    ImGui::Render();
}

void guiSetMousePos(const glm::vec2& pos)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(pos.x, pos.y);
}

void guiSetMouseButtonPressed(int button, bool pressed)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[button] = pressed;
}

void guiSetMouseWheel(float wheel)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = wheel;
}

static void guiUpdateModifiers(ImGuiIO& io)
{
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void guiInjectKeyPress(int key)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = true;
    guiUpdateModifiers(io);
}

void guiInjectKeyRelease(int key)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = false;
    guiUpdateModifiers(io);
}

void guiInjectUnicode(unsigned short code)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(code);
}
