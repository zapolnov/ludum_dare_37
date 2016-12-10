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
#include "mesheditor.h"
#include "game.h"
#include "menu/mainmenu.h"
#include "engine/draw.h"
#include "engine/gui.h"
#include "engine/util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

MeshEditor::MeshEditor(const std::string& file)
    : mFile(file)
    , mSelectedObject(0)
    , mCameraDistance(10.0f)
    , mCameraHorzRotation(0.0f)
    , mCameraVertRotation(0.0f)
{
    if (fileExists(mFile)) {
        mMesh.load(mFile);
        mCameraDistance = std::max(glm::length(mMesh.bboxSize), 2.0f);
    }
}

MeshEditor::~MeshEditor()
{
}

void MeshEditor::run(double time, int width, int height)
{
    glm::vec3 cameraOffset = glm::vec3(
        mCameraDistance * sinf(glm::radians(mCameraHorzRotation)) * cosf(glm::radians(mCameraVertRotation)),
        mCameraDistance * sinf(glm::radians(mCameraVertRotation)),
        mCameraDistance * cosf(glm::radians(mCameraHorzRotation)) * cosf(glm::radians(mCameraVertRotation)));

    glm::vec3 cameraTarget = glm::vec3(0.0f);
    glm::vec3 cameraPosition = cameraTarget + cameraOffset;

    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));

    auto m = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    drawPushMatrix(m);

    drawBeginPrimitive(GL_LINES);
        drawPushColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(10.0f, 0.0f, 0.0f));
        drawPushColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(0.0f, 10.0f, 0.0f));
        drawPushColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 10.0f));
    drawEndPrimitive();

    drawMesh(glm::vec3(0.0f), mMesh);

    drawEnd();

    bool windowVisible = true;
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_FirstUseEver);
    if (!ImGui::Begin("Mesh Editor", &windowVisible, 0)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Save"))
        mMesh.save(mFile);

    ImGui::BeginGroup();
    ImGui::PushID("Camera");
    ImGui::PushMultiItemsWidths(3);
    ImGui::DragFloat("DIST", &mCameraDistance, 1.0f, 2.0f, std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("HORZ", &mCameraHorzRotation, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("VERT", &mCameraVertRotation, 1.0f, -179.0f, 179.0f);
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::EndGroup();

    while (mCameraHorzRotation < 0.0f)
        mCameraHorzRotation += 360.0f;
    while (mCameraHorzRotation >= 360.0f)
        mCameraHorzRotation -= 360.0f;

    static std::string buffer;
    auto listboxGetter = [](void* data, int n, const char** p) -> bool {
            auto object = reinterpret_cast<MeshEditor*>(data)->mMesh.objects[size_t(n)].get();
            buffer = fmt() << '[' << object->typeString() << "] " << n;
            *p = buffer.c_str();
            return true;
        };
    ImGui::ListBox("Objects", &mSelectedObject, listboxGetter, this, int(mMesh.objects.size()), 16);

    bool bake = false;

    if (ImGui::Button("New Cube")) {
        auto object = std::unique_ptr<Mesh::Cube>(new Mesh::Cube);
        object->p1 = glm::vec3(-1.0f);
        object->p2 = glm::vec3(1.0f);
        mSelectedObject = int(mMesh.objects.size());
        mMesh.objects.emplace_back(std::move(object));
        bake = true;
    }

    if (mSelectedObject >= 0 && mSelectedObject < int(mMesh.objects.size())) {
        auto object = mMesh.objects[mSelectedObject].get();

        if (ImGui::Button("Clone Object")) {
            auto newObject = std::unique_ptr<Mesh::Object>(object->clone());
            mSelectedObject = int(mMesh.objects.size());
            mMesh.objects.emplace_back(std::move(newObject));
            bake = true;
        }

        if (ImGui::DragFloat3("Pos", &object->position[0], 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max()))
            bake = true;

        if (ImGui::DragFloat3("Rot", &object->rotation[0], 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
            for (size_t i = 0; i < 3; i++) {
                while (object->rotation[i] < 0.0f)
                    object->rotation[i] += 360.0f;
                while (object->rotation[i] >= 360.0f)
                    object->rotation[i] -= 360.0f;
            }
            bake = true;
        }

        if (ImGui::DragFloat3("Scl", &object->scale[0], 0.1f, 0.1f, std::numeric_limits<float>::max()))
            bake = true;

        if (ImGui::ColorEdit3("Color", &object->color[0]))
            bake = true;
    }

    ImGui::End();

    if (bake)
        mMesh.bake();

    if (!windowVisible) {
        gameSetScreen(mainMenu);
        delete this;
    }
}
