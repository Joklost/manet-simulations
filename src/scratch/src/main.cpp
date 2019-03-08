#include <mpilib/hardware.h>

#include <ostream>
#include <iostream>
#include <csignal>
#include <thread>
#include <random>

#include <geo/geo.h>

class TestPacket {
public:
    unsigned long rank{};
    geo::Location loc{};

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

    geo::Location l1{57.0121621, 9.990679};
    auto l2 = geo::square(l1, 2_km);
    auto l = geo::random_location(l1, l2);
    hardware::init(l1, false, debug);
    auto id = hardware::get_id();
    //auto l = geo::move_location(l1, 500_m * id, 90);

    hardware::handshake(l);
    auto &console = hardware::logger;

    if (id % 2 == 0) {
        auto secret = TestPacket{id, l};
        hardware::sleep(20ms);
        auto duration = hardware::broadcast(mpilib::serialise(secret));
        hardware::sleep(2s - duration - 20ms);
    } else {
        auto packets = hardware::listen(2s);

        if (!packets.empty()) {
            for (auto &packet : packets) {
                console->info(mpilib::deserialise<TestPacket>(packet));
            }
        }
    }

    if (id % 2 == 0) {
        auto packets = hardware::listen(2s);

        if (!packets.empty()) {
            for (auto &packet : packets) {
                console->info(mpilib::deserialise<TestPacket>(packet));
            }
        }
    } else {
        auto secret = TestPacket{id, l};
        hardware::sleep(20ms);
        auto duration = hardware::broadcast(mpilib::serialise(secret));
        hardware::sleep(2s - duration - 20ms);
    }
//    auto slots = hardware::get_world_size();
//    std::random_device rd{};
//    std::mt19937 eng{rd()};
//
//    std::uniform_int_distribution<unsigned long> dist{0ul, 32};
//
//    Packet secret{};
//
//    if (id == 1ul) {
//        secret = Packet{id, 0x42};
//    }
//
//    auto slot_length = 2s;
//    for (auto i = 0; i < 3; ++i) {
//        auto selected = dist(eng);
//        for (auto current = 0; current < slots; ++current) {
//            if (selected == current) {
//                if (secret.rank != 0ul) {
//                    //secret.rank = id;
//                    hardware::sleep(20ms);
//                    auto duration = hardware::broadcast(mpilib::serialise(secret));
//                    hardware::sleep(slot_length - duration - 20ms);
//                } else {
//                    hardware::sleep(slot_length);
//                }
//            } else {
//                if (secret.rank != 0ul) {
//                    hardware::sleep(slot_length);
//                } else {
//                    auto packets = hardware::listen(slot_length);
//
//                    if (!packets.empty()) {
//                        secret = mpilib::deserialise<Packet>(packets.front());
//                    }
//                }
//
//            }
//        }
//    }

//    auto localtime = hardware::get_localtime();
//
//    if (secret.rank != 0ul) {
//        console->info("{} - {}", format_duration(localtime), secret);
//    } else {
//        console->info("{} - Nothing received!", format_duration(localtime));
//    }

    hardware::deinit();

    return 0;
}