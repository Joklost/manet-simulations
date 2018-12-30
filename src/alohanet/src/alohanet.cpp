#include <mpilib/hardware.h>
#include <random>
#include <ostream>

struct Packet {
    unsigned long rank{};
    unsigned long selected{};

    friend std::ostream &operator<<(std::ostream &os, const Packet &packet) {
        os << "Packet{rank: " << packet.rank << ", selected: " << packet.selected << "}";
        return os;
    }
};

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    mpilib::geo::Location l1{57.01266813458001, 10.994625734716218};
    mpilib::geo::Location l2 = square(l1, 1.0);
    mpilib::geo::Location l = random_location(l1, l2);
    hardware::init(l, debug);

    auto id = hardware::get_id();
    std::string sid = std::string(3 - std::to_string(id).length(), '0') + std::to_string(id);
    auto console = spdlog::stderr_color_st("aloha" + sid);

    auto slots = hardware::get_world_size();
    if (id == slots) {
        console->info("start");
    }
    std::random_device rd{};
    std::mt19937 eng{rd()};
    std::uniform_int_distribution<unsigned long> dist{0ul, slots};

    for (auto i = 0; i < 3; ++i) {
        auto selected = dist(eng);
        for (auto current = 0; current < slots; ++current) {
            if (selected == current) {
                Packet p{id, selected};
                auto duration = hardware::broadcast(p);
                hardware::sleep(200ms - duration);
            } else {
                auto packets = hardware::listen<Packet>(200ms);

                for (const auto &item : packets) {
                    console->info(item);
                }
            }
        }
    }

    if (id == slots) {
        console->info("stop");
    }
    hardware::deinit();
}