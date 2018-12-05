#include <reachi/clustering.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <mpilib/helpers.h>

Optics::Optics() {
    //this->console = spdlog::stdout_color_mt("optics");
}

double Optics::core_distance(Node &p) {
    if (!is_equal(p.get_core_distance(), UNDEFINED)) {
        return p.get_core_distance();
    }

    auto &neighbours = compute_neighbours(p);
    if (neighbours.size() >= this->minpts - 1) {

        /* Sort by distance */
        std::sort(neighbours.begin(), neighbours.end(), [](Neighbour &left, Neighbour &right) {
            return left.distance < right.distance;
        });

        p.set_core_distance(neighbours[this->minpts - 2].distance);
        return p.get_core_distance();
    }

    return UNDEFINED;
}

std::vector<Optics::Neighbour> &Optics::compute_neighbours(Node &p) {
    if (this->neighbourhoods.find(p) != this->neighbourhoods.end()) {
        //this->console->info("Neighbourhood for {} found in cache", p.get_id());
        return this->neighbourhoods[p];
    }

    this->neighbourhoods[p] = std::vector<Optics::Neighbour>{};

    for (auto &q : graph) {
        if (p == q.second) {
            continue;
        }

        auto distance = distance_between(p, q.second);

        if (distance <= this->eps) {
            Neighbour n{q.second.get_id(), distance};
            this->neighbourhoods[p].push_back(n);
        }
    }

    return this->neighbourhoods[p];
}

void Optics::update_seeds(Node &p, std::vector<uint32_t> &seeds) {
    auto &p_neighbours = this->compute_neighbours(p);
    auto coredist = this->core_distance(p);

    for (Neighbour &neighbour : p_neighbours) {

        Node &o = this->graph[neighbour.node];
        if (o.is_processed()) {
            continue;
        }

        auto reachdist = std::max(coredist, distance_between(p, o));
        if (is_equal(o.get_reachability_distance(), UNDEFINED)) {
            /* 'o' is not in seeds */
            o.set_reachability_distance(reachdist);
            seeds.emplace_back(o.get_id());
        } else {
            /* 'o' is already in seeds, check for improvement */
            if (reachdist < o.get_reachability_distance()) {
                o.set_reachability_distance(reachdist);
            }
        }
    }
}

std::vector<Node> Optics::compute_ordering(std::vector<Node> &nodes, double eps, int minpts) {
    this->graph.clear();
    this->unprocessed.clear();
    this->ordered.clear();
    this->neighbourhoods.clear();

    for (auto &node : nodes) {
        this->graph.insert(std::make_pair(node.get_id(), node));
        //this->graph[node.get_id()] = node;
        this->unprocessed.emplace_back(node.get_id());
    }

    this->eps = eps;
    this->minpts = minpts;

    for (auto &p : this->graph) {
        p.second.set_reachability_distance(UNDEFINED);
        p.second.set_processed(false);
    }

    while (!this->unprocessed.empty()) {
        auto &p = this->graph[this->unprocessed.front()];
        this->processed(p);

        //auto p_neighbours = this->compute_neighbours(p);

        if (is_equal(this->core_distance(p), UNDEFINED)) {
            continue;
        }

        std::vector<uint32_t> seeds{};
        update_seeds(p, seeds);

        while (!seeds.empty()) {
            std::sort(seeds.begin(), seeds.end(), [this](const int left, const int right) -> bool {
                return this->graph[left].get_reachability_distance() > this->graph[right].get_reachability_distance();
            });
            auto &q = this->graph[seeds.back()];
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
    this->unprocessed.erase(std::remove(this->unprocessed.begin(), this->unprocessed.end(), p.get_id()),
                            this->unprocessed.end());
    Node n(p); // copy
    this->ordered.push_back(n);
}

std::vector<Optics::Cluster> Optics::cluster(std::vector<Node> &ordering) {
    return this->cluster(ordering, this->eps - 0.01);
}

std::vector<Optics::Cluster> Optics::cluster(std::vector<Node> &ordering, double threshold) {
    std::vector<Optics::Cluster> clusters{};
    std::vector<int> separators{};

    enumerate(ordering.begin(), ordering.end(), 0, [&threshold, &separators](int i, Node &p) {

        double reachdist{};

        if (is_equal(p.get_reachability_distance(), UNDEFINED)) {
            reachdist = std::numeric_limits<double>::infinity();
        } else {
            reachdist = p.get_reachability_distance();
        }

        if (reachdist > threshold) {
            separators.emplace_back(i);
        }
    });

    separators.emplace_back(ordering.size());

    for (uint32_t j = 0; j < (separators.size() - 1); ++j) {
        auto start = separators[j];
        auto end = separators[j + 1];
        if (end - start >= this->minpts) {
            std::vector<Node> cluster{ordering.begin() + start, ordering.begin() + end};
            Cluster c{j, cluster};
            clusters.emplace_back(c);
        }
    }

    return clusters;
}

Location Optics::Cluster::centroid() const {
    auto lat = 0.0;
    auto lon = 0.0;
    for (auto &node : this->nodes) {
        lat += node.get_location().get_latitude();
        lon += node.get_location().get_longitude();
    }

    lat = lat / this->nodes.size();
    lon = lon / this->nodes.size();

    Location l{lat, lon};

    return l;
}

unsigned long Optics::Cluster::size() const {
    return this->nodes.size();
}

const std::vector<Node> &Optics::Cluster::get_nodes() const {
    return nodes;
}

double Optics::Cluster::radius() const {
    Location centroid = this->centroid();
    auto radius = 0.0;

    for (auto &node : this->nodes) {
        auto distance = distance_between(centroid, node.get_location());
        if (distance > radius) {
            radius = distance;
        }
    }

    return radius;
}

double Optics::Cluster::cost() const {
    Location centroid = this->centroid();
    auto cost = 0.0;

    for (auto &node : this->nodes) {
        auto distance = distance_between(centroid, node.get_location());
        cost += distance;
    }

    return cost;
}
