//
// Created by sdk on 5/28/2022.
//

#ifndef VIEWFS_SUBPROCESS_H
#define VIEWFS_SUBPROCESS_H

#include "winapi_headers.h"
#include "WinUtils.h"
#include "../Utils.h"

#include <filesystem>
#include <optional>
#include <sstream>

class Subprocess {
    HANDLE procHandle;

    static tstring getCommandLine(const std::filesystem::path& executablePath, const std::vector<std::string>& args)
    {
        std::stringstream builder;
        builder << '\"' << executablePath.u8string() << '\"';

        for(const auto& arg : args)
        {
            builder << " \"" << replaceAll(arg, "\"", "\\\"") << '\"';
        }

        return UTF8StringToTString(builder.str());
    }

public:
    Subprocess(const std::filesystem::path& executablePath, const std::vector<std::string>& args,
               const std::optional<std::filesystem::path>& outputLogPath = {})
    {
        // See https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
        STARTUPINFO startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION newProcInfo;
        ZeroMemory(&newProcInfo, sizeof(newProcInfo));

        tstring commandLine = getCommandLine(executablePath, args);

        if(outputLogPath.has_value())
        {
            SECURITY_ATTRIBUTES inheritHandle = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

            startupInfo.hStdOutput = startupInfo.hStdError = CreateFile(UTF8StringToTString(outputLogPath->u8string()).c_str(),
                                                                        FILE_APPEND_DATA,
                                                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                                        &inheritHandle,
                                                                        OPEN_ALWAYS,
                                                                        FILE_ATTRIBUTE_NORMAL,
                                                                        nullptr);

            if(startupInfo.hStdOutput == nullptr || startupInfo.hStdOutput == INVALID_HANDLE_VALUE) throw getWinAPIError(GetLastError());

            startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        }

        SetLastError(ERROR_SUCCESS);
        CreateProcess(UTF8StringToTString(executablePath.u8string()).c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo,
                      &newProcInfo);
        handleWinAPIError(ERROR_SUCCESS);

        this->procHandle = newProcInfo.hProcess;
    }

    std::optional<DWORD> exitCode()
    {
        DWORD exitCode;
        if(!GetExitCodeProcess(this->procHandle, &exitCode)) throw getWinAPIError(GetLastError());

        if(exitCode == STILL_ACTIVE)
        {
            return {};
        }
        else
        {
            return exitCode;
        }
    }

    ~Subprocess()
    {
        TerminateProcess(this->procHandle, 1);
    }
};


#endif //VIEWFS_SUBPROCESS_H
