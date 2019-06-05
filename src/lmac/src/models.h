#ifndef LMAC_MODELS_H
#define LMAC_MODELS_H

//#include <common/queue.h>
#include <queue>

// 100 000 us (100 ms)
//constexpr auto SLOT_LENGTH = 20000us; // NOLINT(cert-err58-cpp)
constexpr auto SLOT_LENGTH = 100000us; // NOLINT(cert-err58-cpp)
constexpr auto SLOTS = 32ul;
constexpr auto FRAMES = 120ul;
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

std::string to_string(Phase phase) {
    switch (phase) {
        case nil:
            return std::string{"nil"};
        case init:
            return std::string{"i"};
        case wait:
            return std::string{"w"};
        case discover:
            return std::string{"d"};
        case active:
            return std::string{"a"};
        default:
            return std::string{};
    }
}

struct Node {
    short id{};
    soctet selected_slot{NO_CHOSEN_SLOT};
    std::bitset<SLOTS> occupied_slots{0};
    octet gateway_distance{0xFF};
};

struct State {
    short id{};
    Phase phase{init};
    Phase next_phase{nil};
    unsigned long wait_frames{0ul};
    soctet chosen_slot{NO_CHOSEN_SLOT};
    soctet next_slot{NO_CHOSEN_SLOT};
    soctet collision_slot{NO_SLOT};
    octet gateway_distance{0xFF};
    std::bitset<SLOTS> occupied_slots{};
    std::unordered_map<short, Node> neighbourhood{};
    std::bitset<SLOTS> neighbourhood_slots{};
    std::vector<octet> data_packet{};
//    common::Queue<std::vector<octet>> packet_queue{};

    std::queue<std::vector<octet>> packet_queue{};

    /* Model 4: If nothing have been received in a frame, choose a new slot. */
    bool nothing_received{true};
    /* Model 6: Has yet to send its first message. */
    bool sent_first_message{false};
    /* Model 8: If nothing has been heard for two frame lengths, the node is alone. */
    int disconnected_counter{};

    std::string phase_string();

    void log_state_change();
};

std::string State::phase_string() {
    std::string phase_s;
    if (this->phase == active) {
        phase_s = std::to_string(this->chosen_slot);
    } else {
        phase_s = to_string(this->phase);
    }
    return phase_s;
}

void State::log_state_change() {
    hardware::logger->info("{},{},{}", hardware::get_localtime().count() / 1000.0, this->id, this->phase_string());
}

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
