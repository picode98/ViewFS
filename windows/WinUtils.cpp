//
// Created by sdk on 5/25/2022.
//

#include "WinUtils.h"

std::wstring UTF8StrToWideStr(const std::string& UTF8Str)
{
    int reqBufSize = MultiByteToWideChar(CP_UTF8, 0, UTF8Str.c_str(), UTF8Str.size(), nullptr, 0);
    std::wstring str(reqBufSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, UTF8Str.c_str(), UTF8Str.size(), str.data(), str.size());

    return str;
}

std::string WideStrToUTF8Str(const std::wstring& wideStr)
{
    if (wideStr.empty()) return "";

    int reqBufSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), wideStr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(reqBufSize, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), wideStr.size(), str.data(), str.size(), nullptr, nullptr);

    return str;
}

WindowsException getWinAPIError(LSTATUS retVal)
{
    if (retVal != ERROR_SUCCESS)
    {
        TCHAR* errorStrPtr;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, retVal,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&errorStrPtr), 0, nullptr);
        tstring errorDesc(errorStrPtr);
        LocalFree(errorStrPtr);

        return WindowsException(retVal, errorDesc);
    }
    else
    {
        return WindowsException(ERROR_SUCCESS, TEXT(""));
    }
}

void handleWinAPIError(LSTATUS retVal, bool checkGLE)
{
    WindowsException error(ERROR_SUCCESS, tstring(TEXT("")));
    auto test = GetLastError();
    if (retVal != ERROR_SUCCESS)
    {
        error = getWinAPIError(retVal);
    }
    else if (checkGLE)
    {
        error = getWinAPIError(GetLastError());
    }

    if (error.code().value() != ERROR_SUCCESS)
    {
        throw error;
    }
}

void handleCOMError(HRESULT result)
{
    if(result != S_OK)
    {
        throw getWinAPIError(result);
    }
}

std::vector<std::filesystem::path> getAllHardlinks(const std::filesystem::path& destination)
{
    std::vector<std::filesystem::path> result;

    wchar_t stackBuf[256];
    DWORD neededBufSize = 256;

    HANDLE enumHandle = FindFirstFileNameW(destination.c_str(), 0, &neededBufSize, stackBuf);
    if(enumHandle == INVALID_HANDLE_VALUE)
    {
        if(GetLastError() == ERROR_MORE_DATA)
        {
            auto *heapBuf = new wchar_t[neededBufSize];
            enumHandle = FindFirstFileNameW(destination.c_str(), 0, &neededBufSize, heapBuf);

            if(enumHandle != INVALID_HANDLE_VALUE) result.emplace_back(heapBuf);
            delete[] heapBuf;
        }

        if(enumHandle == INVALID_HANDLE_VALUE) throw getWinAPIError(GetLastError());
    }
    else
    {
        result.emplace_back(stackBuf);
    }

    BOOL nextResult = TRUE;
    while(nextResult == TRUE)
    {
        neededBufSize = 256;
        nextResult = FindNextFileNameW(enumHandle, &neededBufSize, stackBuf);

        if(nextResult == 0)
        {
            if(GetLastError() == ERROR_MORE_DATA)
            {
                auto *heapBuf = new wchar_t[neededBufSize];
                nextResult = FindNextFileNameW(enumHandle, &neededBufSize, heapBuf);

                if(nextResult != 0) result.emplace_back(heapBuf);
                delete[] heapBuf;
            }
            else if(GetLastError() == ERROR_HANDLE_EOF) break;

            if(nextResult == 0)
            {
                auto error = getWinAPIError(GetLastError());
                FindClose(enumHandle);
                throw error;
            }
        }
        else
        {
            result.emplace_back(stackBuf);
        }
    }

    FindClose(enumHandle);
    return result;
}