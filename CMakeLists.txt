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

if(EMSCRIPTEN)
    add_definitions(-DPLATFORM_EMSCRIPTEN)
    set(template "${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file \"${template}\" --separate-asm")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/data@data\"")
    set(flags "-s USE_SDL=2 -s NO_EXIT_RUNTIME=1 -s ALIASING_FUNCTION_POINTERS=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1")
    set(flags "${flags} -s TOTAL_MEMORY=128000000")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags} -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3")
endif()

add_executable(LDGame
    lib/imgui/imconfig.h
    lib/imgui/imgui.cpp
    lib/imgui/imgui.h
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_internal.h
    src/editor/leveleditor.cpp
    src/editor/leveleditor.h
    src/editor/mesheditor.cpp
    src/editor/mesheditor.h
    src/engine/draw.cpp
    src/engine/draw.h
    src/engine/gui.cpp
    src/engine/gui.h
    src/engine/main.cpp
    src/engine/mesh.cpp
    src/engine/mesh.h
    src/engine/opengl.cpp
    src/engine/opengl.h
    src/engine/sprite.cpp
    src/engine/sprite.h
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

if(NOT EMSCRIPTEN)
    find_library(GLFW3 glfw)
    find_library(GLES2 GLESv2)
    target_link_libraries(LDGame ${GLFW3} ${GLES2})
endif()
