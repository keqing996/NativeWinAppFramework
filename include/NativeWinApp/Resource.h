#pragma once

#include "Infra/ResourcePtr.hpp"

namespace NWA
{
    struct DataResource;
    struct CursorResource;
    struct BitmapResource;
    struct IconResource;

    class Resource
    {
    public:
        Resource() = delete;

    public:
        template<typename T>
        ResPtr<T> LoadResource(int id);
    };
}
