//
// Created by sdk on 5/31/2022.
//

#ifndef VIEWFS_IFILESOURCE_H
#define VIEWFS_IFILESOURCE_H

class IFileSource
{
public:
    virtual std::vector<std::filesystem::path> enumerateDir(const std::filesystem::path& dirPath) = 0;
};

#endif //VIEWFS_IFILESOURCE_H
