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
#include "leveleditor.h"
#include "game.h"
#include "menu/mainmenu.h"
#include "engine/draw.h"
#include "engine/gui.h"
#include "engine/util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

LevelEditor::LevelEditor(const std::string& file)
    : mFile(file)
    , mSelectedSector(0)
    , mSelectedPoint(0)
    , mSelectedMesh(-1)
    , mCameraDistance(200.0f)
    , mCameraHorzRotation(0.0f)
    , mCameraVertRotation(45.0f)
    , mCameraPosition(0.0f)
    , mCullFace(false)
{
    strcpy(mMeshFile, "");
    if (fileExists(mFile))
        mLevel.load(mFile);
    else {
        auto sector = std::make_shared<Level::Sector>();
        sector->points.emplace_back(std::make_shared<Level::Point>(glm::vec2(-100.0f, -100.0f), 0.0f, 60.0f));
        sector->points.emplace_back(std::make_shared<Level::Point>(glm::vec2( 100.0f, -100.0f), 0.0f, 60.0f));
        sector->points.emplace_back(std::make_shared<Level::Point>(glm::vec2( 100.0f,  100.0f), 0.0f, 60.0f));
        sector->points.emplace_back(std::make_shared<Level::Point>(glm::vec2(-100.0f,  100.0f), 0.0f, 60.0f));
        mSelectedSector = int(mLevel.sectors.size());
        mLevel.sectors.emplace_back(std::move(sector));
    }
}

LevelEditor::~LevelEditor()
{
}

