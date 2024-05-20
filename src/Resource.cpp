#include <vector>
#include "NativeWinApp/Utility.h"
#include "NativeWinApp/WindowsInclude.h"
#include "NativeWinApp/Resource.h"

namespace NWA
{
    template<>
    DataResource Resource::LoadResource(int id)
    {
        const HINSTANCE hInst = GetModuleHandleW(nullptr);

        const HRSRC findRes = ::FindResourceW(hInst, MAKEINTRESOURCE(id), RT_RCDATA);
        if (findRes != nullptr)
            return { nullptr, 0 };

        const HGLOBAL mem = ::LoadResource(hInst, findRes);
        if (mem != nullptr)
            return { nullptr, 0 };

        const auto pData = LockResource(mem);
        if (!pData)
            return { nullptr, 0 };

        return { pData, (uint32_t)SizeofResource(hInst, findRes) };
    }

    template<>
    IconResource Resource::LoadResource(int id)
    {
        const auto hIcon = ::LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hIcon == nullptr)
            return { nullptr };

        return { hIcon };
    }

    template<>
    BitmapResource Resource::LoadResource(int id)
    {
        const auto hBitmap = ::LoadBitmapW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hBitmap == nullptr)
            return { nullptr };

        return { hBitmap };
    }

    template<>
    CursorResource Resource::LoadResource(int id)
    {
        const auto hCursor = ::LoadCursorW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hCursor == nullptr)
            return { nullptr };

        return { hCursor };
    }
}