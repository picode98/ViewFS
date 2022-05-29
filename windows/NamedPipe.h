//
// Created by sdk on 5/27/2022.
//

#ifndef VIEWFS_NAMEDPIPE_H
#define VIEWFS_NAMEDPIPE_H

#include <filesystem>
#include <sstream>
#include <cassert>
#include <queue>

#include "winapi_headers.h"
#include "WinUtils.h"

class NamedPipeServer {
    HANDLE pipeHandle;
    bool connected = false;
    std::deque<std::string> readBuffer;
    std::string filename;

    std::string extractLineFromBuffer()
    {
        std::stringstream contentToThisPoint;
        size_t lineEndIdx;
        while((lineEndIdx = this->readBuffer.front().find('\n')) == std::string::npos)
        {
            contentToThisPoint << this->readBuffer.front();
            this->readBuffer.pop_front();
        }

        contentToThisPoint << this->readBuffer.front().substr(0, lineEndIdx);
        this->readBuffer.front() = this->readBuffer.front().substr(lineEndIdx + 1);
        return contentToThisPoint.str();
    }

public:
    explicit NamedPipeServer(const std::string& pipeName) : filename("\\\\.\\pipe\\" + pipeName)
    {
        this->pipeHandle = CreateNamedPipe(UTF8StringToTString(filename).c_str(), PIPE_ACCESS_DUPLEX,
                                           PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);

        if(this->pipeHandle == nullptr || this->pipeHandle == INVALID_HANDLE_VALUE) throw getWinAPIError(GetLastError());
    }

    void waitForConnection()
    {
        BOOL connectResult = ConnectNamedPipe(this->pipeHandle, nullptr);

        if(!connectResult)
        {
            throw getWinAPIError(GetLastError());
        }
        else
        {
            this->connected = true;
        }
    }

    template<typename T>
    NamedPipeServer& operator<<(const T& data)
    {
        assert(this->connected);

        std::ostringstream stream;
        stream << data;
        std::string strData = stream.str();

        DWORD bytesWritten;
        BOOL result = WriteFile(this->pipeHandle, strData.c_str(), strData.size(), &bytesWritten, nullptr);
        if(!result) throw getWinAPIError(GetLastError());

        return *this;
    }

    std::string readLine()
    {
        assert(this->connected);

        bool hasFullLine = false;
        for(std::string& chunk : this->readBuffer)
        {
            if(chunk.find('\n') != std::string::npos)
            {
                hasFullLine = true;
                break;
            }
        }

        if(!hasFullLine)
        {
            char thisRead[256];
            DWORD bytesRead = 0;

            do
            {
                BOOL result = ReadFile(this->pipeHandle, thisRead, sizeof(thisRead) - 1, &bytesRead, nullptr);
                if(!result) throw getWinAPIError(GetLastError());
                thisRead[bytesRead] = TEXT('\0');
                this->readBuffer.emplace_back(thisRead);
            } while(std::find(thisRead, thisRead + bytesRead + 1, '\n') == thisRead + bytesRead + 1);
        }

        return extractLineFromBuffer();
    }

    std::string getFilename() const
    {
        return this->filename;
    }

    ~NamedPipeServer()
    {
        CloseHandle(this->pipeHandle);
    }
};


#endif //VIEWFS_NAMEDPIPE_H
