//
// Created by sdk on 5/28/2022.
//

#include "Utils.h"

std::string replaceAll(const std::string &src, const std::string &key, const std::string &replacement) {
    std::stringstream builder;
    size_t lastStart, lastEnd = 0;
    while((lastStart = src.find(key, lastEnd)) != std::string::npos)
    {
        builder << src.substr(lastEnd, lastStart - lastEnd) << replacement;
        lastEnd = lastStart + key.size();
    }

    builder << src.substr(lastEnd);
    return builder.str();
}

std::vector<std::string> split(const std::string &src, const std::string &delim) {
    size_t lastStart, lastEnd = 0;
    std::vector<std::string> results;

    while((lastStart = src.find(delim, lastEnd)) != std::string::npos)
    {
        results.push_back(src.substr(lastEnd, lastStart - lastEnd));
        lastEnd = lastStart + delim.size();
    }

    results.push_back(src.substr(lastEnd));
    return results;
}
