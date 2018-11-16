#ifndef MANETSIMS_GPSGEN_H
#define MANETSIMS_GPSGEN_H

#include <mpilib/link.h>


std::vector<Node> generate_nodes(unsigned long count, Location &upper, Location &lower);

std::vector<Link> generate_links(std::vector<Node> &nodes);

void visualise_nodes(std::vector<Node> &nodes);

#endif /* MANETSIMS_GPSGEN_H */
