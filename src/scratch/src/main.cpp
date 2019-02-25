#include <mpilib/hardware.h>

#include <ostream>
#include <iostream>
#include <csignal>
#include <thread>
#include <mpilib/geomath.h>
#include <random>

struct Packet {
    unsigned long rank{};
    octet key{};

    friend std::ostream &operator<<(std::ostream &os, const Packet &packet) {
        os << "Packet{rank: " << packet.rank << ", key: " << packet.key << "}";
        return os;
    }
};

template<class...Durations, class DurationIn>
std::tuple<Durations...> break_down_durations(DurationIn d) {
    std::tuple<Durations...> retval;
    using discard=int[];
    (void) discard{0, (void((
                                    (std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
                                            (d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval)))
                            )), 0)...};
    return retval;
}

std::string format_duration(std::chrono::microseconds us) {
    auto dur = break_down_durations<std::chrono::seconds, std::chrono::milliseconds, std::chrono::microseconds>(us);
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::get<0>(dur).count()
        << "::"
        << std::setw(3)
        << std::get<1>(dur).count()
        << "::"
        << std::setw(3)
        << std::get<2>(dur).count();
    return oss.str();
}
//
//class TestPacket {
//public:
//    unsigned long rank{};
//    mpilib::geo::Location loc{};
//
//    bool operator==(const TestPacket &rhs) const {
//        return rank == rhs.rank;
//    }
//
//    bool operator!=(const TestPacket &rhs) const {
//        return !(rhs == *this);
//    }
//
//    friend std::ostream &operator<<(std::ostream &os, const TestPacket &packet) {
//        os << "Packet{rank: " << packet.rank << ", loc: " << packet.loc << "}";
//        return os;
//    }
//
//};

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    mpilib::geo::Location l1{57.01266813458001, 10.994625734716218};

    hardware::init(l1, false, debug);
    auto id = hardware::get_id();
    auto l = mpilib::geo::move_location(l1, 500_m * id, 90);
    hardware::handshake(l);
    auto &console = hardware::logger;
    auto slots = hardware::get_world_size();
    std::random_device rd{};
    std::mt19937 eng{rd()};
    std::uniform_int_distribution<unsigned long> dist{0ul, 8};

    Packet secret{};

    if (id == 1ul) {
        secret = Packet{id, 0x42};
    }

    auto slot_length = 2s;
    for (auto i = 0; i < 3; ++i) {
        auto selected = dist(eng);
        for (auto current = 0; current < slots; ++current) {
            if (selected == current) {
                if (secret.rank != 0ul) {
                    //secret.rank = id;
                    auto duration = hardware::broadcast(mpilib::serialise(secret));
                    hardware::sleep(slot_length - duration);
                } else {
                    hardware::sleep(slot_length);
                }
            } else {
                if (secret.rank != 0ul) {
                    hardware::sleep(slot_length);
                } else {
                    auto packets = hardware::listen(slot_length);

                    if (!packets.empty()) {
                        secret = mpilib::deserialise<Packet>(packets.front());
                    }
                }

            }
        }
    }

    auto localtime = hardware::get_localtime();

    if (secret.rank != 0ul) {
        console->info("{} - {}", format_duration(localtime), secret);
    } else {
        console->info("{} - Nothing received!", format_duration(localtime));
    }

    hardware::deinit();

    return 0;
}