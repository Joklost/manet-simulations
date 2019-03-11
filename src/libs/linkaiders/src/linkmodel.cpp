#include <iostream>

#include <reachi/node.h>
#include <common/helpers.h>

#include <linkaiders/linkmodel.h>

#include "model.h"
#include "gpslog.h"


#ifdef __cplusplus
extern "C" {
#endif

void *initialize(int num_nodes, int nchans, const char *gpslog) {
    if (!gpslog || num_nodes <= 0 || nchans <= 0) {
        return nullptr;
    }

    /* Parse GPS log. */
    std::unordered_map<unsigned long, reachi::Node> node_map;
    try {
        node_map = parse_gpsfile(gpslog);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }

    if (node_map.empty()) {
        std::cerr << "failed to parse gpslog file" << std::endl;
        return nullptr;
    }

    /* Generate topologies. */
    std::vector<reachi::Node> node_list{};
    node_list.reserve(node_map.size());
    for (auto &item : node_map) {
        node_list.emplace_back(item.second);
    }

    std::map<unsigned long, std::vector<reachi::Link>> topologies{};

    for (auto &node : node_list) {
        for (auto &location : node.location_history) {
            if (topologies.find(location.get_time()) == topologies.end()) {
                topologies[location.get_time()] = {};
            }
        }
    }

    for (auto &topology : topologies) {
        const auto time = topology.first;
        auto &links = topology.second;

        for (unsigned long i = 0; i < node_list.size(); ++i) {
            auto node1 = node_list[i]; /* Copy */

            for (auto &location : node1.location_history) {
                if (location.get_time() <= time) {
                    node1.current_location = location;
                }
            }

            for (unsigned long j = i + 1; j < node_list.size(); ++j) {
                auto node2 = node_list[j]; /* Copy */

                for (auto &location : node2.location_history) {
                    if (location.get_time() <= time) {
                        node2.current_location = location;
                    }
                }

                if (node1.current_location.get_time() > 0 && node2.current_location.get_time() > 0) {
                    auto id = common::combine_ids(node1.get_id(), node2.get_id());
                    links.emplace_back(id, node1, node2);
                }
            }
        }
    }

    /* Return model as void pointer. */
    auto *linkmodel = new LinkModel{num_nodes, nchans, topologies};
    return static_cast<void *>(linkmodel);
}

void deinit(void *model) {
    auto *linkmodel = static_cast<LinkModel *>(model);
    delete linkmodel;
}

bool is_connected(void *model, int x, int y, unsigned long timestamp) {
    auto *linkmodel = static_cast<LinkModel *>(model);

    auto topology = linkmodel->topologies.lower_bound(timestamp);
    if (topology == linkmodel->topologies.end()) {
        return false;
    }

    auto &links = topology->second;

    for (const auto &link : links) {
        auto &nodes = link.get_nodes();
        if ((nodes.first.get_id() == x && nodes.second.get_id() == y) ||
            (nodes.second.get_id() == x && nodes.first.get_id() == y)) {
            return true;
        }
    }

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