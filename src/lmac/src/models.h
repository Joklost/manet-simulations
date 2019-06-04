#ifndef LMAC_MODELS_H
#define LMAC_MODELS_H
// 100 000 us (100 ms)
//constexpr auto SLOT_LENGTH = 20000us; // NOLINT(cert-err58-cpp)
constexpr auto SLOT_LENGTH = 100000us; // NOLINT(cert-err58-cpp)
constexpr auto SLOTS = 8ul;
constexpr auto FRAMES = 90ul;
constexpr auto MAX_WAIT = 6ul;
constexpr soctet NO_CHOSEN_SLOT = static_cast<soctet>(SLOTS + 1);
constexpr soctet NO_SLOT = -1;
constexpr soctet NO_RECEIVER = -1;

enum Phase {
    nil,
    init,
    wait,
    discover,
    active
};

struct Node {
    short id{};
    soctet selected_slot{NO_CHOSEN_SLOT};
    std::bitset<SLOTS> occupied_slots{0};
    octet gateway_distance{0xFF};
};

struct State {
    Phase phase{init};
    Phase next_phase{nil};
    unsigned long wait_frames{0ul};
    soctet chosen_slot{NO_CHOSEN_SLOT};
    soctet next_slot{NO_CHOSEN_SLOT};
    soctet collision_slot{NO_SLOT};
    octet gateway_distance{0xFF};
    std::bitset<SLOTS> occupied_slots{};
    std::unordered_map<short, Node> neighbourhood{};
    std::vector<octet> data_packet{};
};

struct ControlPacket {
    short id{};
    soctet current_slot{};
    std::bitset<SLOTS> occupied_slots{};
    octet gateway_distance{};
    soctet collision_slot{};
    short destination_id{};
    octet data_size{};

    friend std::ostream &operator<<(std::ostream &os, const ControlPacket &packet) {
        os << "ControlPacket{"
           << "id: " << packet.id
           << ", current_slot: " << +packet.current_slot
           << ", occupied_slots: " << packet.occupied_slots
           << ", gateway_distance: " << +packet.gateway_distance
           << ", collision: " << +packet.collision_slot
           << ", destination_id: " << packet.destination_id
           << ", data_size: " << +packet.data_size
           << "}";
        return os;
    }
};


#endif //LMAC_MODELS_H
