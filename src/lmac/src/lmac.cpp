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
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    mpilib::geo::Location l1{57.01266813458001, 10.994625734716218};
    auto l2 = mpilib::geo::square(l1, 300_m);
    auto l = mpilib::geo::random_location(l1, l2);

    hardware::init(l, debug);

    auto id = hardware::get_id();

    if (id % 2ul == 0ul) {
        hardware::sleep(20ms);

        TestPacket p{id, l};
        hardware::broadcast(mpilib::serialise(p));
        hardware::sleep(180ms);
    } else {
        auto packets = hardware::listen(200ms);
        for (const auto &packet : packets) {
            hardware::logger->info(mpilib::deserialise<TestPacket>(packet));
        }

        TestPacket p{id, l};
        hardware::broadcast(mpilib::serialise(p));
    }

//    auto new_location = mpilib::geo::random_location(l1, l2);
//    hardware::set_location(new_location);

    hardware::deinit();

    return 0;
}