#ifndef MANETSIMS_COORDR_H
#define MANETSIMS_COORDR_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>
#include <mpilib/queue.h>
#include <mpilib/link.h>
#include <mpilib/node.h>


class Coordinator {
    enum Type {
        Listen, Transmission, NoOp, Inform
    };

    class Action;

    /* Debug */
    bool debug = false;
    std::shared_ptr<spdlog::logger> c;

    /* MPI */
    int lmc_node{};
    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};

    /* Model */
    struct Action {
        Type type{};
        int rank{};
        unsigned long start{};
        unsigned long end{};

        std::vector<std::vector<octet>> packets{};

        bool is_within(const Action &action) const;
    };

    static bool cmp(const Action &left, const Action &right) {
        return left.end > right.end;
    }

    std::vector<Action> transmissions{};
    mpilib::PriorityQueue<Action, std::vector<Action>, decltype(&cmp)> action_queue{cmp};

    /*  struct Action {
          unsigned long start{};
          unsigned long end{};
          int rank{};

      };
  */
    /*struct Transmission;

    struct Listen : Action {
        bool processed{};
        std::vector<Transmission> transmissions{};
    };

    struct Transmission : Action {
        std::vector<octet> data;

        bool is_within(Listen &listen);
    };

    std::vector<Transmission> transmit_actions{};
    std::vector<Listen> listen_actions{};
*/
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
    //bool has_link(Listen &rx, Transmission &tx);

public:
    explicit Coordinator(bool debug) : debug(debug) {}

    /**
     * Run the Coordinator.
     */
    void run();
};


#endif //MANETSIMS_COORDR_H
