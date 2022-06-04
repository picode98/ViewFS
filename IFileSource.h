//
// Created by sdk on 5/31/2022.
//

#ifndef VIEWFS_IFILESOURCE_H
#define VIEWFS_IFILESOURCE_H

struct ItemRef
{
    std::filesystem::path refTarget;
};

struct Subfolder
{
    std::string folderName;
};

struct DirEnumResult
{
    std::vector<ItemRef> itemRefs;
    std::vector<Subfolder> subfolders;
};

class IFileSource
{
public:
    virtual DirEnumResult enumerateDir(const std::filesystem::path& dirPath) = 0;
};

#endif //VIEWFS_IFILESOURCE_H
