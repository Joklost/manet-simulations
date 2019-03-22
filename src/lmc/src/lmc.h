#ifndef LINKMODEL_COMPUTER_H
#define LINKMODEL_COMPUTER_H

#include <vector>
#include <map>

#include <common/queue.h>
#include <mpilib/mpi.h>
#include <mpilib/node.h>
#include <mpilib/link.h>

#include <sims/node.h>
#include <sims/link.h>

#ifdef INSTALL_HTTP
#include <http/httpclient.h>

using json = nlohmann::json;
#endif /* INSTALL_HTTP */


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
    common::Queue<Action> queue{};

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;

#ifdef INSTALL_HTTP
    http::HttpClient httpclient;
    std::string uuid{};
#endif /* INSTALL_HTTP */

    bool is_valid = false;

    std::vector<mpilib::Link> link_model{};

    bool handshake();

    void recv();

    void control();

    void compute();

    Action compute_link_model();

public:

#ifdef INSTALL_HTTP
    explicit LinkModelComputer(bool debug) : debug(debug), httpclient("http://0.0.0.0:5000") {}
#else
    explicit LinkModelComputer(bool debug) : debug(debug) {}
#endif /* INSTALL_HTTP */

    void run();
};

#ifdef INSTALL_HTTP
namespace sims {
    void to_json(json &j, const sims::Node &p);

    void from_json(const json &j, sims::Node &p);

    void to_json(json &j, const sims::Link &p);

    void from_json(const json &j, sims::Link &p);
}
#endif /* INSTALL_HTTP */

#endif /* LINKMODEL_COMPUTER_H */
