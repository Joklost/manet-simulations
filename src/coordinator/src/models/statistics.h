#ifndef COORDINATOR_STATISTICS_H
#define COORDINATOR_STATISTICS_H


#include <vector>

struct Statistics {
    struct TransmissionsSize {
        TransmissionsSize(unsigned long time, unsigned long tsize);

        unsigned long time{};
        unsigned long tsize{};
    };

    struct QueueSize {
        QueueSize(unsigned long time, unsigned long qsize);

        unsigned long time{};
        unsigned long qsize{};
    };

    struct PacketLoss {
        PacketLoss(bool received, unsigned long txid, unsigned long rxid, unsigned long packetsize, double rssi,
                   double pep, double interferingPower, unsigned long interferers, unsigned long txstart,
                   unsigned long txend);

        bool received{};
        unsigned long tx_id{};
        unsigned long rx_id{};
        unsigned long packet_size{};
        double rssi{};
        double pep{};
        double interfering_power{};
        unsigned long interferers{};
        unsigned long tx_start{};
        unsigned long tx_end{};
    };

    std::vector<Statistics::TransmissionsSize> transmissions_sizes{};
    std::vector<Statistics::QueueSize> queue_sizes{};
    std::vector<Statistics::PacketLoss> packetloss{};

    void save();
};


#endif //COORDINATOR_STATISTICS_H
