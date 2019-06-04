#include <bitset>

#include <mpilib/random.h>
#include <mpilib/interface.h>
#include "models.h"

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

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    hardware::init(debug);
    auto id = static_cast<short>(hardware::get_id());
    auto rank = hardware::get_world_rank();

    std::random_device rd{};
    std::default_random_engine eng{rd()};
    std::uniform_int_distribution<unsigned long> dist{1ul, MAX_WAIT};
    auto gen_wait = std::bind(dist, eng);

    State state{};
    auto gateway = false;
    if (rank == 1) {
        gateway = true;
        state.chosen_slot = 0;
        state.next_slot = 0;
        state.occupied_slots[0] = true;
    }

    auto sensor = false;
    std::vector<octet> sensor_data{};
    if (rank == 16) {
        sensor = true;
        sensor_data = {0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
                       0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01,
                       0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF};
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
                state.wait_frames = gen_wait();
            }

            if (state.next_phase == active) {
                state.chosen_slot = state.next_slot;
                state.occupied_slots[state.chosen_slot] = true;
                state.next_slot = NO_CHOSEN_SLOT;
            }

            state.phase = state.next_phase;
            std::string phase_s;
            if (state.phase == active) {
                phase_s = std::to_string(state.chosen_slot);
            } else {
                phase_s = to_string(state.phase);
            }

            /* Log state change. */
            hardware::logger->info("{},{},{}", hardware::get_localtime().count() / 1000.0, id, phase_s);
            state.next_phase = nil;
        }

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
                    if (sensor) {
                        state.data_packet = sensor_data;
                    }

                    /* We are in the active phase, as we have chosen a slot. */
                    short receiver = NO_RECEIVER;
                    octet gateway_distance;
                    /* Add data size and routing information if we have a packet. */
                    octet data_size = 0;

                    if (gateway) {
                        state.gateway_distance = 0;
                    } else if (!state.data_packet.empty()) {
                        data_size = static_cast<octet>(state.data_packet.size());

                        /* Find receiver (for routing). */
                        std::vector<short> possible_receivers{};
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
                        receiver = possible_receivers[selector(eng)];
                    }

                    /* Create synchronisation signal. */
                    ControlPacket ctrl{id, state.chosen_slot,
                                       state.occupied_slots, state.gateway_distance,
                                       state.collision_slot, receiver, data_size};
//                    hardware::logger->info("send: {}", ctrl);
                    state.collision_slot = NO_SLOT;

                    /* Send initial synchronisation signal. */
                    hardware::sleep(3ms);
                    hardware::transmit(mpilib::serialise(ctrl));

                    /* Send packet, if any. */
                    if (data_size != 0) {
                        hardware::sleep(10ms);
//                        hardware::logger->info("%{}: sending data packet to {}, gateway_distance={}", id, receiver, gateway_distance);
                        hardware::transmit(state.data_packet);
                        state.data_packet.clear();
                    }
                }
            } else if (state.phase != wait) {
                bool collision{};
                short expected_id = -1;
                for (auto &neighbour : state.neighbourhood) {
                    if (neighbour.second.selected_slot == slot) {
                        if (expected_id != -1) {
//                            hardware::logger->info("neighbour_id={}, expected_id={}", neighbour.first, expected_id);
                            collision = true;
                        }
                        expected_id = neighbour.first;
                    }
                }

                /* Listen for synchronisation signal. */
                auto ctrl_data = hardware::receive(20ms);
                if (!ctrl_data.empty()) {
                    auto ctrl = mpilib::deserialise<ControlPacket>(ctrl_data);
//                    hardware::logger->info("recv: {}", ctrl);

                    if (expected_id != -1 && expected_id != ctrl.id) {
//                        hardware::logger->info("received_id={}, expected_id={}", ctrl.id, expected_id);
                        collision = true;
                    }

//                    auto gateway_distance = ctrl.gateway_distance + 1;

                    auto &node = state.neighbourhood[ctrl.id]; /* Will construct Node in none exists. */
                    if (node.id == 0 || node.gateway_distance == 0xFF) {
                        /* Node was just constructed. */
                        node.id = ctrl.id;
                        node.gateway_distance = ctrl.gateway_distance;
                    }
                    node.selected_slot = ctrl.current_slot;
                    node.occupied_slots = ctrl.occupied_slots;

                    if (ctrl.gateway_distance + 1 < state.gateway_distance) {
                        state.gateway_distance = ctrl.gateway_distance + 1;
                    }

                    /* Update occupied time slots in second order neighbourhood. */
                    state.occupied_slots[slot] = true;

                    if (ctrl.collision_slot == state.chosen_slot) {
                        state.phase = wait;
//                        hardware::logger->info("moving {} to {} due to collision", id, to_string(state.phase));
                        hardware::logger->info("{},{},{}", hardware::get_localtime().count() / 1000.0, id,
                                               to_string(state.phase));
                        state.wait_frames = gen_wait();
                        state.chosen_slot = NO_CHOSEN_SLOT;
                    }

                    if (state.phase == init) {
                        state.next_phase = wait;
                    } else if (state.phase == active) {
                        if (ctrl.destination_id == id && ctrl.data_size > 0) {
                            /* Listen for packet. */
                            auto data = hardware::receive(70ms);
                            if (!data.empty()) {
//                                hardware::logger->info("%{}: received data packet", id);
                                if (gateway) {
                                    hardware::logger->info("%{}: received data packet", id);
                                } else {
                                    state.data_packet = data;
                                }
                            }
                        }
                    }
                } else {
                    if (expected_id != -1) {
//                        hardware::logger->info("nothing received, expected_id={}", expected_id);
                        /* We expected to receive a packet from someone in this time slot. Possible collision. */
//                        collision = true;
                    }
                }

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
                hardware::logger->info("no available time slots!");
                state.next_phase = init;
            } else if (state.neighbourhood.empty()) {
                /* No neighbourhood, node is not connected with anyone. */
                hardware::logger->info("no neighbours detected!");
                state.next_phase = init;
            } else {
//                hardware::logger->info("%{} picking      : {}", id, state.occupied_slots);
                std::bitset<SLOTS> neighbourhood_slots{};
                for (auto &neighbour : state.neighbourhood) {
                    auto &node = neighbour.second;
//                    hardware::logger->info("%{}              : {}", node.id, node.occupied_slots);
                    neighbourhood_slots |= node.occupied_slots;
                }

//                hardware::logger->info("%{} neighbourhood: {}", id, neighbourhood_slots);

                std::vector<soctet> available_slots{};
                for (soctet i = 0; i < neighbourhood_slots.size(); ++i) {
                    if (!neighbourhood_slots[i]) {
                        available_slots.push_back(i);
                    }
                }

                std::uniform_int_distribution<unsigned long> selector(0ul, available_slots.size() - 1ul);
                auto slot = available_slots[selector(eng)];

//                hardware::logger->info("%{} picked: {}", id, slot);

                state.next_phase = active;
                state.next_slot = slot;
            }

        }
    }


//    hardware::logger->info("chosen_slot={}, phase={}", state.chosen_slot, to_string(state.phase));
    hardware::deinit();

    return 0;
}
