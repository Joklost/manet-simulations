#include <reachi/linkmap.h>


bool LinkMap::contains(const Link &l1, const Link &l2) const {
    return this->map.find(this->make_pair(l1, l2)) != this->map.end();
}

void LinkMap::emplace(const Link &l1, const Link &l2, double value) {
    this->map.emplace(this->make_pair(l1, l2), value);
}

double &LinkMap::operator[](const linkpair key) {
    return this->get(key.first, key.second);
}

double &LinkMap::get(const Link &l1, const Link &l2) {
    return this->map[this->make_pair(l1, l2)];
}

linkpair LinkMap::make_pair(const Link &l1, const Link &l2) const {
    return l1 > l2 ? std::make_pair(l1, l2) : std::make_pair(l2, l1);
}

unsigned long LinkMap::size() const {
    return this->map.size();
}

std::vector<std::pair<linkpair, double>> LinkMap::get_keypairs(const Link &l, const uint32_t upper_limit) const {
    std::vector<std::pair<linkpair, double>> res;
    auto id = l.get_id();

    std::for_each(this->map.cbegin(), this->map.cend(), [&res, &id, &upper_limit](auto element) {
        if (element.first.first.get_id() == id && element.first.second.get_id() < upper_limit) {
            res.emplace_back(element);
        }
    });

    return res;
}


LinkMap LinkMap::operator*(const double scalar) const {
    LinkMap res;

    std::for_each(this->map.cbegin(), this->map.cend(), [&res, &scalar](auto element) {
        res.emplace(element.first.first, element.first.second, element.second * scalar);
    });

    return res;
}

std::vector<std::pair<linkpair, double>> LinkMap::get_keypairs(const uint32_t row_key) const {
    std::vector<std::pair<linkpair, double>> res;

    std::for_each(this->map.cbegin(), this->map.cend(), [&res, &row_key](auto element) {
        if (element.first.first.get_id() == row_key) {
            res.emplace_back(element);
        }
    });

    return res;
}
