#ifndef LINKAIDERS_MODEL_H
#define LINKAIDERS_MODEL_H

#include <map>

#include <reachi/link.h>


struct LinkModel {
    int num_nodes{};
    int nchans{};

    std::map<unsigned long, std::vector<reachi::Link>> topologies{};
};


#endif /* LINKAIDERS_MODEL_H */
