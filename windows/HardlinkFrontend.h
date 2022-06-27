//
// Created by sdk on 6/12/2022.
//

#ifndef VIEWFS_HARDLINKFRONTEND_H
#define VIEWFS_HARDLINKFRONTEND_H


#include <filesystem>
#include <map>

#include "../IFileSource.h"

class HardlinkFrontend {
    std::map<std::filesystem::path, std::filesystem::path> existingHardlinks;
    void mirrorDirContents(const std::filesystem::path& dirPath, const DirEnumResult& newContents);
    void recursiveMirror(const std::filesystem::path& rootPath, size_t depth = 1);

    IFileSource& fileSource;
    size_t depthLimit = 4;

public:
    explicit HardlinkFrontend(const std::filesystem::path& rootPath, IFileSource& fileSource): fileSource(fileSource)
    {
        std::filesystem::create_directory(rootPath);
        recursiveMirror(rootPath);
    }
};


#endif //VIEWFS_HARDLINKFRONTEND_H
