//
// Created by sdk on 5/31/2022.
//

#ifndef VIEWFS_IFILESOURCE_H
#define VIEWFS_IFILESOURCE_H

#include <set>
#include <map>
#include <cassert>

#include "Utils.h"

typedef std::filesystem::path::string_type fsString;

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

    std::vector<fsString> getDisambiguatedNames() const
    {
        std::map<fsString, std::vector<std::pair<size_t, std::filesystem::path>>> matchingSets;

        for(size_t i = 0; i < this->itemRefs.size(); ++i)
        {
            if(matchingSets.count(this->itemRefs[i].refTarget.filename()) == 0)
            {
                matchingSets.insert({this->itemRefs[i].refTarget.filename(), {{i, this->itemRefs[i].refTarget}}});
            }
            else
            {
                matchingSets.at(this->itemRefs[i].refTarget.filename()).push_back({i, this->itemRefs[i].refTarget});
            }
        }

        std::vector<fsString> result(this->itemRefs.size());
        for(auto& thisSetEntry : matchingSets)
        {
            if(thisSetEntry.second.size() >= 2)
            {
                std::set<fsString> seenNames;
                bool repeatSeen = false;
                do {
                    for(auto& pathEntry: thisSetEntry.second)
                    {
                        pathEntry.second = pathEntry.second.parent_path();
                        if(!pathEntry.second.empty() && seenNames.contains(pathEntry.second.filename()))
                        {
                            repeatSeen = true;
                        }
                    }

                } while(repeatSeen);

                size_t emptyCount = 0;

                for(auto& pathEntry: thisSetEntry.second)
                {
                    std::basic_stringstream<fsString::value_type> pathBuilder;
                    std::filesystem::path::string_type origFilename = this->itemRefs.at(pathEntry.first).refTarget.filename();
                    size_t extStart = origFilename.find('.');

                    pathBuilder << (extStart == fsString::npos ? origFilename : origFilename.substr(0, extStart))
                                << " (";

                    if(pathEntry.second.empty())
                    {
                        ++emptyCount;
                        pathBuilder << emptyCount;
                    }
                    else
                    {
                        pathBuilder << static_cast<fsString>(pathEntry.second.filename());
                    }

                    pathBuilder << ')';
                    if(extStart != fsString::npos) pathBuilder << origFilename.substr(extStart);

                    result.at(pathEntry.first) = pathBuilder.str();
                }
            }
            else
            {
                result.at(thisSetEntry.second.at(0).first) = thisSetEntry.first;
            }
        }

        return result;
    }
//        auto currentPaths =
//                mapVector<ItemRef, std::filesystem::path>(this->itemRefs, [](const auto& thisItemRef) { return thisItemRef.refTarget; });
//        auto numChunksNeeded = std::vector<size_t>(currentPaths.size(), 0);
//        std::set<size_t> activeIndices;
//
//        do {
//            std::map<fsString, size_t> firstNameSightings;
//            std::set<size_t> newActiveIndices;
//
//            for(size_t i : activeIndices)
//            {
//                if(currentPaths[i].empty()) continue;
//
//                if(firstNameSightings.count(currentPaths[i].filename()) > 0)
//                {
//                    newActiveIndices.insert(firstNameSightings[currentPaths[i].filename()]);
//                    newActiveIndices.insert(i);
//                }
//                else
//                {
//                    firstNameSightings[currentPaths[i].filename()] = i;
//                }
//            }
//
//            activeIndices = std::move(newActiveIndices);
//
//            for(size_t i : activeIndices)
//            {
//                ++numChunksNeeded[i];
//                currentPaths[i].remove_filename();
//            }
//        } while(!activeIndices.empty());
//
//        std::vector<fsString> resultNames(this->itemRefs.size());
//        for(size_t i = 0; i < this->itemRefs.size(); ++i)
//        {
//            if(numChunksNeeded[i] == 0)
//            {
//                resultNames[i] = this->itemRefs[i].refTarget.filename();
//            }
//            else
//            {
//                std::basic_stringstream<fsString::value_type> pathBuilder;
//                std::filesystem::path::string_type origFilename = this->itemRefs[i].refTarget.filename();
//                size_t extStart = origFilename.find('.');
//
//                pathBuilder << (extStart == fsString::npos ? origFilename : origFilename.substr(0, extStart))
//                            << ' ';
//                auto pathParts = slicePath(this->itemRefs[i].refTarget, 0);
//
//                if(numChunksNeeded[i] < pathParts.size())
//                {
//
//                }
//            }
//        }
//    }
};

class IFileSource
{
public:
    virtual DirEnumResult enumerateDir(const std::filesystem::path& dirPath) = 0;
};

#endif //VIEWFS_IFILESOURCE_H
