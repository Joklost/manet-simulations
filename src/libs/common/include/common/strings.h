#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <vector>
#include <string>

namespace common {
    std::vector<std::string> split(const std::string &string, const std::string &delim);
}

#endif /* COMMON_STRING_H */
