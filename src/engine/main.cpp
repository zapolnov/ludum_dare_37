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
#include "game.h"

#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

static double prevTime;
static GLFWwindow* window;

void runFrame()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    double time = glfwGetTime();
    double frameTime = time - prevTime;
    prevTime = time;

    gameRunFrame(frameTime, width, height);

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

    glfwMakeContextCurrent(window);

    gameInit();

    prevTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
        runFrame();

    gameShutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
