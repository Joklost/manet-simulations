#include <reachi/linkmap.h>
#include <chrono>
#include <iostream>


bool LinkMap::contains(const uint64_t &row_key, const uint64_t &column_key) const {
    return this->map.find(this->make_pair(row_key, column_key)) != this->map.end();
}

void LinkMap::emplace(const uint64_t &l1, const uint64_t &l2, double value) {
    this->map.emplace(this->make_pair(l1, l2), value);
}

double &LinkMap::operator[](const linkpair key) {
    return this->get(key.first, key.second);
}

double &LinkMap::get(const uint64_t &l1, const uint64_t &l2) {
    return this->map[this->make_pair(l1, l2)];
}

linkpair LinkMap::make_pair(const uint64_t &l1, const uint64_t &l2) const {
    return l1 > l2 ? std::make_pair(l1, l2) : std::make_pair(l2, l1);
}

unsigned long LinkMap::size() const {
    return this->map.size();
}

LinkMap LinkMap::operator*(const double scalar) const {
    LinkMap res;

    for (const auto &element : this->map) {
        res.emplace(element.first.first, element.first.second, element.second * scalar);
    }

    return res;
}

std::vector<uint64_t> LinkMap::get_keypairs(const uint64_t &row_key) const {
    std::vector<uint64_t> res;

    for (const auto &element : this->map) {
        if (element.first.first == row_key) {
            res.emplace_back(element.first.second);
        } else if (element.first.first > row_key) {
            break;
        }
    }

    return res;
}