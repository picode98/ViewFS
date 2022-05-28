#include "windows/winapi_headers.h"
#include "windows/ProjFSBase.h"
#include "windows/NamedPipe.h"

#include <map>
#include <cassert>
#include <iostream>

class ProjFSTest : public ProjFSBase<bool>
{
    static const std::map<std::wstring, std::string> testEntries;
public:
    ProjFSTest() : ProjFSBase(L".\\test_fs") {}

    HRESULT OnDirectoryEnumerationStart(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId, std::optional<bool>& completionState) override {
        completionState = false;
        return S_OK;
    }

    HRESULT OnDirectoryEnumerationEnd(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId, std::optional<bool>& completionState) override {
        return S_OK;
    }

    HRESULT OnDirectoryEnumerationRequested(const PRJ_CALLBACK_DATA *callbackData, const GUID *enumerationId,
                                            PCWSTR searchExpression,
                                            PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle,
                                            std::optional<bool>& completionState) override {
        if(completionState.value_or(false)) return S_OK;

        for(const auto& entry : testEntries)
        {
            PRJ_FILE_BASIC_INFO info = { FALSE, static_cast<INT64>(entry.second.size()) };

            handleWinAPIError(PrjFillDirEntryBuffer(entry.first.c_str(), &info, dirEntryBufferHandle), false);
        }

        completionState = true;

        return S_OK;
    }

    HRESULT OnPlaceholderInfoGet(const PRJ_CALLBACK_DATA *callbackData) override {
        for(const auto& entry : testEntries)
        {
            if(PrjFileNameCompare(callbackData->FilePathName, entry.first.c_str()) == 0)
            {
                PRJ_PLACEHOLDER_INFO info = { { FALSE, static_cast<INT64>(entry.second.size()) } };
                handleWinAPIError(PrjWritePlaceholderInfo(this->instanceHandle, callbackData->FilePathName, &info, sizeof(info)), false);
                return S_OK;
            }
        }

        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    HRESULT OnFileInfoRequested(const PRJ_CALLBACK_DATA *callbackData, UINT64 byteOffset, UINT32 length) override {
        void* dataBuf = PrjAllocateAlignedBuffer(this->instanceHandle, length);

        for(const auto& entry : testEntries)
        {
            if(PrjFileNameCompare(callbackData->FilePathName, entry.first.c_str()) == 0)
            {
                assert(byteOffset + length <= entry.second.size());
                std::memcpy(dataBuf, entry.second.c_str() + byteOffset, length);
                handleWinAPIError(PrjWriteFileData(this->instanceHandle, &callbackData->DataStreamId, dataBuf, byteOffset, length), false);
                return S_OK;
            }
        }

        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

};
const std::map<std::wstring, std::string> ProjFSTest::testEntries = { { L"aFile.txt", "This is some text." },
                                                                      { L"anotherFile.docx", "This is not really a DOCX." } };

int main() {
//    auto testFS = ProjFSTest();
//    testFS.startVirtualFS();
//    system("pause");

    try {
        NamedPipeServer server(TEXT("aPipe"));
        server.waitForConnection();
        std::cout << "Connected!" << std::endl;

        while(true)
        {
            server << "list_dir\x1f" << "C:\\Program Files\\Python39" << '\n';
            std::string result = server.readLine();
            std::cout << result << std::endl;
        }
    } catch(const WindowsException& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
