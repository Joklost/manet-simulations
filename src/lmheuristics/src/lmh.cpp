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

std::unordered_map<double, std::vector<sims2::Node>> load_nodes() {
    std::fstream stream("../../logs/parsed/original", stream.in);
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

void write_data_to_file(const char *file_name, std::unordered_map<double, std::vector<sims2::Link>> data) {
    std::fstream stream(file_name, stream.out);
    std::for_each(data.cbegin(), data.cend(), [&stream](auto element) {
        auto time = element.first;
        for (const auto &link : element.second) {

        }
    });
    stream.close();
}

int main(int argc, char *argv[]) {
    auto nodes = load_nodes();
    double max_links_time = 0.0;
    int max_links_count = 0;

    std::for_each(nodes.begin(), nodes.end(), [&max_links_time, &max_links_count](auto element) {
        auto links = sims2::data::create_links(element.second);
        if (links.size() > max_links_count) {
            max_links_time = element.first;
            max_links_count = links.size();
        }
    });

    auto links = sims2::data::create_links(nodes[max_links_time]);

    auto linkmodel = sims2::Linkmodel(links);
    linkmodel.compute();
    auto rssi = linkmodel.get_rssi();

    std::fstream stream("../../logs/parsed/data1", stream.out);
    for (const auto &node : nodes[max_links_time]) {
        // iter through every node and find neighbourhood
        std::string line;
        for (const auto &link : links) {
            if (!link.has_node(node)) {
                continue;
            }

            auto common = node;
            auto other = link.node1 == node ? link.node2 : link.node1;
            std::cout << rssi[link.id] << std::endl;
            line += "," + std::to_string(other.id) + "," + std::to_string(rssi[link.id]);
        }
        stream
                << std::to_string(node.id) << ","
                << std::to_string(node.location.get_latitude()) << ","
                << std::to_string(node.location.get_longitude()) << ","
                << std::to_string(max_links_time)
                << line
                << std::endl;
    }
}
