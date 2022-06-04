// Utils.h: Non-platform-specific helper functions

#ifndef VIEWFS_UTILS_H
#define VIEWFS_UTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>

std::string replaceAll(const std::string& src, const std::string& key, const std::string& replacement);

std::vector<std::string> split(const std::string &src, const std::string &delim, bool keepEmpty = false);

template<typename SrcType, typename DstType>
std::vector<DstType> mapVector(const std::vector<SrcType>& src, std::function<DstType(const SrcType&)> mapper)
{
    std::vector<DstType> results(src.size());

    for(size_t i = 0; i < src.size(); ++i)
    {
        results[i] = mapper(src[i]);
    }

    return results;
}

#endif //VIEWFS_UTILS_H
