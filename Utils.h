// Utils.h: Non-platform-specific helper functions

#ifndef VIEWFS_UTILS_H
#define VIEWFS_UTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <filesystem>

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

template<typename IType, typename StrType>
StrType join(IType begin, IType end, const StrType& delim)
{
    std::basic_stringstream<typename StrType::value_type> outStream;

    for(IType thisElem = begin; thisElem != end; ++thisElem)
    {
        if(thisElem != begin) outStream << delim;

        outStream << *thisElem;
    }

    return outStream.str();
}

template<typename T>
bool vectorAny(const std::vector<T>& items, std::function<bool(const T&)> predicate)
{
    for(const T& item : items)
    {
        if(predicate(item)) return true;
    }

    return false;
}

//std::vector<std::filesystem::path::string_type> slicePath(const std::filesystem::path& filePath, size_t start,
//                                                          size_t end = std::numeric_limits<size_t>::max())
//{
//    std::vector<std::filesystem::path::string_type> result;
//    size_t i = 0;
//    for(const auto& part : filePath)
//    {
//        if(i >= start && (end == std::numeric_limits<size_t>::max() || i < end))
//        {
//            result.push_back(part);
//        }
//    }
//
//    return result;
//}

#endif //VIEWFS_UTILS_H
