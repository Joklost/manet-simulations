#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include <geo/geo.h>

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

std::vector<std::string> split(const std::string &string, const std::string &delim) {
    std::vector<std::string> tokens{};
    size_t prev{}, pos{};

    do {
        pos = string.find(delim, prev);
        if (pos == std::string::npos) {
            pos = string.length();
        }

        std::string token = string.substr(prev, pos - prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        prev = pos + delim.length();
    } while (pos < string.length() && prev < string.length());
    return tokens;
}

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

            auto tokens = split(line, ",");
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