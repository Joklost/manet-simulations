#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <reachi/clustering.h>
#include <reachi/math.h>
#include <mpilib/helpers.h>

Optics::Optics() = default;

double Optics::core_distance(Node &p) {
    if (!mpilib::is_equal(p.get_core_distance(), UNDEFINED)) {
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

        auto distance = distance_between(p.get_location(), q.second.get_location());

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

        auto reachdist = std::max(coredist, distance_between(p.get_location(), o.get_location()));
        if (mpilib::is_equal(o.get_reachability_distance(), UNDEFINED)) {
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

        if (mpilib::is_equal(this->core_distance(p), UNDEFINED)) {
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

            if (!mpilib::is_equal(this->core_distance(q), UNDEFINED)) {
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

    mpilib::enumerate(ordering.begin(), ordering.end(), 0, [&threshold, &separators](int i, Node &p) {

        double reachdist{};

        if (mpilib::is_equal(p.get_reachability_distance(), UNDEFINED)) {
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
    /*
     * TODO: Create clusters containing single nodes for any node not in a cluster.
     */
    return clusters;
}

mpilib::geo::Location Optics::Cluster::centroid() const {
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

    mpilib::geo::Location l{lat, lon};

    this->_centroid = l;
    this->cached = true;
    return l;
}

unsigned long Optics::Cluster::size() const {
    return this->nodes.size();
}

const std::vector<Node> &Optics::Cluster::get_nodes() const {
    return nodes;
}

double Optics::Cluster::radius() const {
    mpilib::geo::Location centroid = this->centroid();
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
    mpilib::geo::Location centroid = this->centroid();
    auto cost = 0.0;

    for (auto &node : this->nodes) {
        auto distance = distance_between(centroid, node.get_location());
        cost += distance;
    }

    return cost;
}

uint32_t Optics::Cluster::get_id() const {
    return this->id;
}

bool Optics::Cluster::contains(const Node &node) const {
    return std::find(this->nodes.begin(), this->nodes.end(), node) != this->nodes.end();
}

bool Optics::Cluster::operator==(const Optics::Cluster &rhs) const {
    return id == rhs.id;
}

bool Optics::Cluster::operator!=(const Optics::Cluster &rhs) const {
    return !(rhs == *this);
}

uint64_t Optics::CLink::get_id() const {
    return this->id;
}

double Optics::CLink::get_distance() const {
    return distance_between(this->clusters.first.centroid(), this->clusters.second.centroid());
}

const std::pair<Optics::Cluster, Optics::Cluster> &Optics::CLink::get_clusters() const {
    return clusters;
}

Optics::CLink::CLink(uint64_t id, Optics::Cluster &c1, Optics::Cluster &c2) : id(id), clusters(std::make_pair(c1, c2)) {
    this->distance = distance_between(c1.centroid(), c2.centroid());
}

bool Optics::CLink::operator==(const Optics::CLink &rhs) const {
    return id == rhs.id;
}

bool Optics::CLink::operator!=(const Optics::CLink &rhs) const {
    return !(rhs == *this);
}

bool Optics::CLink::operator<(const Optics::CLink &rhs) const {
    return id < rhs.id;
}

bool Optics::CLink::operator>(const Optics::CLink &rhs) const {
    return rhs < *this;
}

bool Optics::CLink::operator<=(const Optics::CLink &rhs) const {
    return !(rhs < *this);
}

bool Optics::CLink::operator>=(const Optics::CLink &rhs) const {
    return !(*this < rhs);
}

bool Optics::CLink::contains(const Node &node) const {
    return this->clusters.first.contains(node) || this->clusters.second.contains(node);
}

bool Optics::CLink::contains(const Node &n1, const Node &n2) const {
    return (this->clusters.first.contains(n1) && this->clusters.second.contains(n2))
           || (this->clusters.first.contains(n2) && this->clusters.second.contains(n1));
}

double Optics::CLink::get_rssi() const {
    return rssi;
}

void Optics::CLink::set_rssi(double rssi) {
    CLink::rssi = rssi;
}

double Optics::CLink::get_pep() const {
    return pep;
}

void Optics::CLink::set_pep(double pep) {
    CLink::pep = pep;
}
