#ifndef MANETSIMS_LINKMODEL_INTERFACE_H
#define MANETSIMS_LINKMODEL_INTERFACE_H

#include <vector>
#include <map>

#include <mpilib/mpi.h>
#include <mpilib/queue.h>
#include <mpilib/node.h>
#include <mpilib/link.h>
#include <http/httpclient.h>

#include <reachi/node.h>
#include <reachi/link.h>

using json = nlohmann::json;

enum Type {
    update_location_t, link_model_t, poison_t
};

struct Action {
    Type type;
    int rank;

    std::vector<octet> data;

    std::vector<mpilib::Link> link_model;
};


class LinkModelComputer {
    std::mutex mutex_;
    std::condition_variable cond_;

    std::map<int, mpilib::Node> nodes{};

    bool debug = false;
    bool work = true;
    mpilib::Queue<Action> queue{};

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;

    http::HttpClient httpclient;
    std::string uuid{};

    bool is_valid = false;

    std::vector<mpilib::Link> link_model{};

    bool handshake();

    void recv();

    void control();

    void compute();

    Action compute_link_model();

public:

    explicit LinkModelComputer(bool debug) : debug(debug), httpclient("http://0.0.0.0:5000") {}

    void run();
};

namespace reachi {
    void to_json(json &j, const reachi::Node &p);

    void from_json(const json &j, reachi::Node &p);

    void to_json(json &j, const reachi::Link &p);

    void from_json(const json &j, reachi::Link &p);
}

#endif /* MANETSIMS_LINKMODEL_INTERFACE_H */
