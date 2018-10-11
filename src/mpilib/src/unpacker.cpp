
#include <stdlib.h>
#include "mpilib/unpacker.h"

#ifdef __cplusplus
/*
std::vector<packet_t *> unpack(const uint8_t *packets) {
    std::vector<packet_t *> packet_vector;
    if (packets == nullptr) {
        return packet_vector;
    }

    uint8_t packet_count = *(packets)++;
    packet_vector.reserve(packet_count);

    for (uint8_t i = 0; i < packet_count; ++i) {
        auto *p = new packet_t();
        packet_vector.push_back(p);
        p->size = *(packets)++;
        p->data = new uint8_t[p->size];

        for (uint8_t j = 0; j < p->size; ++j) {
            p->data[j] = *(packets)++;
        }
    }

    return packet_vector;
}*/

#else

#endif /* __cplusplus */
