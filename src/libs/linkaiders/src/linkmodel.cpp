#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include <mpilib/location.h>
#include <mpilib/helpers.h>

struct Node {
    unsigned long id{};
    std::vector<mpilib::geo::Location> location_history{};
};

struct LinkModel {
    int num_nodes{};
    int nchans{};

    std::unordered_map<unsigned long, Node> nodes{};
};

std::unordered_map<unsigned long, Node> parse_gpsfile(char *gpslog) {
    std::unordered_map<unsigned long, Node> nodes{};

    std::vector<std::string> gps{};
    std::ifstream logfile{gpslog};

    if (logfile.is_open()) {
        while (logfile.good()) {
            std::string line;
            std::getline(logfile, line);
            gps.push_back(line);

            auto tokens = mpilib::split(line, ",");

            /* 0: id, 1: latitude, 2: longitude, 3: timestamp */
            auto id = std::stoul(tokens[0]);
            auto latitude = std::stod(tokens[1]);
            auto longitude = std::stod(tokens[2]);
            auto timestamp = std::stoul(tokens[3]);

            auto node = nodes[id]; /* operator[] implicitly constructs new object */
            node.id = id;
            node.location_history.emplace_back(timestamp, latitude, longitude);
        }

        logfile.close();
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

    auto nodes = parse_gpsfile(gpslog);
    if (nodes.empty()) {
        return nullptr;
    }

    auto *linkmodel = new LinkModel{num_nodes, nchans, nodes};
    return static_cast<void *>(linkmodel);
}

void deinit(void *model) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    
    std::cout << linkmodel->nodes.size() << std::endl;
    
    delete linkmodel;
}

bool is_connected(void *model, int x, int y, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return false;
}

void begin_send(void *model, int id, int chn, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

void end_send(void *model, int id, int chn, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

void begin_listen(void *model, int id, int chn, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

}

int status(void *model, int id, int chn, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return 0;
}

int end_listen(void *model, int id, int chn, double timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    return 0;
}

#ifdef __cplusplus
}
#endif