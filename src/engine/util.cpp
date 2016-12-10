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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

void logPrint(const std::string& message)
{
    fprintf(stderr, "%s\n", message.c_str());
}

void fatalExit(const std::string& message)
{
    fprintf(stderr, "FATAL ERROR: %s\n", message.c_str());
    exit(1);
}

std::string loadFile(const std::string& name)
{
    logPrint(fmt() << "Loading \"" << name << "\"...");

    struct stat st;
    if (stat(name.c_str(), &st) < 0) {
        const char* errorMessage = strerror(errno);
        fatalExit(fmt() << "Unable to stat file \"" << name << "\": " << errorMessage);
    }

    size_t size = size_t(st.st_size);
    std::string result(size, 0);

    FILE* f = fopen(name.c_str(), "rb");
    if (!f) {
        const char* errorMessage = strerror(errno);
        fatalExit(fmt() << "Unable to open file \"" << name << "\": " << errorMessage);
    }

    size_t bytesRead = fread(&result[0], 1, size, f);
    if (ferror(f)) {
        const char* errorMessage = strerror(errno);
        fclose(f);
        fatalExit(fmt() << "Unable to read file \"" << name << "\": " << errorMessage);
    }
    if (bytesRead != size) {
        fclose(f);
        fatalExit(fmt() << "Incomplete read in file \"" << name << "\".");
    }

    fclose(f);

    return result;
}
