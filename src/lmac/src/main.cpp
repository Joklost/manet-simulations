#include <mpilib/hardware.h>
#include <ostream>
#include <iostream>
#include <csignal>

class Packet {
public:
    std::uint8_t v1{};
    std::uint8_t v2{};
    std::uint8_t v3{};
    std::uint8_t v4{};
    std::uint8_t v5{};
    std::uint8_t v6{};
    std::uint8_t v7{};
    std::uint8_t v8{};
    std::uint16_t v9{};
    std::uint32_t v10{};
    unsigned long v11{};
    int v12{};
    double v13{};
    float v14{};

    friend std::ostream &operator<<(std::ostream &os, const Packet &packet) {
        os << "v1: " << packet.v1 << " v2: " << packet.v2 << " v3: " << packet.v3 << " v4: " << packet.v4 << " v5: "
           << packet.v5 << " v6: " << packet.v6 << " v7: " << packet.v7 << " v8: " << packet.v8 << " v9: " << packet.v9
           << " v10: " << packet.v10 << " v11: " << packet.v11 << " v12: " << packet.v12 << " v13: " << packet.v13
           << " v14: " << packet.v14;
        return os;
    }

    bool operator==(const Packet &rhs) const {
        return v1 == rhs.v1 &&
               v2 == rhs.v2 &&
               v3 == rhs.v3 &&
               v4 == rhs.v4 &&
               v5 == rhs.v5 &&
               v6 == rhs.v6 &&
               v7 == rhs.v7 &&
               v8 == rhs.v8 &&
               v9 == rhs.v9 &&
               v10 == rhs.v10 &&
               v11 == rhs.v11 &&
               v12 == rhs.v12 &&
               v13 == rhs.v13 &&
               v14 == rhs.v14;
    }

    bool operator!=(const Packet &rhs) const {
        return !(rhs == *this);
    }
};

static bool work = true;

void stop_work(int signal) {
    work = false;
}

int main(int argc, char *argv[]) {

    std::signal(SIGINT, stop_work);

    init_hardware<Packet>();

    Packet p1{'D', 'E', 'A', 'D', 'B', 'E', 'E', 'F', 0x1234, 0x87654321, 4242, 42, 42.42, 13.37};
    /*Packet p2{43, "Packet 2"};
    Packet p3{44, "Packet 3"};
    Packet p4{45, "Packet 4"};
    Packet p5{46, "Packet 5"};
*/
    tx(p1);
    /*  tx(p2);
      tx(p3);
      tx(p4);
      tx(p5);
  */
    auto packets = rx<Packet>(1ul);
    assert(p1 == packets.front());
    //for (const auto &packet : packets) {
    //    std::cout << packet << std::endl;
    //}


    //while (work);

    deinit_hardware<Packet>();
}