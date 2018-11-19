#include <reachi/clustering.h>


bool cmp_reachability(const Node &left, const Node &right) {
    return left.get_reachability_distance() > right.get_reachability_distance();
}

double core_distance(neighbourhood_t &neighbours, Node &node, double eps, int minpts) {
    if (!is_equal(node.get_core_distance(), UNDEFINED)) {
        return node.get_core_distance();
    }

    if (neighbours.size() >= minpts - 1) {
        /* Sort by distance */
        std::sort(neighbours.begin(), neighbours.end(), [](neighbour_t left, neighbour_t right) {
            return left.second > right.second;
        });

        /* -2? */
        auto coredist = neighbours[minpts - 2].second;
        node.set_core_distance(coredist);
        return coredist;
    }

    return UNDEFINED;
}

neighbourhood_t get_neighbours(std::vector<Node> &nodes, Node &p, double eps) {
    neighbourhood_t neighbours{};

    std::for_each(nodes.begin(), nodes.end(), [&p, &eps, &neighbours](Node &n) {
        if (p == n) {
            return;
        }

        auto distance = distance_between(p, n);

        //console->info("Distance between {} and {}: {}", p.get_id(), n.get_id(), distance);
        if (distance <= eps) {
            neighbours.emplace_back(std::make_pair(std::ref(n), distance));
        }
    });

    return neighbours;
}

void update(neighbourhood_t &neighbours, Node &p, std::vector<Node> &seeds, double eps, int minpts) {
    auto coredist = core_distance(neighbours, p, eps, minpts);

    std::for_each(neighbours.begin(), neighbours.end(), [&p, &seeds, &coredist](neighbour_t &pair) {
        auto o = pair.first;
        if (o.is_processed()) {
            return;
        }

        auto new_reachdist = std::max(coredist, distance_between(p, o));
        if (is_equal(o.get_reachability_distance(), UNDEFINED)) {
            /* o is not in seeds */
            o.set_reachability_distance(new_reachdist);
            seeds.emplace_back(o);
        } else {
            /* o is in seeds, check for improvement */
            if (new_reachdist < o.get_reachability_distance()) {
                o.set_reachability_distance(new_reachdist);
            }
        }
    });
}

std::vector<Node> optics(std::vector<Node> &nodes, double eps, int minpts) {
    std::vector<Node> ordered_list{};

    std::for_each(nodes.begin(), nodes.end(), [](Node node) {
        node.set_reachability_distance(UNDEFINED);
    });

    std::for_each(nodes.begin(), nodes.end(), [&nodes, &eps, &minpts, &ordered_list](Node p) {
        if (p.is_processed()) {
            return;
        }

        auto pneighbours = get_neighbours(nodes, p, eps);

        p.set_processed(true);

        ordered_list.push_back(p);

        if (is_equal(core_distance(pneighbours, p, eps, minpts), UNDEFINED)) {
            return;
        }

        std::vector<Node> seeds{};
        update(pneighbours, p, seeds, eps, minpts);

        while (!seeds.empty()) {
            //console->info("Seeds: {}", seeds.size());
            std::sort(seeds.begin(), seeds.end(), cmp_reachability);
            auto q = seeds.back();
            seeds.pop_back();

            auto qneighbours = get_neighbours(nodes, q, eps);

            q.set_processed(true);

            ordered_list.push_back(q);
            if (!is_equal(core_distance(qneighbours, q, eps, minpts), UNDEFINED)) {
                update(qneighbours, q, seeds, eps, minpts);
            }
        }
    });

    return ordered_list;
}

Optics::Optics() {
    this->console = spdlog::stdout_color_mt("optics");
}

double Optics::core_distance(Node &p) {
    if (!is_equal(p.get_core_distance(), UNDEFINED)) {
        return p.get_core_distance();
    }

    auto &neighbours = compute_neighbours(p);
    if (neighbours.size() >= this->minpts - 1) {

        /* Sort by distance */
        std::sort(neighbours.begin(), neighbours.end(), [](Neighbour &left, Neighbour &right) {
            return left.distance > right.distance;
        });

        p.set_core_distance(neighbours[this->minpts - 2].distance);
        return p.get_core_distance();
    }

    return UNDEFINED;
}

std::vector<Optics::Neighbour> &Optics::compute_neighbours(Node &p) {
    if (this->neighbourhoods.find(p) != this->neighbourhoods.end()) {
        this->console->info("Neighbourhood for {} found in cache", p.get_id());
        return this->neighbourhoods[p];
    }

    this->neighbourhoods[p] = std::vector<Optics::Neighbour>{};

    for (auto &q : graph) {
        if (p == q) {
            continue;
        }

        auto distance = distance_between(p, q);

        if (distance <= this->eps) {
            Neighbour n{q, distance};
            this->neighbourhoods[p].push_back(n);
        }
    }

    return this->neighbourhoods[p];
}

void Optics::update_seeds(Node &p, std::vector<Node> &seeds) {
    auto &p_neighbours = this->compute_neighbours(p);
    auto coredist = this->core_distance(p);

    for (auto &neighbour : p_neighbours) {
        if (neighbour.node.is_processed()) {
            continue;
        }

        auto &o = neighbour.node;
        auto reachdist = std::max(coredist, distance_between(p, o));
        if (is_equal(o.get_reachability_distance(), UNDEFINED)) {
            /* 'o' is not in seeds */
            o.set_reachability_distance(reachdist);
            seeds.push_back(o);
        } else {
            /* 'o' is already in seeds, check for improvement */
            if (reachdist < o.get_reachability_distance()) {
                o.set_reachability_distance(reachdist);
            }
        }
    }
}

std::vector<Node> Optics::compute_clusters(std::vector<Node> &nodes, const double eps, const int minpts) {
    this->graph = std::vector<Node>(nodes);
    this->unprocessed = std::vector<Node>(nodes);
    this->eps = eps;
    this->minpts = minpts;

    for (auto &p : this->graph) {
        p.set_reachability_distance(UNDEFINED);
        p.set_processed(false);
    }

    while (!this->unprocessed.empty()) {
        auto &p = this->unprocessed.front();

        this->processed(p);

        if (is_equal(this->core_distance(p), UNDEFINED)) {
            continue;
        }

        std::vector<Node> seeds{};
        update_seeds(p, seeds);

        while (!seeds.empty()) {
            std::sort(seeds.begin(), seeds.end(), cmp_reachability);
            auto &q = seeds.back();
            seeds.pop_back();

            this->processed(q);

            if (!is_equal(this->core_distance(q), UNDEFINED)) {
                update_seeds(q, seeds);
            }
        }
    }

    return this->ordered;
}

void Optics::processed(Node &p) {
    p.set_processed(true);
    this->unprocessed.erase(std::remove(this->unprocessed.begin(), this->unprocessed.end(), p),
                            this->unprocessed.end());
    this->ordered.push_back(p);
}

