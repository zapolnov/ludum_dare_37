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
#include "menu/gamescreen.h"
#include "menu/mainmenu.h"

static GameScreen* currentScreen;

void gameInit()
{
    Level::loadResources();
    mainMenu = new MainMenu;
    gameSetScreen(mainMenu);
}

void gameShutdown()
{
    delete mainMenu;
    Level::unloadResources();
}

GameScreen* gameScreen()
{
    return currentScreen;
}

void gameSetScreen(GameScreen* screen)
{
    currentScreen = screen;
}

void gameRunFrame(double frameTime, int width, int height)
{
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentScreen)
        currentScreen->run(frameTime, width, height);
}
