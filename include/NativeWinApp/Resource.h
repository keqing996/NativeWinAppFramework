#pragma once

#include <cstdint>

namespace NWA
{
    struct DataResource
    {
        void* pData;
        uint32_t size;
    };

    struct CursorResource
    {
        void* hCursor;
    };

    struct BitmapResource
    {
        void* hBitmap;
    };

    struct IconResource
    {
        void* hIcon;
    };

    class Resource
    {
    public:
        Resource() = delete;

    public:
        template<typename T>
        T LoadResource(int id);
    };
}
