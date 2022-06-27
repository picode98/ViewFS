//
// Created by sdk on 5/28/2022.
//

#ifndef VIEWFS_PYTHONSUBPROCESSSOURCE_H
#define VIEWFS_PYTHONSUBPROCESSSOURCE_H


#include <filesystem>

#include "PlatformUtils.h"
#include "IFileSource.h"

class PythonSubprocessSource : public IFileSource {
    NamedPipeServer commandPipe;
    Subprocess pythonProc;

public:
    PythonSubprocessSource(const std::filesystem::path& virtRoot, const std::filesystem::path& pathToPythonExecutable,
                           const std::filesystem::path& routerFile)
        : commandPipe("viewfs-python-" + GUIDToString(newGUID())),
          pythonProc(pathToPythonExecutable, { "..\\ViewFS_python_host.py", "--fs-root=" + virtRoot.string(),
                                                    "--command-pipe=" + commandPipe.getFilename(),
                                                    "--router-file=" + routerFile.string()})
    {
        commandPipe.waitForConnection();
    }

    DirEnumResult enumerateDir(const std::filesystem::path& dirPath) override
    {
        commandPipe << "list_dir\x1f" << std::filesystem::absolute(dirPath).string() << '\n';
        std::string itemRefsStr = commandPipe.readLine();
        auto subItemPaths = mapVector<std::string, ItemRef>(split(itemRefsStr, "\x1f"),
                                     [](const auto& strPath) { return ItemRef { .refTarget = std::filesystem::path(strPath) }; });
        std::string subfoldersStr = commandPipe.readLine();
        auto subfolders = mapVector<std::string, Subfolder>(split(subfoldersStr, "\x1f"),
                                   [](const auto& folderName) { return Subfolder { .folderName = folderName }; });

        return {
            .itemRefs = subItemPaths,
            .subfolders = subfolders
        };
    }
};


#endif //VIEWFS_PYTHONSUBPROCESSSOURCE_H
