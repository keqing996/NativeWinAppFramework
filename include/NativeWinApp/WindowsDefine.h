#pragma once

#include "../PlatformDefine.hpp"

#if PLATFORM_WINDOWS

// disable min max cacro in Windows.h
#ifndef NOMINMAX
#   define NOMINMAX
#endif

// Excludes some less commonly used APIs and reduces the compilation
// time and the size of the resulting executable
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN 1
#endif

// Use xxxW functions in Windows
#ifndef UNICODE
#   define UNICODE
#endif

// Use xxxW functions in Windows
#ifndef _UNICODE
#   define _UNICODE
#endif

#include <Windows.h>

#endif