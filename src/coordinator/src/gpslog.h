#ifndef COORDINATOR_GPSLOG_H
#define COORDINATOR_GPSLOG_H

#include <vector>

#include "models/node.h"

std::pair<double, std::vector<Node>> load_log(const char *logpath);

#endif /* COORDINATOR_GPSLOG_H */
