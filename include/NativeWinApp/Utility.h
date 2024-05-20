#pragma once

#include <functional>
#include <string>

namespace NWA
{
    class Utility
    {
    public:
        Utility() = delete;

    public:
        static std::string WideStringToString(const std::wstring& wStr);
        static std::wstring StringToWideString(const std::string& str);
    };

    class NonCopyable
    {
    public:
        NonCopyable() = default;
        ~NonCopyable() = default;

    public:
        NonCopyable( const NonCopyable& ) = delete;
        const NonCopyable& operator=( const NonCopyable& ) = delete;
    };

    class ScopeGuard : NonCopyable
    {
    public:
        ScopeGuard(ScopeGuard&& other) = delete;

        ~ScopeGuard()
        {
            _cleanup();
        }

        template<class Func>
        ScopeGuard(const Func& cleanup) // NOLINT(*-explicit-constructor)
            : _cleanup(cleanup)
        {
        }

    private:
        std::function<void()> _cleanup;
    };
}