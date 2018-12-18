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
    Location l1{57.01266813458001, 10.994625734716218};
    Location l2 = square(l1, 1.0);
    Location l = random_location(l1, l2);
    init_hardware(l);

    auto id = get_id();
    std::string sid = std::string(3 - std::to_string(id).length(), '0') + std::to_string(id);
    auto console = spdlog::stderr_color_st("aloha" + sid);

    auto slots = get_world_size();

    std::random_device rd{};
    std::mt19937 eng{rd()};
    std::uniform_int_distribution<unsigned long> dist{0ul, slots};

    auto selected = dist(eng);
    console->info("selected={}", selected);
    for (auto current = 0; current < slots; ++current) {
        if (selected == current) {
            Packet p{id, selected};
            tx(p);
        } else {
            auto packets = rx<Packet>(1ul);

            for (const auto &item : packets) {
                console->info(item);
            }
        }
    }

    deinit_hardware();
}