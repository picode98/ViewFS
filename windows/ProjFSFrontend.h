//
// Created by sdk on 5/31/2022.
//

#ifndef VIEWFS_PROJFSFRONTEND_H
#define VIEWFS_PROJFSFRONTEND_H


#include "ProjFSBase.h"
#include "../IFileSource.h"

struct ProjFSFrontendSessionState
{
    std::vector<std::filesystem::path> pathList;
    size_t startIndex = 0;
};

class ProjFSFrontend : public ProjFSBase<ProjFSFrontendSessionState> {
    IFileSource& fileSource;
    std::map<std::filesystem::path, std::filesystem::path> symlinkTargets;
public:
    explicit ProjFSFrontend(const std::filesystem::path& rootPath, IFileSource& fileSource)
    : ProjFSBase<ProjFSFrontendSessionState>(rootPath), fileSource(fileSource)
    {}

    HRESULT OnDirectoryEnumerationStart(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                        std::optional<ProjFSFrontendSessionState> &sessionState) override;

    HRESULT OnDirectoryEnumerationEnd(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                      std::optional<ProjFSFrontendSessionState> &sessionState) override;

    HRESULT OnDirectoryEnumerationRequested(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                            PCWSTR searchExpression, PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle,
                                            std::optional<ProjFSFrontendSessionState> &sessionState) override;

    HRESULT OnPlaceholderInfoGet(const PRJ_CALLBACK_DATA *callbackData) override;

    HRESULT OnFileInfoRequested(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) override;
};


#endif //VIEWFS_PROJFSFRONTEND_H
