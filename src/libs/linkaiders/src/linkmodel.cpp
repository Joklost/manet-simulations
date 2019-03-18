#include <iostream>
#include <algorithm>
#include <iterator>

#include <reachi/math.h>
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
    linkaiders::NodeMap node_map;
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
        auto node = item.second;
        node_list.push_back(node);
    }

    linkaiders::TopologyMap topologies{};

    for (auto &node : node_list) {
        for (auto &location : node.location_history) {
            if (topologies.find(location.get_time()) == topologies.end()) {
                topologies[location.get_time()] = {};
            }
        }
    }

    for (auto &topology : topologies) {
        const auto time = topology.first;
        auto &links = topology.second.links;

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
                    auto &link = links.back();
                    /* Compute distance based path loss on the links as we create them. */
                    auto distance = geo::distance_between(node1.current_location, node2.current_location);
                    auto pathloss = reachi::math::distance_pathloss(distance * KM);
                    link.distance = pathloss;
                }
            }
        }
    }

    /* Return model as void pointer. */
    auto *lm = new linkaiders::LinkModel{node_map, topologies};
    return static_cast<void *>(lm);
}

void deinit(void *model) {
    if (model == nullptr) {
        return;
    }

    auto *lm = static_cast<linkaiders::LinkModel *>(model);
    delete lm;
}

bool is_connected(void *model, int x, int y, unsigned long timestamp) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);

    auto topology = lm->topologies.lower_bound(timestamp);
    if (topology == lm->topologies.end()) {
        return false;
    }

    auto &links = topology->second.links;

    for (const auto &link : links) {
        auto &nodes = link.get_nodes();
        if (((nodes.first.get_id() == x && nodes.second.get_id() == y) ||
             (nodes.first.get_id() == y && nodes.second.get_id() == x)) &&
            ((linkaiders::TX_POWER - link.distance) > linkaiders::DISTANCE_THRESHOLD)) {
            return true;
        }
    }

    return false;
}

void begin_send(void *model, int id, int chn, unsigned long timestamp, unsigned long duration) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);
    lm->acts.emplace_back(linkaiders::Transmit, id, chn, timestamp, timestamp + duration);
}

void end_send(void *model, int id, int chn, unsigned long timestamp) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);
    auto it = std::find(lm->acts.begin(), lm->acts.end(), linkaiders::Action{linkaiders::Transmit, id, chn, timestamp});
    it->end = timestamp;
}

void begin_listen(void *model, int id, int chn, unsigned long timestamp, unsigned long duration) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);
    lm->acts.emplace_back(linkaiders::Listen, id, chn, timestamp, timestamp + duration);

}

int status(void *model, int id, int chn, unsigned long timestamp) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);

    return 0;
}

int end_listen(void *model, int id, int chn, unsigned long timestamp) {
    auto *lm = static_cast<linkaiders::LinkModel *>(model);
    auto it = std::find(lm->acts.begin(), lm->acts.end(), linkaiders::Action{linkaiders::Listen, id, chn, timestamp});
    it->end = timestamp;

    return 0;
}

#ifdef __cplusplus
}
#endif