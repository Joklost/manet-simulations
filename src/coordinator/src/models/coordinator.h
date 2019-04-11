#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <random>
#include <vector>
#include <map>

#include <common/queue.h>
#include <common/equality.h>
#include <mpilib/mpi.h>

#include "node.h"
#include "action.h"
#include "topology.h"

const int CSUCCESS = 0;
const int CERROR = -1;

class Coordinator {
    /* Debug */
    bool debug = false;
    std::shared_ptr<spdlog::logger> c;

    /* MPI */
    unsigned long world_size{};
    unsigned long world_rank{};
    std::string processor_name{};

    /* Model */
    unsigned long dead_nodes{};
    std::map<unsigned long, Node> nodes{};
    std::map<double, Topology, common::is_less<double>> topologies{};
    std::map<unsigned long, std::vector<Action>> transmissions{};
    common::PriorityQueue<Action, std::vector<Action>, decltype(&compare_actions)> action_queue{compare_actions};

    /* Helpers */
    bool handshake();

    int enqueue_message(const mpi::Status &status);

    void process_actions(std::mt19937 &gen);

public:
    Coordinator(const char *logpath, bool debug);

    /**
     * Run the Coordinator.
     */
    void run();
};


#endif /* COORDINATOR_H */
