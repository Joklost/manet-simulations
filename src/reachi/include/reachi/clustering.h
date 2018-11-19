#ifndef MANETSIMS_CLUSTERING_H
#define MANETSIMS_CLUSTERING_H

#include <vector>
#include <limits>
#include <cmath>
#include <queue>
#include <mpilib/node.h>
#include <spdlog/spdlog.h>

#include "math.h"

using neighbour_t = std::pair<Node &, double>;
using neighbourhood_t = std::vector<neighbour_t>;


class Optics {

public:
    Optics();

    std::vector<Node> compute_clusters(std::vector<Node> &nodes, double eps /* kilometers */, int minpts);

private:
    struct Neighbour {
        Node node;
        double distance{};
    };

    void processed(Node &p);

    double core_distance(Node &p);

    std::vector<Neighbour> &compute_neighbours(Node &p);

    void update_seeds(Node &p, std::vector<Node> &seeds);

    double eps;
    int minpts;

    std::vector<Node> graph{};
    std::vector<Node> unprocessed{};
    std::vector<Node> ordered{};
    std::unordered_map<Node, std::vector<Neighbour>> neighbourhoods{};
    std::shared_ptr<spdlog::logger> console;
};


bool cmp_reachability(const Node &left, const Node &right);

double core_distance(neighbourhood_t &neighbours, Node &node, double eps, int minpts);

neighbourhood_t get_neighbours(std::vector<Node> &nodes, Node &p, double eps);

void update(neighbourhood_t &neighbours, Node &p, std::vector<Node> &seeds, double eps, int minpts);

std::vector<Node> optics(std::vector<Node> &nodes, double eps /* kilometers */, int minpts);

#endif /* MANETSIMS_CLUSTERING_H */
