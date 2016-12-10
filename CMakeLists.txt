#
# Copyright (c) 2016 Nikolay Zapolnov (zapolnov@gmail.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

include_directories(BEFORE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

include_directories(BEFORE SYSTEM
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
    )

add_executable(LDGame
    lib/imgui/imconfig.h
    lib/imgui/imgui.cpp
    lib/imgui/imgui.h
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_internal.h
    src/engine/draw.cpp
    src/engine/draw.h
    src/engine/gui.cpp
    src/engine/gui.h
    src/engine/main.cpp
    src/engine/opengl.cpp
    src/engine/opengl.h
    src/engine/util.cpp
    src/engine/util.h
    src/menu/gamescreen.cpp
    src/menu/gamescreen.h
    src/menu/mainmenu.cpp
    src/menu/mainmenu.h
    src/game.cpp
    src/game.h
    src/level.cpp
    src/level.h
    )

find_library(GLFW3 glfw)
find_library(GLES2 GLESv2)
target_link_libraries(LDGame ${GLFW3} ${GLES2})
