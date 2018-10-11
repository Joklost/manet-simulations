#ifndef MANETSIMS_UNPACKER_H
#define MANETSIMS_UNPACKER_H

#include <stdint.h>
#include "packet.h"

#ifdef __cplusplus

#include <vector>

#endif /* __cplusplus */


#ifdef __cplusplus

//std::vector<packet_t *> unpack(const uint8_t *packets);

#else

extern "C" packet_t * unpack(uint8_t *packets);

#endif /* __cplusplus */

#endif /* MANETSIMS_UNPACKER_H */
