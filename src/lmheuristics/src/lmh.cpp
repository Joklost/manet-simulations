#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include <common/strings.h>
#include <geo/geo.h>
#include <geo/location.h>
#include <sims2/link.h>
#include <sims2/linkmodel.h>
#include <sims2/datagen.h>

std::unordered_map<double, std::vector<sims2::Node>> load_nodes(const char *file) {
    std::fstream stream(std::string("../../logs/parsed/").append(file), stream.in);
//    std::fstream stream("../../logs/parsed/gridlog_8x8_rssi.txt", stream.in);
    std::unordered_map<double, std::vector<sims2::Node>> nodes{};

    for (std::string line; std::getline(stream, line);) {
        auto split = common::split(line, ",");
        auto time = std::stod(split[3]);
        if (nodes.find(time) == nodes.end())
            nodes[time] = std::vector<sims2::Node>{};

        nodes[time].emplace_back(sims2::Node{
                std::stoul(split[0]),
                geo::Location{std::stod(split[1]), std::stod(split[2])}
        });
    }
    stream.close();
    return nodes;
}

int main(int argc, char *argv[]) {
    auto input_file = "original";
    auto output_file = "data1";

    if (argc == 3) {
        input_file = argv[1];
        output_file = argv[2];
    }

    auto log = load_nodes(input_file);
    std::fstream stream(std::string("../../logs/parsed/").append(output_file), stream.out);
//    std::fstream stream("../../logs/parsed/data1", stream.out);

    for (const auto &nodes : log) {
        auto links = sims2::data::create_links(nodes.second);

        auto linkmodel = sims2::Linkmodel(links);
        linkmodel.compute();
        auto rssi = linkmodel.get_rssi();

        for (const auto &node : nodes.second) {
            // iter through every node and find neighbourhood
            std::string line;
            for (const auto &link : links) {
                if (!link.has_node(node)) {
                    continue;
                }

                auto common = node;
                auto other = link.node1 == node ? link.node2 : link.node1;
                line += "," + std::to_string(other.id) + "," + std::to_string(rssi[link.id]);
            }
            stream
                    << std::to_string(node.id) << ","
                    << std::to_string(node.location.get_latitude()) << ","
                    << std::to_string(node.location.get_longitude()) << ","
                    << std::to_string(nodes.first)
                    << line
                    << std::endl;
        }
    }
    stream.close();
}
