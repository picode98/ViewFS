//
// Created by sdk on 5/31/2022.
//

#include <cassert>
#include "ProjFSFrontend.h"

HRESULT ProjFSFrontend::OnDirectoryEnumerationStart(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                                    std::optional<ProjFSFrontendSessionState> &sessionState) {
    auto relPath = std::filesystem::path(callbackData->FilePathName);
    sessionState = { .resultSet = this->fileSource.enumerateDir(relPath.empty() ? this->rootPath : this->rootPath / relPath) };

    for(const ItemRef& entry : sessionState->resultSet.itemRefs)
    {
        this->fsObjects[relPath / entry.refTarget.filename()] = entry;
    }

    for(const Subfolder& entry : sessionState->resultSet.subfolders)
    {
        this->fsObjects[relPath / entry.folderName] = entry;
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
    for(; i < sessionState->resultSet.itemRefs.size(); ++i)
    {
        std::wstring sourceFilenameWideStr = sessionState->resultSet.itemRefs[i].refTarget.filename(),
            targetWideStr = sessionState->resultSet.itemRefs[i].refTarget;

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
            else break;
        }
    }

    for(; i >= sessionState->resultSet.itemRefs.size()
        && i < sessionState->resultSet.itemRefs.size() + sessionState->resultSet.subfolders.size(); ++i)
    {
        size_t idx = i - sessionState->resultSet.itemRefs.size();
        PRJ_FILE_BASIC_INFO info = { TRUE, 0 };
        std::wstring wideFolderName = UTF8StrToWideStr(sessionState->resultSet.subfolders[idx].folderName);

        try
        {
            handleWinAPIError(PrjFillDirEntryBuffer2(dirEntryBufferHandle, wideFolderName.c_str(), &info, nullptr), false);
        }
        catch(const WindowsException& ex)
        {
            if(ex.code().value() != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) throw;
            else break;
        }
    }

    sessionState->startIndex = i;

    return S_OK;
}

HRESULT ProjFSFrontend::OnPlaceholderInfoGet(const PRJ_CALLBACK_DATA *callbackData) {
    std::filesystem::path srcRelPath = callbackData->FilePathName;

    if(this->fsObjects.count(srcRelPath) > 0)
    {
        if(ItemRef* itemRef = std::get_if<ItemRef>(&this->fsObjects.at(srcRelPath)))
        {
            std::wstring targetPath = itemRef->refTarget;

            PRJ_PLACEHOLDER_INFO info = { { FALSE, 0 } };
            PRJ_EXTENDED_INFO extInfo = { .InfoType = PRJ_EXT_INFO_TYPE_SYMLINK, .NextInfoOffset = 0,
                    .Symlink = { .TargetName = targetPath.c_str() } };

            handleWinAPIError(PrjWritePlaceholderInfo2(callbackData->NamespaceVirtualizationContext, callbackData->FilePathName, &info,
                                     sizeof(info), &extInfo), false);
            return S_OK;
        }
        else if(Subfolder* subfolder = std::get_if<Subfolder>(&this->fsObjects.at(srcRelPath)))
        {
            PRJ_PLACEHOLDER_INFO info = { { TRUE, 0 } };

            handleWinAPIError(PrjWritePlaceholderInfo2(callbackData->NamespaceVirtualizationContext, callbackData->FilePathName, &info,
                                                       sizeof(info), nullptr), false);
            return S_OK;
        }
        else
        {
            throw std::runtime_error("Unsupported object type in virtual filesystem");
        }
//        std::wstring targetPath = this->symlinkTargets.at(srcRelPath);
//
//        PRJ_PLACEHOLDER_INFO info = { { FALSE, 0 } };
//        PRJ_EXTENDED_INFO extInfo = { .InfoType = PRJ_EXT_INFO_TYPE_SYMLINK, .NextInfoOffset = 0,
//                .Symlink = { .TargetName = targetPath.c_str() } };
//
//        handleWinAPIError(PrjWritePlaceholderInfo2(callbackData->NamespaceVirtualizationContext, callbackData->FilePathName, &info,
//                                 sizeof(info), &extInfo), false);
//        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
}

HRESULT ProjFSFrontend::OnFileInfoRequested(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) {
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}
