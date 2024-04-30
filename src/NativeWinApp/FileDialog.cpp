#include <shobjidl.h>
#include "Infra/String.hpp"
#include "Infra/ScopeGuard.hpp"
#include "NativeWinApp/FileDialog.h"

namespace Infra
{
    struct FileTypeFilterW
    {
        std::wstring name;
        std::wstring suffix;
    };

    FileTypeFilterW GetWideStringVersion(const FileDialog::FileTypeFilter& fileFilter)
    {
        return FileTypeFilterW {
            String::StringToWideString(fileFilter.name),
            String::StringToWideString(fileFilter.suffix)
        };
    }

    std::optional<std::string> ShowShellDialogAndGetResult(IFileDialog* pFileDialog)
    {
        if (FAILED(pFileDialog->Show(nullptr)))
            return std::nullopt;

        IShellItem* pShellItem;
        if (FAILED(pFileDialog->GetResult(&pShellItem)))
            return std::nullopt;

        ScopeGuard guardShellItem = [&]{ pShellItem->Release(); };
        {
            PWSTR filePath;
            if (FAILED(pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath)))
                return std::nullopt;

            ScopeGuard guardFilePath = [&] { CoTaskMemFree(filePath); };
            {
                return String::WideStringToString(filePath);
            }
        }
    }

    std::optional<std::string> FileDialog::OpenFile(const std::string& titleMsg, const std::vector<FileTypeFilter>* pFilter)
    {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
            return std::nullopt;

        ScopeGuard guardInit = [] { CoUninitialize(); };
        {
            IFileDialog* pFileDialog;
            if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileDialog))))
                return std::nullopt;

            ScopeGuard guardFileDialogRaw = [&] { pFileDialog->Release(); };
            {
                // title
                const auto titleMsgW = String::StringToWideString(titleMsg);
                if (FAILED(pFileDialog->SetTitle(titleMsgW.c_str())))
                    return std::nullopt;

                // filter
                if (pFilter != nullptr)
                {
                    std::vector<FileTypeFilterW> filterVec;
                    for (const auto& filter : *pFilter)
                        filterVec.push_back(GetWideStringVersion(filter));

                    std::vector<COMDLG_FILTERSPEC> filterSpecVec;
                    for (const auto& [filterName, filterSuffic] : filterVec)
                    {
                        COMDLG_FILTERSPEC spec;
                        spec.pszName = filterName.c_str();
                        spec.pszSpec = filterSuffic.c_str();
                        filterSpecVec.push_back(spec);
                    }

                    if (FAILED(pFileDialog->SetFileTypes(filterSpecVec.size() * sizeof(COMDLG_FILTERSPEC), filterSpecVec.data())))
                        return std::nullopt;
                }

                // show
                return ShowShellDialogAndGetResult(pFileDialog);
            }
        }
    }

    std::optional<std::string> FileDialog::SaveFile(const std::string& titleMsg, const std::string& defaultName, const std::vector<FileTypeFilter>* pFilter)
    {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
            return std::nullopt;

        ScopeGuard guardInit = [] { CoUninitialize(); };
        {
            IFileDialog* pFileDialog;
            if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileDialog))))
                return std::nullopt;

            ScopeGuard guardFileDialogRaw = [&] { pFileDialog->Release(); };
            {
                // title
                const auto titleMsgW = String::StringToWideString(titleMsg);
                if (FAILED(pFileDialog->SetTitle(titleMsgW.c_str())))
                    return std::nullopt;

                // filter
                if (pFilter != nullptr)
                {
                    std::vector<FileTypeFilterW> filterVec;
                    for (const auto& filter : *pFilter)
                        filterVec.push_back(GetWideStringVersion(filter));

                    std::vector<COMDLG_FILTERSPEC> filterSpecVec;
                    for (const auto& [filterName, filterSuffic] : filterVec)
                    {
                        COMDLG_FILTERSPEC spec;
                        spec.pszName = filterName.c_str();
                        spec.pszSpec = filterSuffic.c_str();
                        filterSpecVec.push_back(spec);
                    }

                    if (FAILED(pFileDialog->SetFileTypes(filterSpecVec.size() * sizeof(COMDLG_FILTERSPEC), filterSpecVec.data())))
                        return std::nullopt;
                }

                // save name
                const auto defaultNameW = String::StringToWideString(defaultName);
                if (FAILED(pFileDialog->SetFileName(defaultNameW.c_str())))
                    return std::nullopt;

                // show
                return ShowShellDialogAndGetResult(pFileDialog);
            }
        }
    }

    std::optional<std::string> FileDialog::OpenDirectory(const std::string& titleMsg)
    {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
            return std::nullopt;

        ScopeGuard guardInit = [] { CoUninitialize(); };
        {
            IFileDialog* pFileDialog;
            if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileDialog))))
                return std::nullopt;

            ScopeGuard guardFileDialogRaw = [&] { pFileDialog->Release(); };
            {
                // title
                const auto titleMsgW = String::StringToWideString(titleMsg);
                if (FAILED(pFileDialog->SetTitle(titleMsgW.c_str())))
                    return std::nullopt;

                // set directory
                DWORD options;
                if (FAILED(pFileDialog->GetOptions(&options)))
                    return std::nullopt;

                if (FAILED(pFileDialog->SetOptions(options | FOS_PICKFOLDERS)))
                    return std::nullopt;

                // show
                return ShowShellDialogAndGetResult(pFileDialog);
            }
        }
    }

}