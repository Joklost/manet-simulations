#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include <geo/geo.h>
#include <common/strings.h>

struct Node {
    unsigned long id{};
    std::vector<geo::Location> location_history{};

    bool operator==(const Node &rhs) const {
        return id == rhs.id;
    }

    bool operator!=(const Node &rhs) const {
        return !(rhs == *this);
    }
};

using NodeMap = std::unordered_map<unsigned long, Node>;

struct LinkModel {
    int num_nodes{};
    int nchans{};

    NodeMap nodes{};
};


NodeMap parse_gpsfile(char *gpslog) {
    NodeMap nodes{};

    std::ifstream logfile{gpslog};

    if (logfile.is_open()) {
        while (logfile.good()) {
            std::string line;
            std::getline(logfile, line);

            if (line.empty()) {
                continue;
            }

            auto tokens = common::split(line, ",");
            auto id = std::stoul(tokens[0]);
            auto latitude = std::stod(tokens[1]);
            auto longitude = std::stod(tokens[2]);
            auto timestamp = std::stoul(tokens[3]);

            auto &node = nodes[id]; /* operator[] implicitly constructs new object */
            node.id = id;
            node.location_history.emplace_back(timestamp, latitude, longitude);
        }

        logfile.close();
    } else {
        throw std::runtime_error("failed to open gpslog file");
    }

    return nodes;
}

#ifdef __cplusplus
extern "C" {
#endif

void *initialize(int num_nodes, int nchans, char *gpslog) {
    if (!gpslog || num_nodes <= 0 || nchans <= 0) {

        return nullptr;
    }

    NodeMap nodes;
    try {
        nodes = parse_gpsfile(gpslog);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }

    if (nodes.empty()) {
        std::cerr << "failed to parse gpslog file" << std::endl;
        return nullptr;
    }

    auto *linkmodel = new LinkModel{num_nodes, nchans, nodes};
    return static_cast<void *>(linkmodel);
}

void deinit(void *model) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    delete linkmodel;
}

bool is_connected(void *model, int x, int y, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return false;
}

void begin_send(void *model, int id, int chn, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

void end_send(void *model, int id, int chn, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

void begin_listen(void *model, int id, int chn, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

int status(void *model, int id, int chn, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return 0;
}

int end_listen(void *model, int id, int chn, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return 0;
}

#ifdef __cplusplus
}
#endif