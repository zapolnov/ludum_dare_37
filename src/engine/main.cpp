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
#include "util.h"
#include "draw.h"
#include "game.h"
#include "mesh.h"
#include "gui.h"

#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

static float mouseWheel;
static bool mouseButtonPressed[3];
static double prevTime;
static GLFWwindow* window;

static void mouseButtonCallback(GLFWwindow*, int button, int action, int)
{
    if (action == GLFW_PRESS && button >= 0 && button < 3)
        mouseButtonPressed[button] = true;
}

static void mouseWheelCallback(GLFWwindow*, double, double yoffset)
{
    mouseWheel += float(yoffset);
}

static void keyCallback(GLFWwindow*, int key, int, int action, int)
{
    switch (action) {
        case GLFW_PRESS: guiInjectKeyPress(key); break;
        case GLFW_RELEASE: guiInjectKeyRelease(key); break;
    }
}

static void charCallback(GLFWwindow*, unsigned int c)
{
    if (c > 0 && c < 0x10000)
        guiInjectUnicode(static_cast<unsigned short>(c));
}

void runFrame()
{
    if (!glfwGetWindowAttrib(window, GLFW_FOCUSED))
        guiSetMousePos(glm::vec2(-1.0f));
    else {
        double mouseX = 0.0, mouseY = 0.0;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        guiSetMousePos(glm::vec2(float(mouseX), float(mouseY)));
    }

    for (int i = 0; i < 3; i++) {
        guiSetMouseButtonPressed(i, mouseButtonPressed[i] || glfwGetMouseButton(window, i));
        mouseButtonPressed[i] = false;
    }

    guiSetMouseWheel(mouseWheel);
    mouseWheel = 0.0f;

    double time = glfwGetTime();
    double frameTime = time - prevTime;
    prevTime = time;

    int winWidth = 0, winHeight = 0;
    glfwGetWindowSize(window, &winWidth, &winHeight);
    guiBeginFrame(frameTime, winWidth, winHeight);

    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    gameRunFrame(frameTime, winWidth, winHeight);

    guiEndFrame();
    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main()
{
    glfwSetErrorCallback([](int, const char* message){ logPrint(fmt() << "GLFW: " << message); });

    if (!glfwInit())
        fatalExit("GLFW initialization failed.");

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(1024, 768, "Ludum Dare", nullptr, nullptr);
    if (!window)
        fatalExit("Unable to create main window.");

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseWheelCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);

    glfwMakeContextCurrent(window);

    drawInit();
    guiInit();
    meshInitCache();
    gameInit();

    prevTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
        runFrame();

    gameShutdown();
    meshShutdownCache();
    guiShutdown();
    drawShutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
