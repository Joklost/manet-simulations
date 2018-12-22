#ifndef MANETSIMS_CTRL_H
#define MANETSIMS_CTRL_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>
#include <mpilib/queue.h>

enum Action {
    transmit_t, listen_t, sleep_t, update_location_t, set_time_t, poison_t
};

enum States {
    transmitting, listening, sleeping
};

struct Transmission {
    unsigned long start{};
    unsigned long duration{};
    unsigned long source{};
    std::vector<octet> data;
};

struct Message {
    Action action;
    int rank;
    unsigned long localtime;
    unsigned long duration;
    std::vector<octet> data;
};

struct Packet {
    unsigned long time{};
    std::vector<octet> *data{};
};

class Node {
public:
    int rank{};
    std::string name{};
    mpilib::geo::Location loc{};
    unsigned long localtime{};
    bool dead{};

    States state = sleeping;
    std::vector<std::vector<octet>> packets{};

    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;
};

class Controller {
    std::mutex mutex_;
    std::condition_variable cond_;

    mpilib::Queue<Message> queue{};
    mpilib::Queue<Transmission> transmissions{};

    bool debug = false;
    bool work = true;
    int dies = 1;
    unsigned long current_time = 0;

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;

    std::map<int, Node> nodes{};
    std::map<int, Packet> packets{};


    void message_handler();

    void clock();

    void recv();

    void control();

    bool timeslot_complete(unsigned long time);

    bool handshake();

    void update_location(const mpi::Status &status);

public:
    explicit Controller(bool debug) : debug(debug) {}

    void run();
};

#endif //MANETSIMS_CTRL_H
