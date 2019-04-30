#include <fstream>

#include "statistics.h"

void Statistics::save() {
    std::ofstream tsize_out{"transmissions_sizes.txt"};
    tsize_out << "#timestamp,transmissions_size" << std::endl;
    for (const auto &item : this->transmissions_sizes) {
        tsize_out
                << item.time << ","
                << item.tsize << std::endl;
    }
    tsize_out.close();


    std::ofstream qsize_out{"queue_sizes.txt"};
    qsize_out << "#timestamp,queue_size" << std::endl;
    for (const auto &item : this->queue_sizes) {
        qsize_out
                << item.time << ","
                << item.qsize << std::endl;
    }
    qsize_out.close();

    std::ofstream packetloss_out{"packetloss.txt"};
    packetloss_out << "#received,tx_id,rx_id,rssi,pep,interfering_power,interferers,tx_start" << std::endl;
    for (const auto &item : this->packetloss) {
        packetloss_out
                << (item.received ? "recv" : "drop") << ","
                << item.tx_id << ","
                << item.rx_id << ","
                << item.rssi << ","
                << item.pep << ","
                << item.interfering_power << ","
                << item.interferers << ","
                << item.tx_start << std::endl;
    }
    packetloss_out.close();


}

Statistics::QueueSize::QueueSize(unsigned long time, unsigned long qsize) : time(time), qsize(qsize) {}

Statistics::PacketLoss::PacketLoss(bool received, unsigned long txid, unsigned long rxid, double rssi, double pep,
                                   double interferingPower, unsigned long interferers, unsigned long txstart)
        : received(received), tx_id(txid), rx_id(rxid), rssi(rssi), pep(pep), interfering_power(interferingPower),
          interferers(interferers), tx_start(txstart) {}

Statistics::TransmissionsSize::TransmissionsSize(unsigned long time, unsigned long tsize) : time(time), tsize(tsize) {}
