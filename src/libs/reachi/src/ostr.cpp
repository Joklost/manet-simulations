#include <reachi/ostr.h>

std::ostream &operator<<(std::ostream &os, const std::pair<Node, Node> &pair) {
    return os << "{" << pair.first << ", " << pair.second << "}";
}
