#include <mpilib/hardware.h>
#include <random>
#include <ostream>
#include <cstdlib>

struct Packet {
    unsigned long rank{};
    octet key{};

    friend std::ostream &operator<<(std::ostream &os, const Packet &packet) {
        os << "Packet{rank: " << packet.rank << ", key: " << packet.key << "}";
        return os;
    }
};

int main(int argc, char *argv[]) {
    auto debug = false;
    mpilib::geo::Location l{};
    if (argc > 1 && std::string{"--debug"} == std::string{argv[1]}) {
        debug = true;

        mpilib::geo::Location l1{57.01266813458001, 10.994625734716218};
        mpilib::geo::Location l2 = square(l1, 1.0);
        l = random_location(l1, l2);
    }

    if (argc == 3) {
        char* end;
        auto lat = std::strtod(argv[1], &end);
        auto lon = std::strtod(argv[2], &end);
        l = mpilib::geo::Location{lat, lon};
    }

    hardware::init(l, debug);

    auto id = hardware::get_id();
    std::string sid = std::string(3 - std::to_string(id).length(), '0') + std::to_string(id);
    auto console = spdlog::stderr_color_st("aloha" + sid);
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
                    auto duration = hardware::broadcast(mpilib::serialise(secret));
                    hardware::sleep(slot_length - duration);
                } else {
                    hardware::sleep(slot_length);
                }
            } else {
                auto packets = hardware::listen(slot_length);

                if (!packets.empty()) {
                    secret = mpilib::deserialise<Packet>(packets.front());
                }

                //for (const auto &item : packets) {
                //    console->info(mpilib::deserialise<Packet>(item));
                //}
            }
        }
    }

    if (secret.rank != 0ul) {
      //  console->info(secret);
    } else {
        console->info("Nothing received!");
    }

    hardware::deinit();
}