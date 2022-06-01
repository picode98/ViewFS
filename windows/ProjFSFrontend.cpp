//
// Created by sdk on 5/31/2022.
//

#include "ProjFSFrontend.h"

HRESULT ProjFSFrontend::OnDirectoryEnumerationStart(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                                    std::optional<ProjFSFrontendSessionState> &sessionState) {
    auto path = std::filesystem::path(callbackData->FilePathName);
    sessionState = { .pathList = this->fileSource.enumerateDir(path) };

    for(const std::filesystem::path& entry : sessionState->pathList)
    {
        this->symlinkTargets[path / entry.filename()] = entry;
    }

    return S_OK;
}

HRESULT ProjFSFrontend::OnDirectoryEnumerationEnd(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                                  std::optional<ProjFSFrontendSessionState> &sessionState) {
    return S_OK;
}

HRESULT
ProjFSFrontend::OnDirectoryEnumerationRequested(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                                PCWSTR searchExpression,
                                                PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle,
                                                std::optional<ProjFSFrontendSessionState> &sessionState) {
    size_t i = sessionState->startIndex;
    for(; i < sessionState->pathList.size(); ++i)
    {
        std::wstring sourceFilenameWideStr = sessionState->pathList[i].filename(),
            targetWideStr = sessionState->pathList[i];

        PRJ_FILE_BASIC_INFO info = { FALSE, 0 };
        PRJ_EXTENDED_INFO extInfo = { .InfoType = PRJ_EXT_INFO_TYPE_SYMLINK, .NextInfoOffset = 0,
                                      .Symlink = { .TargetName = targetWideStr.c_str() } };

        try
        {
            handleWinAPIError(PrjFillDirEntryBuffer2(dirEntryBufferHandle, sourceFilenameWideStr.c_str(), &info, &extInfo), false);
        }
        catch(const WindowsException& ex)
        {
            if(ex.code().value() != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) throw;
        }
    }

    sessionState->startIndex = i;

    return S_OK;
}

HRESULT ProjFSFrontend::OnPlaceholderInfoGet(const PRJ_CALLBACK_DATA *callbackData) {
    std::filesystem::path srcRelPath = callbackData->FilePathName;

    if(this->symlinkTargets.count(srcRelPath) > 0)
    {
        std::wstring targetPath = this->symlinkTargets.at(srcRelPath);

        PRJ_PLACEHOLDER_INFO info = { { FALSE, 0 } };
        PRJ_EXTENDED_INFO extInfo = { .InfoType = PRJ_EXT_INFO_TYPE_SYMLINK, .NextInfoOffset = 0,
                .Symlink = { .TargetName = targetPath.c_str() } };

        handleWinAPIError(PrjWritePlaceholderInfo2(callbackData->NamespaceVirtualizationContext, callbackData->FilePathName, &info,
                                 sizeof(info), &extInfo), false);
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
}

HRESULT ProjFSFrontend::OnFileInfoRequested(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) {
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}
