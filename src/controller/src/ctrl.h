#ifndef MANETSIMS_CTRL_H
#define MANETSIMS_CTRL_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>

enum States {
    transmitting, listening, sleeping
};

struct Packet {
    unsigned long time{};
    std::vector<octet> *data{};
};

struct Node {
    int rank{};
    Location loc{};
    unsigned long time{};

    States state = sleeping;
    std::vector<std::vector<octet>> packets{};
};

class Controller {
    std::mutex mutex_;
    std::condition_variable cond_;

    bool work = true;
    int dies = 1;
    unsigned long current_time = 0;

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;

    std::map<int, Node> nodes{};
    std::map<int, std::vector<Packet>> packets{};

    void message_handler();

    void clock();

    bool timeslot_complete(unsigned long time);

    bool handshake();

    void handle_tx(const Status &status);

    void handle_rx(const Status &status);

    void handle_sleep(const Status &status);

    void update_location(const Status &status);

    void die(Status &status);

public:
    void run();
};

#endif //MANETSIMS_CTRL_H
