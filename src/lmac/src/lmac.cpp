#include <mpilib/hardware.h>

#include <ostream>
#include <iostream>
#include <csignal>

class TestPacket {
public:
    unsigned long rank{};
    mpilib::geo::Location loc{};

    bool operator==(const TestPacket &rhs) const {
        return rank == rhs.rank;
    }

    bool operator!=(const TestPacket &rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const TestPacket &packet) {
        os << "Packet{rank: " << packet.rank << ", loc: " << packet.loc << "}";
        return os;
    }

};

int main(int argc, char *argv[]) {

    mpilib::geo::Location l1{57.01266813458001, 10.994625734716218};
    auto l2 = square(l1, 1.0);
    auto l = random_location(l1, l2);

    hardware::init(l, true);

    auto id = hardware::get_id();

    TestPacket p{id, l};
    hardware::broadcast<TestPacket>(p);
    auto packets = hardware::listen<TestPacket>(1ul);
    for (const auto &packet : packets) {
        hardware::logger->info(packet);
    }

    hardware::deinit();

    return 0;
}