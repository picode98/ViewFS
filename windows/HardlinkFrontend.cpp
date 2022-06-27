//
// Created by sdk on 6/12/2022.
//

#include "HardlinkFrontend.h"

#include "WinUtils.h"

void HardlinkFrontend::mirrorDirContents(const std::filesystem::path& dirPath, const DirEnumResult& newContents)
{
    auto disambiguatedNames = newContents.getDisambiguatedNames();
    for(size_t i = 0; i < newContents.itemRefs.size(); ++i)
    {
        try
        {
            auto existingItems = getAllHardlinks(dirPath / disambiguatedNames.at(i));
            bool linkFound = false;
            for(auto& item : existingItems)
            {
                if(item == newContents.itemRefs.at(i).refTarget)
                {
                    linkFound = true;
                    break;
                }
            }
        }
        catch(const WindowsException& ex)
        {
            if(ex.code().value() == ERROR_FILE_NOT_FOUND)
            {
                std::filesystem::create_hard_link(newContents.itemRefs.at(i).refTarget,
                                                  dirPath / disambiguatedNames.at(i));
            }
            else throw;
        }
    }

    for(const Subfolder& thisFolder : newContents.subfolders)
    {
        std::filesystem::create_directory(dirPath / thisFolder.folderName);
    }
}

void HardlinkFrontend::recursiveMirror(const std::filesystem::path &rootPath, size_t depth)
{
    DirEnumResult enumResult = this->fileSource.enumerateDir(rootPath);
    this->mirrorDirContents(rootPath, enumResult);

    if(depth <= this->depthLimit)
    {
        for (const Subfolder &thisFolder: enumResult.subfolders)
        {
            this->recursiveMirror(rootPath / thisFolder.folderName, depth + 1);
        }
    }
}
