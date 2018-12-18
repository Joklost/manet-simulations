#ifndef MANETSIMS_GPSGEN_H
#define MANETSIMS_GPSGEN_H

#include "link.h"
#include "clustering.h"

/**
 * Generate a vector of nodes (a graph) within a set of geographical bounds.
 *
 * The nodes are generated using a uniform distribution.
 *
 * @param count The amount of nodes to generate
 * @param upper The upper left geographical bound
 * @param lower The lower right geographical bound
 * @return A vector of nodes (a graph)
 */
std::vector<Node> generate_nodes(unsigned long count, Location &upper, Location &lower);

std::vector<Node>
generate_cluster(Location &center, uint32_t begin, unsigned long count, double radius /* kilometer */);

/**
 * Create a vector of unique (undirected) links from a set of nodes (a graph).
 *
 * If threshold is less than or equal to 0.01, it is assumed that the graph is fully connected.
 * If the graph is fully connected, the length of the vector of links will be
 * ((N * (N + 1)) / 2) - N.
 *
 * @param nodes A vector of nodes
 * @param threshold The threshold radius wherein nodes are assumed reachable
 * @return A vector of unique (undirected) links
 */
std::vector<Link> create_link_vector(std::vector<Node> &nodes, double threshold /* kilometers */);

std::vector<Optics::CLink> create_link_vector(std::vector<Optics::Cluster> &clusters, double threshold /* km */);

/**
 * Send nodes to the vizualising tool in chunks of 10000.
 *
 * @param nodes A vector of nodes (a graph)
 */
void visualise_nodes(std::vector<Node> &nodes);

/**
 * Send nodes to the visualising tool in chunks of #chunk_size.
 *
 * @param nodes A vector of nodes (a graph)
 * @param chunk_size Size of chunks to send
 */
void visualise_nodes(std::vector<Node> &nodes, unsigned long chunk_size);

void visualise_clusters(std::vector<Optics::Cluster> clusters);

#endif /* MANETSIMS_GPSGEN_H */
