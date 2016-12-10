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
#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include "menu/gamescreen.h"
#include "engine/mesh.h"
#include "level.h"

class LevelEditor : public GameScreen
{
public:
    explicit LevelEditor(const std::string& file = std::string());
    ~LevelEditor();

    void run(double time, int width, int height) override;

private:
    std::string mFile;
    Level mLevel;
    int mSelectedSector;
    int mSelectedPoint;
    int mSelectedMesh;
    float mCameraDistance;
    float mCameraHorzRotation;
    float mCameraVertRotation;
    glm::vec3 mCameraPosition;
    bool mCullFace;
    char mMeshFile[1024];
};

#endif
