/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "cascfile.h"
#include <CascLib.h>
#include <cstdio>

CASCFile::CASCFile(CASC::StorageHandle const& casc, const char* filename, bool warnNoExist /*= true*/) :
    eof(false),
    buffer(0),
    pointer(0),
    size(0)
{
    CASC::FileHandle file = CASC::OpenFile(casc, filename, CASC_LOCALE_ALL, false);
    if (!file)
    {
        if (warnNoExist || GetLastError() != ERROR_FILE_NOT_FOUND)
            fprintf(stderr, "Can't open %s: %s\n", filename, CASC::HumanReadableCASCError(GetLastError()));
        eof = true;
        return;
    }

    DWORD fileSizeHigh = 0;
    DWORD fileSize = CASC::GetFileSize(file, &fileSizeHigh);
    if (fileSize == CASC_INVALID_SIZE)
    {
        fprintf(stderr, "Can't open %s, failed to get size: %s!\n", filename, CASC::HumanReadableCASCError(GetLastError()));
        eof = true;
        return;
    }

    if (fileSizeHigh)
    {
        fprintf(stderr, "Can't open %s, file larger than 2GB", filename);
        eof = true;
        return;
    }

    size = fileSize;

    DWORD read = 0;
    buffer = new char[size];
    if (!CASC::ReadFile(file, buffer, size, &read) || size != read)
    {
        fprintf(stderr, "Can't read %s, size=%u read=%u: %s\n", filename, uint32(size), uint32(read), CASC::HumanReadableCASCError(GetLastError()));
        eof = true;
        return;
    }
}

size_t CASCFile::read(void* dest, size_t bytes)
{
    if (eof)
        return 0;

    size_t rpos = pointer + bytes;
    if (rpos > size)
    {
        bytes = size - pointer;
        eof = true;
    }

    memcpy(dest, &(buffer[pointer]), bytes);

    pointer = rpos;

    return bytes;
}

void CASCFile::seek(int offset)
{
    pointer = offset;
    eof = (pointer >= size);
}

void CASCFile::seekRelative(int offset)
{
    pointer += offset;
    eof = (pointer >= size);
}

void CASCFile::close()
{
    delete[] buffer;
    buffer = 0;
    eof = true;
}
