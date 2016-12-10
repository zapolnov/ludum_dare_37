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
#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>
#include <string>
#include <sstream>

class fmt
{
public:
    fmt() = default;
    template <typename T> fmt& operator<<(T&& value) { mStream << std::forward<T>(value); return *this; }
    operator std::string() const { return mStream.str(); }

private:
    std::stringstream mStream;
};

void logPrint(const std::string& message);
void fatalExit(const std::string& message);

bool fileExists(const std::string& name);
std::string loadFile(const std::string& name);
void saveFile(const std::string& name, const std::string& data);

uint32_t toUInt32(const glm::vec4& c);

#endif
