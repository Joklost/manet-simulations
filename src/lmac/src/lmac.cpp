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

//    if (rank == 2 || rank == 3) {
//        hardware::logger->set_level(spdlog::level::debug);
//    }

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

    auto slot_count = 1;

    for (auto frame = 1ul; frame <= FRAMES; ++frame) {
//        if (gateway) {
//            hardware::logger->info("frame: {}", frame);
//        }

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
                state.next_slot = NO_CHOSEN_SLOT;
            }

            state.phase = state.next_phase;
            std::string phase_s;
            if (state.phase == active) {
                phase_s = std::to_string(state.chosen_slot);
            } else {
                phase_s = to_string(state.phase);
            }
            hardware::logger->info("{},{},{}", hardware::get_localtime(), id, phase_s);
//            hardware::logger->info("moving to {} at {}", to_string(state.phase), hardware::get_localtime());
            state.next_phase = nil;
        }

        for (auto slot = 0ul; slot < SLOTS; ++slot) {
//            auto time = hardware::get_localtime();
//            hardware::logger->info("start time {}", time.count());

            if (slot == state.chosen_slot) {
                if (state.phase == init) {
                    /* Send initial synchronisation signal. */
                    hardware::sleep(2ms);
//                    hardware::logger->info("sending init signal at slot:{}, frame:{}", slot, frame);
                    ControlPacket ctrl{id, state.chosen_slot, state.occupied_slots, -1, -1, -1, 0};
                    hardware::transmit(mpilib::serialise(ctrl));

                    if (gateway) {
                        state.next_phase = active;
                    } else {
                        state.next_phase = wait;
                    }
                } else {
                    /* We are in the active phase, as we have chosen a slot. */
                    short receiver = NO_RECEIVER;
                    soctet gateway_distance = -1;
                    /* Add data size and routing information if we have a packet. */
                    octet data_size = 0;
                    if (!state.data_packet.empty()) {
                        data_size = static_cast<octet>(state.data_packet.size());

                        /* Find receiver (for routing). */
                        receiver = state.neighbourhood.begin()->first;
                        gateway_distance = state.neighbourhood.begin()->second.gateway_distance;
                        for (auto &neighbour : state.neighbourhood) {
                            auto &node = neighbour.second;
                            if (node.gateway_distance < gateway_distance) {
                                receiver = neighbour.first;
                                gateway_distance = node.gateway_distance;
                            }
                        }
                        gateway_distance += 1;
                    }

                    /* Create synchronisation signal. */
                    ControlPacket ctrl{id, state.chosen_slot,
                                       state.occupied_slots, gateway_distance,
                                       state.collision_slot, receiver, data_size};

                    if (state.collision_slot != NO_SLOT) {
                        state.collision_slot = NO_SLOT;
                    }

                    /* Send initial synchronisation signal. */
                    hardware::sleep(2ms);
//                    hardware::logger->info("sending sync signal at slot:{}, frame:{}", slot, frame);
                    hardware::transmit(mpilib::serialise(ctrl));

                    /* Send packet, if any. */
                    if (data_size != 0) {
                        hardware::sleep(10ms);
                        hardware::logger->info("sending data packet");
                        hardware::transmit(state.data_packet);
                        state.data_packet.clear();
                    }
                }
            } else if (state.phase != wait) {
                /* Listen for synchronisation signal. */
//                hardware::logger->info("receive for sync signal at slot:{}, frame:{}", slot, frame);
                auto ctrl_data = hardware::receive(15ms);
                if (!ctrl_data.empty()) {
                    auto ctrl = mpilib::deserialise<ControlPacket>(ctrl_data);
//                    hardware::logger->info("received sync signal at slot:{}, frame:{} from:{}", slot, frame, ctrl.id);

                    auto &node = state.neighbourhood[ctrl.id]; /* Will construct Node in none exists. */
                    if (node.id == 0 || node.gateway_distance == -1) {
                        /* Node was just constructed. */
                        node.id = ctrl.id;
                        node.gateway_distance = ctrl.gateway_distance;
                    }
                    node.current_slot = ctrl.current_slot;
                    node.occupied_slots = ctrl.occupied_slots;
                    if (node.gateway_distance > ctrl.gateway_distance) {
                        /* Update routing information. */
                        node.gateway_distance = ctrl.gateway_distance;
                    }

                    /* Update occupied time slots in second order neighbourhood. */
                    state.occupied_slots |= node.occupied_slots;

//                    auto neighbour = state.neighbourhood.find(ctrl.id);
//                    if (neighbour == state.neighbourhood.end()) {
//                        /* Add node to neighbourhood information. */
//                        Node n{ctrl.id, ctrl.current_slot, ctrl.occupied_slots, ctrl.gateway_distance};
//                        state.neighbourhood[ctrl.id] = n;
//
//                    } else {
//                        /* Update information on neighbour. */
//                        auto &node = neighbour->second;
//                        node.current_slot = ctrl.current_slot;
//                        node.occupied_slots = ctrl.occupied_slots;
//                        if (node.gateway_distance > ctrl.gateway_distance) {
//                            /* Update routing information. */
//                            node.gateway_distance = ctrl.gateway_distance;
//                        }
//
//                        state.occupied_slots |= node.occupied_slots;
//                    }

                    if (ctrl.collision_slot == state.chosen_slot) {
                        state.phase = wait;
                        hardware::logger->info("moving to {} due to collision", to_string(state.phase));
                        state.wait_frames = gen_wait();
                        state.chosen_slot = NO_CHOSEN_SLOT;
                    } else {
//                        auto colliding_slots = ctrl.occupied_slots & state.occupied_slots;
//                        if (colliding_slots.count()) {
//                            hardware::logger->info("{}, {}", ctrl.occupied_slots, state.occupied_slots);
//                            hardware::logger->info("collision detected: {}", colliding_slots);
//                            for (soctet i = 0; i < colliding_slots.size(); ++i) {
//                                if (colliding_slots[i]) {
//                                    state.collision_slot = i;
//                                }
//                            }
                    }

//                        state.occupied_slots |= ctrl.occupied_slots;

                    if (state.phase == init) {
                        state.next_phase = wait;
                    } else if (state.phase == active) {
                        if (ctrl.destination_id == id && ctrl.data_size > 0) {
                            /* Listen for packet. */
                            auto data = hardware::receive(70ms);
                            if (!data.empty()) {
                                hardware::logger->info("received data packet");
                                state.data_packet = data;
                            }
                        }
                    }
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
                std::vector<soctet> available_slots{};

                for (soctet i = 0; i < state.occupied_slots.size(); ++i) {
                    if (!state.occupied_slots[i]) {
                        available_slots.push_back(i);
                    }
                }

                std::uniform_int_distribution<unsigned long> selector(0ul, available_slots.size() - 1ul);
                auto slot = selector(eng);
                state.next_phase = active;
                state.next_slot = slot;
            }

        }
    }


//    hardware::logger->info("chosen_slot={}, phase={}", state.chosen_slot, to_string(state.phase));
    hardware::deinit();

    return 0;
}