#include <cassert>
#include <algorithm>

#include <sims/clustering.h>
#include <sims/math.h>

#include <common/equality.h>
#include <common/iters.h>

sims::Optics::Optics() = default;

double sims::Optics::core_distance(Node &p) {
    if (!common::is_equal(p.get_core_distance(), sims::UNDEFINED)) {
        return p.get_core_distance();
    }

    auto &neighbours = this->compute_neighbours(p);
    if (neighbours.size() >= this->minpts - 1) {

        /* Sort by distance */
        std::sort(neighbours.begin(), neighbours.end(), [](Neighbour &left, Neighbour &right) {
            return left.distance < right.distance;
        });

        p.set_core_distance(neighbours[this->minpts - 2].distance);
        return p.get_core_distance();
    }

    return sims::UNDEFINED;
}

std::vector<sims::Optics::Neighbour> &sims::Optics::compute_neighbours(Node &p) {
    if (this->neighbourhoods.find(p) != this->neighbourhoods.end()) {
        //this->console->info("Neighbourhood for {} found in cache", p.get_id());
        return this->neighbourhoods[p];
    }

    this->neighbourhoods[p] = std::vector<sims::Optics::Neighbour>{};

    for (auto &q : graph) {
        if (p == q.second) {
            continue;
        }

        auto distance = distance_between(p.get_location(), q.second.get_location());

        if (distance <= this->eps) {
            Neighbour n{q.second.get_id(), distance};
            this->neighbourhoods[p].push_back(n);
        }
    }

    return this->neighbourhoods[p];
}

