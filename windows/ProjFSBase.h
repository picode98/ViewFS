//
// Created by sdk on 5/25/2022.
//

#ifndef VIEWFS_PROJFSBASE_H
#define VIEWFS_PROJFSBASE_H

#include <filesystem>
#include <functional>
#include <map>
#include <optional>

#include "winapi_headers.h"
#include "WinUtils.h"

class GUIDCompare
{
public:
    bool operator()(const GUID& g1, const GUID& g2) const
    {
        if(g1.Data1 < g2.Data1) return true;
        else if(g1.Data1 == g2.Data1)
        {
            if(g1.Data2 < g2.Data2) return true;
            else if(g1.Data2 == g2.Data2)
            {
                if(g1.Data3 < g2.Data3) return true;
                else if(g1.Data3 == g2.Data3)
                {
                    return std::memcmp(g1.Data4, g2.Data4, sizeof(GUID::Data4)) < 0;
                }
                else return false;
            }
            else return false;
        }
        else return false;
    }
};

template<typename DirEnumSessionType>
class ProjFSBase {
protected:
    GUID instanceID;
    PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT instanceHandle;
    std::filesystem::path rootPath;

    std::map<GUID, std::optional<DirEnumSessionType>, GUIDCompare> currentDirEnumSessions;
private:
    static HRESULT startEnumCBHelper(const PRJ_CALLBACK_DATA* p1, const GUID* p2)
    {
        auto instancePtr = reinterpret_cast<ProjFSBase*>(p1->InstanceContext);
        instancePtr->currentDirEnumSessions.insert({*p2, std::optional<DirEnumSessionType>()});

        return instancePtr->OnDirectoryEnumerationStart(p1, p2, instancePtr->currentDirEnumSessions.at(*p2));
    }

    static HRESULT endEnumCBHelper(const PRJ_CALLBACK_DATA* p1, const GUID* p2)
    {
        auto instancePtr = reinterpret_cast<ProjFSBase*>(p1->InstanceContext);

        HRESULT result = instancePtr->OnDirectoryEnumerationEnd(p1, p2, instancePtr->currentDirEnumSessions.at(*p2));
        instancePtr->currentDirEnumSessions.erase(*p2);
        return result;
    }

    static HRESULT getPlaceholderInfoCBHelper(const PRJ_CALLBACK_DATA* p1)
    {
        return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnPlaceholderInfoGet(p1);
    }

    static HRESULT getEnumCBHelper(const PRJ_CALLBACK_DATA* p1, const GUID* p2, PCWSTR p3, PRJ_DIR_ENTRY_BUFFER_HANDLE p4)
    {
        auto instancePtr = reinterpret_cast<ProjFSBase*>(p1->InstanceContext);
        return instancePtr->OnDirectoryEnumerationRequested(p1, p2, p3, p4, instancePtr->currentDirEnumSessions.at(*p2));
    }

    static HRESULT fileInfoRequestCBHelper(const PRJ_CALLBACK_DATA* p1, UINT64 p2, UINT32 p3)
    {
        return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnFileInfoRequested(p1, p2, p3);
    }

public:
    explicit ProjFSBase(const std::filesystem::path& rootPath) : rootPath(rootPath)
    {
        try
        {
            std::filesystem::create_directory(rootPath);
        }
        catch(const std::filesystem::filesystem_error& ex)
        {
            if(ex.code().value() != ERROR_ALREADY_EXISTS) throw;
        }

        std::wstring winAPIRootPath = rootPath;

        this->instanceID = newGUID();
        handleWinAPIError(PrjMarkDirectoryAsPlaceholder(winAPIRootPath.c_str(),
                                                        nullptr, nullptr, &this->instanceID), false);
    }

    void startVirtualFS()
    {
        PRJ_CALLBACKS cbTable;
        std::memset(&cbTable, 0, sizeof(PRJ_CALLBACKS));

        // this->startEnumCB = [](const PRJ_CALLBACK_DATA* p1, auto p2) { return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnDirectoryEnumerationStart(p1, p2); };
        // this->endEnumCB = [](const PRJ_CALLBACK_DATA* p1, auto p2) { return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnDirectoryEnumerationEnd(p1, p2); };
        // this->getEnumCB = [](const PRJ_CALLBACK_DATA* p1, auto p2, auto p3, auto p4) { return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnDirectoryEnumerationRequested(p1, p2, p3, p4); };
        // this->getPlaceholderInfoCB = [](const PRJ_CALLBACK_DATA* p1) { return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnPlaceholderInfoGet(p1); };
        // this->fileInfoRequestCB = [](const PRJ_CALLBACK_DATA* p1, auto p2, auto p3) { return reinterpret_cast<ProjFSBase*>(p1->InstanceContext)->OnFileInfoRequested(p1, p2, p3); };

        cbTable.StartDirectoryEnumerationCallback = &ProjFSBase::startEnumCBHelper;
        cbTable.EndDirectoryEnumerationCallback = &ProjFSBase::endEnumCBHelper;
        cbTable.GetDirectoryEnumerationCallback = &ProjFSBase::getEnumCBHelper;
        cbTable.GetPlaceholderInfoCallback = &ProjFSBase::getPlaceholderInfoCBHelper;
        cbTable.GetFileDataCallback = &ProjFSBase::fileInfoRequestCBHelper;

        handleWinAPIError(PrjStartVirtualizing(UTF8StrToWideStr(rootPath.string()).c_str(), &cbTable, this, nullptr, &this->instanceHandle), false);
    }

    virtual HRESULT OnDirectoryEnumerationStart(
        _In_ const PRJ_CALLBACK_DATA* callbackData,
        _In_ const GUID* enumerationId,
        std::optional<DirEnumSessionType>& sessionState
    ) = 0;

    virtual HRESULT OnDirectoryEnumerationEnd(
            _In_ const PRJ_CALLBACK_DATA* callbackData,
            _In_ const GUID* enumerationId,
            std::optional<DirEnumSessionType>& sessionState
    ) = 0;

    virtual HRESULT OnDirectoryEnumerationRequested(
            _In_ const PRJ_CALLBACK_DATA* callbackData,
            _In_ const GUID* enumerationId,
            _In_opt_ PCWSTR searchExpression,
            _In_ PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle,
            std::optional<DirEnumSessionType>& sessionState
    ) = 0;

    virtual HRESULT OnPlaceholderInfoGet(
            _In_ const PRJ_CALLBACK_DATA* callbackData
    ) = 0;

    virtual HRESULT OnFileInfoRequested(
            _In_ const PRJ_CALLBACK_DATA* callbackData,
            _In_ UINT64 byteOffset,
            _In_ UINT32 length
    ) = 0;

    virtual ~ProjFSBase()
    {
        PrjStopVirtualizing(this->instanceHandle);
    }
};


#endif //VIEWFS_PROJFSBASE_H
