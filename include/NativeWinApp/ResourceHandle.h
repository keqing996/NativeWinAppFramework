#pragma once

#include "../../PlatformDefine.hpp"

#if PLATFORM_WINDOWS

#include "../../Windows/WindowsDefine.h"

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

#endif