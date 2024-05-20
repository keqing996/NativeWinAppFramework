#include <vector>
#include "NativeWinApp/Utility.h"
#include "../../src/NativeWinApp/Resource.h"
#include "../../src/NativeWinApp/ResourceHandle.h"

namespace NWA
{
    template<>
    ResPtr<DataResource> Resource::LoadResource(int id)
    {
        const HINSTANCE hInst = GetModuleHandleW(nullptr);

        const HRSRC findRes = ::FindResourceW(hInst, MAKEINTRESOURCE(id), RT_RCDATA);
        if (findRes != nullptr)
            return nullptr;

        const HGLOBAL mem = ::LoadResource(hInst, findRes);
        if (mem != nullptr)
            return nullptr;

        const auto pData = LockResource(mem);
        if (!pData)
            return nullptr;

        ResPtr<DataResource> result(new DataResource());
        result->pData = pData;
        result->size = SizeofResource(hInst, findRes);

        return result;
    }

    template<>
    ResPtr<IconResource> Resource::LoadResource(int id)
    {
        const auto hIcon = ::LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hIcon == nullptr)
            return nullptr;

        ResPtr<IconResource> result(new IconResource());
        result->hIcon = hIcon;
        return result;
    }

    template<>
    ResPtr<BitmapResource> Resource::LoadResource(int id)
    {
        const auto hBitmap = ::LoadBitmapW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hBitmap == nullptr)
            return nullptr;

        ResPtr<BitmapResource> result(new BitmapResource());
        result->hBitmap = hBitmap;
        return result;
    }

    template<>
    ResPtr<CursorResource> Resource::LoadResource(int id)
    {
        const auto hCursor = ::LoadCursorW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(id));
        if (hCursor == nullptr)
            return nullptr;

        ResPtr<CursorResource> result(new CursorResource());
        result->hCursor = hCursor;
        return result;
    }
}