#pragma once

#include <string>
#include <optional>
#include <vector>

namespace NWA
{
    class FileDialog
    {
    public:
        FileDialog() = delete;

    public:
        struct FileTypeFilter
        {
            std::string name;
            std::string suffix;
        };

    public:
        static std::optional<std::string> OpenFile(const std::string& titleMsg,
                                        const std::vector<FileTypeFilter>* pFilter = nullptr);

        static std::optional<std::string> SaveFile(const std::string& titleMsg,
                                            const std::string& defaultName,
                                            const std::vector<FileTypeFilter>* pFilter = nullptr);

        static std::optional<std::string> OpenDirectory(const std::string& titleMsg);
    };
}
