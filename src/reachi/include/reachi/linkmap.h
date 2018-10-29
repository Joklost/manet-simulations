#ifndef MANETSIMS_LINKMAP_H
#define MANETSIMS_LINKMAP_H

#include <map>
#include <queue>
#include <mpilib/link.h>


class LinkMap {
public:
    bool contains(const Link &l1, const Link &l2) const;

    auto begin() { return this->map.begin(); };

    auto end() { return this->map.end(); };

    auto cbegin() { return this->map.cbegin(); };

    auto cend() { return this->map.cend(); };

    LinkMap operator*(double scalar) const;

    void emplace(const Link &l1, const Link &l2, double value);

    double &operator[](linkpair key);

    double &get(const Link &l1, const Link &l2);

    unsigned long size() const;

    std::vector<std::pair<linkpair, double>> get_keypairs(const Link &l, uint32_t upper_limit) const;

    std::vector<std::pair<linkpair, double>> get_keypairs(uint32_t row_key) const;

private:
    std::map<linkpair, double> map;

    linkpair make_pair(const Link &l1, const Link &l2) const;
};

#endif //MANETSIMS_LINKMAP_H
