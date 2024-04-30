#pragma once

#include "Infra/Windows/WindowsDefine.hpp"

namespace Infra
{
    struct DataResource
    {
        void* pData;
        DWORD size;
    };

    struct CursorResource
    {
        HCURSOR hCursor;
    };

    struct BitmapResource
    {
        HBITMAP hBitmap;
    };

    struct IconResource
    {
        HICON hIcon;
    };
}
