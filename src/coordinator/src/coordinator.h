#ifndef MANETSIMS_COORDINATOR_H
#define MANETSIMS_COORDINATOR_H

#include <random>
#include <vector>
#include <map>

#include <common/queue.h>
#include <mpilib/mpi.h>
#include <mpilib/link.h>
#include <mpilib/node.h>

const int CSUCCESS = 0;
const int CERROR = -1;

class Coordinator {
    enum Type {
        Listen, Transmission, Sleep, Inform
    };

    /* Debug */
    bool debug = false;
    std::shared_ptr<spdlog::logger> c;

    /* MPI */
    int lmc_node{};
    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    int dead_nodes{};

    struct Packet {
        unsigned long time{0ul};
        std::vector<octet> data{};

        bool operator==(const Packet &rhs) const;

        bool operator!=(const Packet &rhs) const;
    };

    /* Model */
    struct Action {
        Type type{};
        int rank{};
        unsigned long start{};
        unsigned long end{};

        Packet packet{};

        bool is_within(const Action &action) const;
    };

    static bool action_cmp(const Action &left, const Action &right) {
        return left.end > right.end;
        /* TODO: order by type i < s < t < l. */
    }

    std::map<int, std::vector<Action> > transmissions{};
    common::PriorityQueue<Action, std::vector<Action>, decltype(&action_cmp)> action_queue{action_cmp};

    std::map<int, mpilib::Node> nodes{};
    std::vector<std::vector<double>> link_model;

    /* Helpers */

    /**
     * Handshake with all participating nodes and the LMC node.
     * @return bool if succesful.
     */
    bool handshake();

    /**
     * Receive updated location for a node.
     * @param rank The node's rank.
     * @return The updated location of the node.
     */
    geo::Location update_location(int rank);

    void set_linkmodel(std::vector<mpilib::Link> &links);

    int process_message(const mpi::Status &status);

    void process_queue(std::mt19937 &gen);

public:
    explicit Coordinator(bool debug) : debug(debug) {}

    /**
     * Run the Coordinator.
     */
    void run();
};


#endif /* MANETSIMS_COORDINATOR_H */
