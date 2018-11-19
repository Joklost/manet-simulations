#ifndef MANETSIMS_LINKMAP_H
#define MANETSIMS_LINKMAP_H

#include <map>
#include <queue>
#include <mpilib/link.h>


class LinkMap {
public:
    //bool contains(const Link &l1, const Link &l2) const;

    bool contains(const uint64_t &row_key, const uint64_t &column_key) const;

    auto begin() { return this->map.begin(); };

    auto end() { return this->map.end(); };

    auto cbegin() { return this->map.cbegin(); };

    auto cend() { return this->map.cend(); };

    LinkMap operator*(double scalar) const;

    void emplace(const uint64_t &l1, const uint64_t &l2, double value);

    double &operator[](linkpair key);

    double &get(const uint64_t &l1, const uint64_t &l2);

    unsigned long size() const;

    std::vector<uint64_t> get_keypairs(const uint64_t &row_key) const;

private:
    std::map<linkpair, double> map;

    linkpair make_pair(const uint64_t &l1, const uint64_t &l2) const;
};

#endif //MANETSIMS_LINKMAP_H
