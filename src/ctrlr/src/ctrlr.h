#ifndef MANETSIMS_CTRL_H
#define MANETSIMS_CTRL_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>
#include <mpilib/queue.h>
#include <mpilib/node.h>

enum Type {
    transmit_t, listen_t, sleep_t, set_time_t, update_location_t, poison_t
};


struct ListenAction {
    unsigned long start{};
    unsigned long duration{};
    int rank{};
    bool is_processed{};
};

struct TransmissionAction {
    unsigned long start{};
    unsigned long duration{};
    int rank{};
    std::vector<octet> data;

    bool is_within(ListenAction listen);
};

struct Action {
    Type type;
    int rank;
    unsigned long localtime;
    unsigned long duration;
    std::vector<octet> data;
};

class Controller {
    int lmc_node{};
    std::map<int, mpilib::Node> nodes{};

    std::vector<TransmissionAction> transmission_actions{};
    std::vector<ListenAction> listen_actions{};

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
