#include <utility>

#ifndef LINKAIDERS_MODEL_H
#define LINKAIDERS_MODEL_H

#include <map>
#include <unordered_map>

#include <reachi/link.h>

#include <common/queue.h>

namespace linkaiders {
    const double DISTANCE_THRESHOLD = -110.0;
    const double TX_POWER = 26.0;

    enum State {
        Listen,
        Transmit,
//        Idle,
    };

    /* Model */
    struct Action {
        State type{};
        int id{};
        int chn{};
        unsigned long start{};
        unsigned long end{};

        bool is_within(const Action &action) const;

        bool operator==(const Action &rhs) const;

        bool operator!=(const Action &rhs) const;
    };

    struct Topology {
        std::vector<reachi::Link> links{};
    };

    using NodeMap = std::unordered_map<unsigned long, reachi::Node>;
    using TopologyMap = std::map<unsigned long, Topology>;

    struct LinkModel {
        NodeMap nodes{};
        TopologyMap topologies{};

//        static bool cmp(const Action &left, const Action &right) {
//            return left.end > right.end;
//        }

        std::vector<Action> acts{};
//        common::PriorityQueue<Action, std::vector<Action>, decltype(&cmp)> listen_queue{cmp};

        LinkModel(NodeMap node_map, TopologyMap topology_map) : nodes(std::move(node_map)),
                                                                topologies(std::move(topology_map)) {};
    };
}


#endif /* LINKAIDERS_MODEL_H */