void LevelEditor::run(double time, int width, int height)
{
    glm::vec3 cameraOffset = glm::vec3(
        mCameraDistance * sinf(glm::radians(mCameraHorzRotation)) * cosf(glm::radians(mCameraVertRotation)),
        mCameraDistance * sinf(glm::radians(mCameraVertRotation)),
        mCameraDistance * cosf(glm::radians(mCameraHorzRotation)) * cosf(glm::radians(mCameraVertRotation)));

    glm::vec3 cameraTarget = glm::vec3(mCameraPosition.x, mCameraPosition.y, -mCameraPosition.z);
    glm::vec3 cameraPosition = cameraTarget + cameraOffset;

    if (mCullFace)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    drawBegin(glm::perspective(glm::radians(90.0f), float(width) / float(height), 1.0f, 1000.0f));

    auto m = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    drawPushMatrix(m);

    drawBeginPrimitive(GL_LINES);
        drawPushColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(10.0f, 0.0f, 0.0f));
        drawSetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(0.0f, 10.0f, 0.0f));
        drawSetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 0.0f));
        drawVertex3D(glm::vec3(0.0f, 0.0f, 10.0f));
        drawPopColor();
    drawEndPrimitive();

    mLevel.draw3D();

    if (mSelectedSector >= 0 && mSelectedSector < int(mLevel.sectors.size())) {
        const auto& sector = mLevel.sectors[size_t(mSelectedSector)];

        drawSetTexture(0);
        drawPushColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        drawSetLineWidth(5.0f);

        drawBeginPrimitive(GL_LINES);
        for (size_t i = 0; i < sector->points.size(); i++) {
            const auto& point = sector->points[size_t(i)];
            const auto& nextPoint = sector->points[size_t(i + 1) % sector->points.size()];

            if (int(i) == mSelectedPoint)
                drawPushColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

            auto p1 = glm::vec3(point->pos.x, point->pos.y, point->maxZ);
            auto p2 = glm::vec3(nextPoint->pos.x, nextPoint->pos.y, nextPoint->maxZ);
            drawVertex3D(p1);
            drawVertex3D(p2);

            if (int(i) == mSelectedPoint)
                drawPopColor();
        }
        drawEndPrimitive();

        drawSetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        if (mSelectedPoint >= 0 && mSelectedPoint < int(sector->points.size())) {
            const auto& point = sector->points[size_t(mSelectedPoint)];
            const auto& nextPoint = sector->points[size_t(mSelectedPoint + 1) % sector->points.size()];

            drawBeginPrimitive(GL_TRIANGLES);
            auto p1 = glm::vec3(point->pos.x - 1.0f, point->pos.y - 1.0f, point->minZ);
            auto p2 = glm::vec3(point->pos.x + 1.0f, point->pos.y + 1.0f, point->maxZ + 4.0f);
            for (const auto& v : cubeVertices) {
                glm::vec3 p;
                p.x = (v.x < 0.0f ? p1.x : p2.x);
                p.y = (v.y < 0.0f ? p1.y : p2.y);
                p.z = (v.z < 0.0f ? p1.z : p2.z);
                drawVertex3D(p);
            }
            drawEndPrimitive();
        }

        drawSetLineWidth(1.0f);
        drawPopColor();
    }

    drawEnd();

    bool windowVisible = true;
    ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiSetCond_FirstUseEver);
    if (!ImGui::Begin("Level Editor", &windowVisible, 0)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Save"))
        mLevel.save(mFile);

    ImGui::Checkbox("Cull Faces", &mCullFace);

    ImGui::BeginGroup();
    ImGui::PushID("Camera");

    ImGui::PushMultiItemsWidths(3);
    ImGui::DragFloat("X", &mCameraPosition.x, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("Y", &mCameraPosition.z, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("Z", &mCameraPosition.y, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    ImGui::PopItemWidth();

    ImGui::PushMultiItemsWidths(3);
    ImGui::DragFloat("DIST", &mCameraDistance, 1.0f, 2.0f, std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("HORZ", &mCameraHorzRotation, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PopItemWidth();
    ImGui::DragFloat("VERT", &mCameraVertRotation, 1.0f, -89.0f, 89.0f);
    ImGui::PopItemWidth();

    ImGui::PopID();
    ImGui::EndGroup();

    while (mCameraHorzRotation < 0.0f)
        mCameraHorzRotation += 360.0f;
    while (mCameraHorzRotation >= 360.0f)
        mCameraHorzRotation -= 360.0f;

    static std::string buffer;
    auto listboxGetter = [](void* data, int n, const char** p) -> bool {
            //const auto& sector = reinterpret_cast<LevelEditor*>(data)->mLevel.sectors[size_t(n)];
            buffer = fmt() << n;
            *p = buffer.c_str();
            return true;
        };
    if (ImGui::ListBox("Sectors", &mSelectedSector, listboxGetter, this, int(mLevel.sectors.size()), 5)) {
        mSelectedPoint = 0;
        mSelectedMesh = -1;
    }

    if (mSelectedSector >= 0 && mSelectedSector < int(mLevel.sectors.size())) {
        const auto& sector = mLevel.sectors[mSelectedSector];

        ImGui::Button("Drag Sector");
        if (ImGui::IsItemActive()) {
            ImVec2 value = ImGui::GetIO().MouseDelta;
            for (const auto& pt : sector->points) {
                auto adjacentPoint = pt->adjacentPoint.lock();
                if (adjacentPoint) {
                    adjacentPoint->pos.x += value.x;
                    adjacentPoint->pos.y += value.y;
                }
                pt->pos.x += value.x;
                pt->pos.y += value.y;
            }
        }

        ImGui::Button("Raise/Lower Sector Floor");
        if (ImGui::IsItemActive()) {
            ImVec2 value = ImGui::GetIO().MouseDelta;
            for (const auto& pt : sector->points) {
                auto adjacentPoint = pt->adjacentPoint.lock();
                if (adjacentPoint)
                    adjacentPoint->minZ += value.y;
                pt->minZ += value.y;
            }
        }

        ImGui::Button("Raise/Lower Sector Ceiling");
        if (ImGui::IsItemActive()) {
            ImVec2 value = ImGui::GetIO().MouseDelta;
            for (const auto& pt : sector->points) {
                auto adjacentPoint = pt->adjacentPoint.lock();
                if (adjacentPoint)
                    adjacentPoint->maxZ += value.y;
                pt->maxZ += value.y;
            }
        }

        static std::string buffer;
        auto listboxGetter = [](void* data, int n, const char** p) -> bool {
                LevelEditor* self = reinterpret_cast<LevelEditor*>(data);
                const auto& sector = self->mLevel.sectors[self->mSelectedSector];
                const auto& point = sector->points[size_t(n)];
                std::stringstream ss;
                ss << n << " (" << point->pos.x << "; " << point->pos.y << ')';
                auto adjacentSector = point->adjacentSector.lock();
                if (adjacentSector) {
                    for (size_t i = 0; i < self->mLevel.sectors.size(); i++) {
                        if (self->mLevel.sectors[i] == adjacentSector) {
                            ss << " => " << i;
                            break;
                        }
                    }
                }
                buffer = ss.str();
                *p = buffer.c_str();
                return true;
            };
        ImGui::ListBox("Points", &mSelectedPoint, listboxGetter, this, int(sector->points.size()), 5);

        if (mSelectedPoint >= 0 && mSelectedPoint < int(sector->points.size())) {
            size_t nextSelectedPoint = size_t(mSelectedPoint + 1) % sector->points.size();
            auto point = sector->points[size_t(mSelectedPoint)];
            auto nextPoint = sector->points[nextSelectedPoint];

            if (ImGui::Button("New Point")) {
                auto newPoint = std::make_shared<Level::Point>();
                newPoint->pos = point->pos + (nextPoint->pos - point->pos) * 0.5f;
                newPoint->minZ = point->minZ + (nextPoint->minZ - point->minZ) * 0.5f;
                newPoint->maxZ = point->maxZ + (nextPoint->maxZ - point->maxZ) * 0.5f;
                ++mSelectedPoint;
                sector->points.emplace(sector->points.begin() + mSelectedPoint, std::move(newPoint));
            }

            auto adjacentSector = point->adjacentSector.lock();
            auto adjacentPoint = point->adjacentPoint.lock();
            auto nextAdjacentSector = nextPoint->adjacentSector.lock();
            auto nextAdjacentPoint = nextPoint->adjacentPoint.lock();
            if (!adjacentSector && !adjacentPoint && !nextAdjacentSector && !nextAdjacentPoint && ImGui::Button("New Adjacent Sector")) {
                auto d = nextPoint->pos - point->pos;
                auto normal = glm::vec2(d.y, -d.x);

                auto newSector = std::make_shared<Level::Sector>();
                auto anp = std::make_shared<Level::Point>(nextPoint->pos, nextPoint->minZ, nextPoint->maxZ, sector, nextPoint);
                newSector->points.emplace_back(anp);
                auto ap = std::make_shared<Level::Point>(point->pos, point->minZ, point->maxZ, nullptr, point);
                newSector->points.emplace_back(ap);
                newSector->points.emplace_back(std::make_shared<Level::Point>(point->pos + normal, point->minZ, point->maxZ));
                newSector->points.emplace_back(std::make_shared<Level::Point>(nextPoint->pos + normal, nextPoint->minZ, nextPoint->maxZ));
                mSelectedSector = int(mLevel.sectors.size());
                mSelectedPoint = 0;
                mLevel.sectors.emplace_back(newSector);

                point->adjacentSector = newSector;
                point->adjacentPoint = ap;
                nextPoint->adjacentPoint = anp;
            }

            if (ImGui::DragFloat2("Pos", &point->pos[0], 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
                if (adjacentPoint)
                    adjacentPoint->pos = point->pos;
            }

            if (ImGui::DragFloat("MinZ", &point->minZ, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
                if (adjacentPoint)
                    adjacentPoint->minZ = point->minZ;
            }

            if (ImGui::DragFloat("MaxZ", &point->maxZ, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
                if (adjacentPoint)
                    adjacentPoint->maxZ = point->maxZ;
            }

            if (sector->points.size() > 3) {
                if (!adjacentPoint && !adjacentSector && ImGui::Button("Delete point"))
                    sector->points.erase(sector->points.begin() + mSelectedPoint);
            }
        }

        if (mLevel.sectors.size() > 1) {
            if (ImGui::Button("Delete sector"))
                mLevel.sectors.erase(mLevel.sectors.begin() + mSelectedSector);
        }
    }

    auto listboxGetter2 = [](void* data, int n, const char** p) -> bool {
            const auto& mesh = reinterpret_cast<LevelEditor*>(data)->mLevel.meshes[size_t(n)];
            buffer = fmt() << mesh->meshName << ' ' << n;
            *p = buffer.c_str();
            return true;
        };
    if (ImGui::ListBox("Meshes", &mSelectedMesh, listboxGetter2, this, int(mLevel.meshes.size()), 10)) {
        mSelectedSector = -1;
        mSelectedPoint = -1;
    }

    ImGui::InputText("New Mesh", mMeshFile, sizeof(mMeshFile));
    ImGui::SameLine();
    if (ImGui::Button("Create")) {
        auto staticMesh = std::make_shared<Level::StaticMesh>();
        staticMesh->meshName = mMeshFile;
        staticMesh->loadMesh();
        staticMesh->calcMatrix();
        mLevel.meshes.emplace_back(std::move(staticMesh));
    }

    if (mSelectedMesh >= 0 && mSelectedMesh < int(mLevel.meshes.size())) {
        const auto& mesh = mLevel.meshes[mSelectedMesh];
        bool recalcMatrix = false;

        if (ImGui::DragFloat3("Pos", &mesh->pos[0], 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max()))
            recalcMatrix = true;

        if (ImGui::DragFloat3("Rot", &mesh->rot[0], 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
            for (size_t i = 0; i < 3; i++) {
                while (mesh->rot[i] < 0.0f)
                    mesh->rot[i] += 360.0f;
                while (mesh->rot[i] >= 360.0f)
                    mesh->rot[i] -= 360.0f;
            }
            recalcMatrix = true;
        }

        if (ImGui::DragFloat3("Scl", &mesh->scale[0], 0.1f, 0.1f, std::numeric_limits<float>::max()))
            recalcMatrix = true;

        if (recalcMatrix)
            mesh->calcMatrix();
    }

    ImGui::End();

    if (!windowVisible) {
        gameSetScreen(mainMenu);
        delete this;
    }
}
