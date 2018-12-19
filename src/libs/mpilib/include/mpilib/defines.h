#ifndef MANETSIMS_DEFINES_H
#define MANETSIMS_DEFINES_H

/* Id/Source */
#define CTLR 0

/* Tags */
#define TX_PKT 1
#define TX_PKT_ACK 11
#define RX_PKT 2
#define RX_PKT_ACK 22
#define RX_PKT_COUNT 222
#define SLEEP 3
#define SLEEP_ACK 33
#define LOCATION 4
#define LOCAL_TIME_REQ 5
#define LOCAL_TIME_RSP 55
#define WORLD_SIZE_REQ 6
#define WORLD_SIZE_RSP 66

#define HANDSHAKE 10
#define DIE 101

/* Using declarations */
using octet = unsigned char;

template<typename T>
using vecvec = std::vector<std::vector<T>>;

#endif /* MANETSIMS_DEFINES_H */
