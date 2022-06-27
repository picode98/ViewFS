//
// Created by sdk on 5/25/2022.
//

#ifndef VIEWFS_WINUTILS_H
#define VIEWFS_WINUTILS_H

#include "winapi_headers.h"

#include <string>
#include <system_error>
#include <vector>
#include <filesystem>

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

std::wstring UTF8StrToWideStr(const std::string& UTF8Str);
std::string WideStrToUTF8Str(const std::wstring& wideStr);

#ifdef UNICODE
#define TStringToUTF8String WideStrToUTF8Str
#define UTF8StringToTString UTF8StrToWideStr
#else
#define TStringToUTF8String(str) str
#define UTF8StringToTString(str) str
#endif

class WindowsException : public std::system_error
{
public:
    WindowsException(DWORD errorCode, const tstring& description): system_error(errorCode, std::system_category(), TStringToUTF8String(description))
    {}
};

WindowsException getWinAPIError(LSTATUS retVal);
void handleWinAPIError(LSTATUS retVal, bool checkGLE = true);
void handleCOMError(HRESULT result);

inline GUID newGUID()
{
    GUID result;
    handleCOMError(CoCreateGuid(&result));
    return result;
}

inline std::string GUIDToString(const GUID& guid)
{
    wchar_t resultBuf[64] = {0};
    StringFromGUID2(guid, resultBuf, 64);
    return WideStrToUTF8Str(resultBuf);
}

std::vector<std::filesystem::path> getAllHardlinks(const std::filesystem::path& destination);

#endif //VIEWFS_WINUTILS_H