void sims::Optics::update_seeds(Node &p, std::vector<uint32_t> &seeds) {
    auto &p_neighbours = this->compute_neighbours(p);
    auto coredist = this->core_distance(p);

    for (Neighbour &neighbour : p_neighbours) {

        Node &o = this->graph[neighbour.node];
        if (o.is_processed()) {
            continue;
        }

        auto reachdist = std::max(coredist, distance_between(p.get_location(), o.get_location()));
        if (common::is_equal(o.get_reachability_distance(), sims::UNDEFINED)) {
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

std::vector<sims::Node> sims::Optics::compute_ordering(std::vector<sims::Node> &nodes, double eps, int minpts) {
    this->graph.clear();
    this->unprocessed.clear();
    this->ordered.clear();
    this->neighbourhoods.clear();

    for (auto &node : nodes) {
        this->graph.insert(std::make_pair(node.get_id(), node));
        this->unprocessed.emplace_back(node.get_id());
    }

    this->eps = eps;
    this->minpts = minpts;

    for (auto &p : this->graph) {
        p.second.set_reachability_distance(sims::UNDEFINED);
        p.second.set_core_distance(sims::UNDEFINED);
        p.second.set_processed(false);
    }

    while (!this->unprocessed.empty()) {
        auto &p = this->graph[this->unprocessed.front()];
        this->processed(p);

        if (common::is_equal(this->core_distance(p), sims::UNDEFINED)) {
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

            if (!common::is_equal(this->core_distance(q), sims::UNDEFINED)) {
                update_seeds(q, seeds);
            }
        }
    }

    return this->ordered;
}

void sims::Optics::processed(Node &p) {
    p.set_processed(true);
    this->unprocessed.erase(std::remove(this->unprocessed.begin(), this->unprocessed.end(), p.get_id()),
                            this->unprocessed.end());
    Node n(p); // copy
    this->ordered.push_back(n);
}

std::vector<sims::Optics::Cluster> sims::Optics::cluster(std::vector<Node> &ordering) {
    return this->cluster(ordering, this->eps - 0.01);
}

std::vector<sims::Optics::Cluster> sims::Optics::cluster(std::vector<Node> &ordering, double threshold) {
    std::vector<sims::Optics::Cluster> clusters{};
    std::vector<int> separators{};

    common::enumerate(ordering.begin(), ordering.end(), 0, [&threshold, &separators](int i, Node &p) {

        double reachdist{};

        if (common::is_equal(p.get_reachability_distance(), sims::UNDEFINED)) {
            reachdist = std::numeric_limits<double>::infinity();
        } else {
            reachdist = p.get_reachability_distance();
        }

        if (reachdist > threshold) {
            separators.emplace_back(i);
        }
    });

    separators.emplace_back(ordering.size());

    uint32_t id = 0;
    for (int j = 0; j < (separators.size() - 1); ++j) {
        auto start = separators[j];
        auto end = separators[j + 1];
        if (end - start >= this->minpts) {
            std::vector<Node> cluster{ordering.begin() + start, ordering.begin() + end};
            Cluster c{id++, cluster};
            clusters.emplace_back(c);
        }
    }

    clusterize_remaining(ordering, clusters);

    return clusters;
}


void sims::Optics::clusterize_remaining(const std::vector<sims::Node> &nodes, std::vector<sims::Optics::Cluster> &clusters) {
    auto cnodes = nodes;
    unsigned long node_count = 0;
    for (auto &cluster : clusters) {
        node_count += cluster.size();

        for (auto &node : cluster.get_nodes()) {
            if (cluster.contains(node)) {
                cnodes.erase(std::remove(cnodes.begin(), cnodes.end(), node), cnodes.end());
            }
        }
    }
    node_count = nodes.size() - node_count + clusters.size();
    auto id = static_cast<uint32_t>(clusters.size());
    for (auto &node : cnodes) {
        id++;
        std::vector<Node> single_node_cluster{node};
        Optics::Cluster c{id, single_node_cluster};
        clusters.push_back(c);
    }

    assert(node_count == clusters.size());
}

geo::Location sims::Optics::Cluster::centroid() const {
    if (this->nodes.size() == 1) {
        return this->nodes.front().get_location();
    }

    if (this->cached) {
        return this->_centroid;
    }

    auto lat = 0.0;
    auto lon = 0.0;
    for (auto &node : this->nodes) {
        lat += node.get_location().get_latitude();
        lon += node.get_location().get_longitude();
    }

    lat = lat / this->nodes.size();
    lon = lon / this->nodes.size();

    geo::Location l{lat, lon};

    this->_centroid = l;
    this->cached = true;
    return l;
}

unsigned long sims::Optics::Cluster::size() const {
    return this->nodes.size();
}

const std::vector<sims::Node> &sims::Optics::Cluster::get_nodes() const {
    return nodes;
}

double sims::Optics::Cluster::radius() const {
    geo::Location centroid = this->centroid();
    auto radius = 0.0;

    for (auto &node : this->nodes) {
        auto distance = distance_between(centroid, node.get_location());
        if (distance > radius) {
            radius = distance;
        }
    }

    return radius;
}

double sims::Optics::Cluster::cost() const {
    geo::Location centroid = this->centroid();
    auto cost = 0.0;

    for (auto &node : this->nodes) {
        auto distance = geo::distance_between(centroid, node.get_location());
        cost += distance;
    }

    return cost;
}

uint32_t sims::Optics::Cluster::get_id() const {
    return this->id;
}

bool sims::Optics::Cluster::contains(const Node &node) const {
    return std::find(this->nodes.begin(), this->nodes.end(), node) != this->nodes.end();
}

bool sims::Optics::Cluster::operator==(const sims::Optics::Cluster &rhs) const {
    return id == rhs.id;
}

bool sims::Optics::Cluster::operator!=(const sims::Optics::Cluster &rhs) const {
    return !(rhs == *this);
}

uint64_t sims::Optics::CLink::get_id() const {
    return this->id;
}

double sims::Optics::CLink::get_distance() const {
    return distance_between(this->clusters.first.centroid(), this->clusters.second.centroid());
}

const std::pair<sims::Optics::Cluster, sims::Optics::Cluster> &sims::Optics::CLink::get_clusters() const {
    return clusters;
}

sims::Optics::CLink::CLink(uint64_t id, sims::Optics::Cluster &c1, sims::Optics::Cluster &c2) : id(id), clusters(
        std::make_pair(c1, c2)) {
    this->distance = distance_between(c1.centroid(), c2.centroid());
}

bool sims::Optics::CLink::operator==(const sims::Optics::CLink &rhs) const {
    return id == rhs.id;
}

bool sims::Optics::CLink::operator!=(const sims::Optics::CLink &rhs) const {
    return !(rhs == *this);
}

bool sims::Optics::CLink::operator<(const sims::Optics::CLink &rhs) const {
    return id < rhs.id;
}

bool sims::Optics::CLink::operator>(const sims::Optics::CLink &rhs) const {
    return rhs < *this;
}

bool sims::Optics::CLink::operator<=(const sims::Optics::CLink &rhs) const {
    return !(rhs < *this);
}

bool sims::Optics::CLink::operator>=(const sims::Optics::CLink &rhs) const {
    return !(*this < rhs);
}

bool sims::Optics::CLink::contains(const Node &node) const {
    return this->clusters.first.contains(node) || this->clusters.second.contains(node);
}

bool sims::Optics::CLink::contains(const Node &n1, const Node &n2) const {
    return (this->clusters.first.contains(n1) && this->clusters.second.contains(n2))
           || (this->clusters.first.contains(n2) && this->clusters.second.contains(n1));
}

double sims::Optics::CLink::get_rssi() const {
    return rssi;
}

void sims::Optics::CLink::set_rssi(double rssi) {
    CLink::rssi = rssi;
}

double sims::Optics::CLink::get_pep() const {
    return pep;
}

void sims::Optics::CLink::set_pep(double pep) {
    CLink::pep = pep;
}
