#ifndef COORDINATOR_GPSLOG_H
#define COORDINATOR_GPSLOG_H

#include <map>

#include "node.h"

std::map<unsigned long, Node> load_log(const char *logpath);

#endif /* COORDINATOR_GPSLOG_H */
