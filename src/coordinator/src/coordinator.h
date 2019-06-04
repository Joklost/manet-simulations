#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <random>
#include <vector>
#include <map>
#include <set>

#include <common/equality.h>
#include <mpilib/mpi.h>

#include "models/node.h"
#include "models/action.h"
#include "models/topology.h"
#include "models/statistics.h"

const int CSUCCESS = 0;
const int CERROR = -1;
const double TIME_GAP = 20000.0;

class Coordinator {
    /* Debug */
    bool debug = false;
    bool plots = false;

    Statistics stats{};

    /* MPI */
    unsigned long world_size{};
    unsigned long world_rank{};
    std::string processor_name{};

    /* Model */
    unsigned long dead_nodes{};
    std::map<unsigned long, Node> nodes{};
    std::vector<Node> nodelist{};
    std::map<double, Topology, common::is_less<double>> topologies{};
    std::vector<Action> transmissions{};
    //common::PriorityQueue<Action, std::vector<Action>, decltype(&compare_actions)> action_queue{compare_actions};

    std::set<Action, decltype(&compare_actions)> actions{compare_actions};

    /* Helpers */
    bool handshake();

    int enqueue_message(const mpi::Status &status);

    void process_actions(std::mt19937 &gen);

    Topology &get_topology(unsigned long time);

    Topology &get_topology(double time);

public:
    explicit Coordinator(const char *logpath, bool debug = false, bool plots = false);

    void run();

    std::shared_ptr<spdlog::logger> c;
};


#endif /* COORDINATOR_H */
