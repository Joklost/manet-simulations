#ifndef MANETSIMS_CTRL_H
#define MANETSIMS_CTRL_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>
#include <mpilib/queue.h>
#include <mpilib/node.h>
#include <mpilib/link.h>

enum Type {
    transmit_t, listen_t, sleep_t, set_time_t, update_location_t, link_model_t, poison_t
};


struct Listen {
    unsigned long start{};
    unsigned long duration{};
    int rank{};
    bool is_processed{};
};

struct Transmission {
    unsigned long start{};
    unsigned long duration{};
    int rank{};
    std::vector<octet> data;

    bool is_within(Listen listen);
};

struct Action {
    Type type;
    int rank;
    unsigned long localtime;
    unsigned long duration;
    std::vector<octet> data;

    std::vector<mpilib::Link> link_model;
};

class Controller {
    int lmc_node{};
    std::map<int, mpilib::Node> nodes{};
    std::vector<mpilib::Link> link_model;

    std::vector<Transmission> transmission_actions{};
    std::vector<Listen> listen_actions{};

    mpilib::Queue<Action> queue{};
    bool debug = false;
    bool work = true;

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;

    void recv();

    void control();

    bool handshake();

    mpilib::geo::Location update_location(const mpi::Status &status);

public:
    explicit Controller(bool debug) : debug(debug) {}

    void run();

};

#endif //MANETSIMS_CTRL_H
