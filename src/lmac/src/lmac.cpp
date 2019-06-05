#include <bitset>

#include <mpilib/random.h>
#include <mpilib/interface.h>
#include "models.h"

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    hardware::init(debug);
    auto id = static_cast<short>(hardware::get_id());
    auto rank = hardware::get_world_rank();
    auto world_size = hardware::get_world_size();

    std::random_device rd{};
    std::default_random_engine eng{rd()};
    std::uniform_int_distribution<unsigned long> dist{1ul, MAX_WAIT};
    auto gen_wait = std::bind(dist, eng);

    State state{id};

    auto gateway = false;
    if (rank == 1) {
        gateway = true;
        state.chosen_slot = 0;
        state.next_slot = 0;
        state.occupied_slots[0] = true;
    }

    auto sensor = false;
    octet seq_num = 0;
    std::vector<octet> sensor_data{};
    if (rank == world_size) {
        sensor = true;
        sensor_data = {
                0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF,
                0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF,
                0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF,
                0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF,
                0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF
        };
    }

    hardware::logger->info("{},{},{}", 0.0, id, to_string(state.phase));

    auto slot_count = 1;
    for (auto frame = 1ul; frame <= FRAMES; ++frame) {
        /* If waiting, decrement the number of frames before moving on to discover phase. */
        if (state.phase == wait) {
            if (state.wait_frames > 0ul) {
                state.wait_frames--;
            } else {
                state.next_phase = discover;
            }
        }

        /* Move node to next phase. */
        if (state.next_phase != nil) {
            /* Generate the number of frames the node should be in waiting. */
            if (state.next_phase == wait) {
                state.chosen_slot = NO_CHOSEN_SLOT;
                state.next_slot = NO_CHOSEN_SLOT;
                state.wait_frames = gen_wait();
            }

            if (state.next_phase == active) {
                state.sent_first_message = false;
                state.chosen_slot = state.next_slot;
                state.next_slot = NO_CHOSEN_SLOT;
                state.occupied_slots[state.chosen_slot] = true;
            }

            state.phase = state.next_phase;
            state.next_phase = nil;

            /* Log state change. */
            state.log_state_change();
        }

//        state.nothing_received = true;

        for (auto slot = 0ul; slot < SLOTS; ++slot) {
            if (slot == state.chosen_slot) {
                if (state.phase == init) {
                    /* Send initial synchronisation signal. */
                    hardware::sleep(2ms);
                    ControlPacket ctrl{id, state.chosen_slot, state.occupied_slots, 0, -1, -1, 0};
                    hardware::transmit(mpilib::serialise(ctrl));

                    if (gateway) {
                        state.next_phase = active;
                    } else {
                        state.next_phase = wait;
                    }
                } else {
                    /* We are in the active phase, as we have chosen a slot. */
                    if (sensor) {
//                        const auto& packet = sensor_data;
//                        state.packet_queue.push(packet);
                        state.data_packet = sensor_data;
//                        state.packet_queue.push(sensor_data);
                        sensor_data[0] = ++seq_num;
                    }

                    /* Add data size and routing information if we have a packet. */
                    short receiver = NO_RECEIVER;
                    octet gateway_distance;
                    octet data_size = 0;
//                    std::vector<octet> packet{};
//                    if (!state.packet_queue.empty()) {
//                        packet = state.packet_queue.front();
//                        state.packet_queue.pop();
//                        data_size = packet.size();
//                    }

                    if (gateway) {
                        state.gateway_distance = 0;
                    } else if (!state.data_packet.empty() && !state.neighbourhood.empty()) {
//                        data_size = state.packet_queue.head().size();
                        data_size = state.data_packet.size();

                        /* Find receiver (for routing). */
                        std::vector<short> possible_receivers{};
                        possible_receivers.push_back(state.neighbourhood.begin()->first);
                        gateway_distance = state.neighbourhood.begin()->second.gateway_distance;
                        for (auto &neighbour : state.neighbourhood) {
                            auto &node = neighbour.second;
                            if (node.gateway_distance < gateway_distance) {
                                possible_receivers.clear();
                                possible_receivers.push_back(node.id);
                                gateway_distance = node.gateway_distance;
                            } else if (node.gateway_distance == gateway_distance) {
                                possible_receivers.push_back(node.id);
                            }
                        }

                        std::uniform_int_distribution<unsigned long> selector(0ul, possible_receivers.size() - 1ul);
                        auto selected = selector(eng);
                        if (selected >= possible_receivers.size()) {
                            hardware::logger->info("selected receiver >= possible_receivers. {}, {}", selected,
                                                   possible_receivers.size());
                        }
                        receiver = possible_receivers.at(selected);
                    }

                    /* Create synchronisation signal. */
                    ControlPacket ctrl{id, state.chosen_slot,
                                       state.occupied_slots, state.gateway_distance,
                                       state.collision_slot, receiver, data_size};
                    state.collision_slot = NO_SLOT;

                    /* Send initial synchronisation signal. */
                    hardware::sleep(3ms);
                    hardware::transmit(mpilib::serialise(ctrl));
                    state.sent_first_message = true;

                    /* Send packet, if any. */
                    if (!state.data_packet.empty()) {
//                        auto data_packet = state.packet_queue.pop();
                        hardware::sleep(10ms);
//                        hardware::transmit(data_packet);
                        hardware::transmit(state.data_packet);
                        state.data_packet.clear();
                    }

                    /* Model 3: Reset all neighbourhood information after sending. */
//                    state.neighbourhood.clear();
                }
            } else if (state.phase != wait) {
                bool collision{};
                short expected_id = -1;
                for (auto &neighbour : state.neighbourhood) {
                    if (neighbour.second.selected_slot == slot) {
                        if (expected_id != -1) {
                            collision = true;
                        }
                        expected_id = neighbour.first;
                    }
                }

                /* Listen for synchronisation signal. */
                auto ctrl_data = hardware::receive(20ms);
                if (!ctrl_data.empty()) {
                    auto ctrl = mpilib::deserialise<ControlPacket>(ctrl_data);

                    if (expected_id != -1 && expected_id != ctrl.id) {
                        collision = true;
                    }

                    auto &node = state.neighbourhood[ctrl.id]; /* Will construct Node in none exists. */
                    if (node.id == 0 || node.gateway_distance == 0xFF) {
                        /* Node was just constructed. */
                        node.id = ctrl.id;
                        node.gateway_distance = ctrl.gateway_distance;
                    }
                    node.selected_slot = ctrl.current_slot;
                    node.occupied_slots = ctrl.occupied_slots;
//                    state.neighbourhood_slots |= node.occupied_slots;

                    if (ctrl.gateway_distance + 1 < state.gateway_distance) {
                        state.gateway_distance = ctrl.gateway_distance + 1;
                    }

                    /* Update occupied time slots in first order neighbourhood. */
                    state.occupied_slots[slot] = true;

                    if (ctrl.collision_slot == state.chosen_slot) {
                        state.phase = wait;
                        state.wait_frames = gen_wait();
                        state.chosen_slot = NO_CHOSEN_SLOT;
                        state.log_state_change();
                    }

                    if (state.phase == init) {
                        state.next_phase = wait;
                    } else if (state.phase == active) {
//                        state.nothing_received = false;

                        if (ctrl.destination_id == id && ctrl.data_size > 0) {
                            /* Listen for packet. */
                            auto data = hardware::receive(70ms);
                            if (!data.empty()) {
                                if (gateway) {
                                    seq_num = *data.begin();
                                    hardware::logger->info("%{}: received data packet {}", id, seq_num);
                                } else {
//                                    state.packet_queue.push(data);
                                    state.data_packet = data;
                                }
                            }
                        }
                    }
                }
//                else {
//                    if (expected_id != -1) {
//                        /* We expected to receive a packet from someone in this time slot. Possible collision. */
//                        collision = true;
//                    }
//                }

                if (collision) {
                    state.collision_slot = slot;
                }
            }
            /* Sleep for the remainder of the time slot. */
            hardware::sleep((SLOT_LENGTH * slot_count) - hardware::get_localtime());
            slot_count++;
        }

        /* Pick a time slot from information gathered through the discover phase. */
        if (state.phase == discover) {
            if (state.occupied_slots.all()) {
                /* No available slots. */
                hardware::logger->info("#{}, no available time slots!", id);
                state.next_phase = init;
            } else if (state.neighbourhood.empty()) {
                hardware::logger->info("#{}, no neighbours detected!", id);
                /* Model 8: If no neighbours was discovered in two frame lengths, do not pick a slot. */
                state.disconnected_counter++;
                if (state.disconnected_counter == 2) {
                    state.next_phase = init;
                    state.disconnected_counter = 0;
                }
            } else {
                std::bitset<SLOTS> neighbourhood_slots{};
                for (auto &neighbour : state.neighbourhood) {
                    auto &node = neighbour.second;
                    neighbourhood_slots |= node.occupied_slots;
                }

                if (neighbourhood_slots.all()) {
                    hardware::logger->info("#{}, no available time slots in second order neighbourhood!", id);
                    state.next_phase = init;
                } else {
                    std::vector<soctet> available_slots{};
                    for (soctet i = 0; i < neighbourhood_slots.size(); ++i) {
                        if (!neighbourhood_slots[i]) {
                            available_slots.push_back(i);
                        }
                    }
                    std::uniform_int_distribution<unsigned long> selector(0ul, available_slots.size() - 1ul);
                    auto selected = selector(eng);
                    if (selected >= available_slots.size()) {
                        hardware::logger->info("selected slot >= available_slots. {}, {}", selected,
                                               available_slots.size());
                    }
                    auto slot = available_slots.at(selected);
                    state.next_phase = active;
                    state.next_slot = slot;
                }
            }
        }

//        else if (state.phase == active) {
//            if (state.nothing_received) {
//                state.next_phase = wait;
//            }
//        }
    }


//    hardware::logger->info("chosen_slot={}, phase={}", state.chosen_slot, to_string(state.phase));
    hardware::deinit();

    return 0;
}
