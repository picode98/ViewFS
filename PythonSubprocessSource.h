//
// Created by sdk on 5/28/2022.
//

#ifndef VIEWFS_PYTHONSUBPROCESSSOURCE_H
#define VIEWFS_PYTHONSUBPROCESSSOURCE_H


#include <filesystem>

#include "PlatformUtils.h"

class PythonSubprocessSource {
    NamedPipeServer commandPipe;
    Subprocess pythonProc;

public:
    PythonSubprocessSource(const std::filesystem::path& pathToPythonExecutable, const std::filesystem::path& routerFile)
        : commandPipe("viewfs-python-" + GUIDToString(newGUID())),
          pythonProc(pathToPythonExecutable, { "..\\ViewFS_python_host.py", "--command-pipe=" + commandPipe.getFilename(),
                                                    "--router-file=" + routerFile.u8string()})
    {
        commandPipe.waitForConnection();
    }

    std::vector<std::filesystem::path> enumerateDir(const std::filesystem::path& dirPath)
    {
        commandPipe << "list_dir\x1f" << dirPath.u8string() << '\n';
        std::string result = commandPipe.readLine();
        return mapVector<std::string, std::filesystem::path>(split(result, "\x1f"), [](const auto& strPath) { return std::filesystem::path(strPath); });
    }
};


#endif //VIEWFS_PYTHONSUBPROCESSSOURCE_H
