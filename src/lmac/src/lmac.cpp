#include <bitset>

#include <mpilib/random.h>
#include <mpilib/interface.h>
#include "models.h"

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
        state.occupied_slots[0] = true;
    }

    auto slot_count = 1;

    for (auto frame = 0ul; frame < FRAMES; ++frame) {
        hardware::logger->info("frame: {}", frame);

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

            state.phase = state.next_phase;
            hardware::logger->info("moving to {}", phase_string[state.phase]);
            state.next_phase = nil;
        }

        for (auto slot = 0ul; slot < SLOTS; ++slot) {
//            auto time = hardware::get_localtime();
//            hardware::logger->info("start time {}", time.count());

            if (slot == state.chosen_slot) {
                if (state.phase == init) {
                    /* Send initial synchronisation signal. */
                    hardware::sleep(2ms);
                    hardware::logger->info("sending init signal");
                    ControlPacket ctrl{id, state.chosen_slot, state.occupied_slots, -1, -1, -1, 0};
                    hardware::broadcast(mpilib::serialise(ctrl));

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

                    /* Send initial synchronisation signal. */
                    hardware::sleep(2ms);
                    hardware::logger->info("sending sync signal");
                    hardware::broadcast(mpilib::serialise(ctrl));

                    /* Send packet, if any. */
                    if (data_size != 0) {
                        hardware::sleep(100ms);
                        hardware::logger->info("sending data packet");
                        hardware::broadcast(state.data_packet);
                        state.data_packet.clear();
                    }
                }
            } else {
                /* Listen for synchronisation signal. */
                auto ctrl_data = hardware::listen(20ms);
                if (!ctrl_data.empty()) {
                    hardware::logger->info("received sync signal");
                    auto ctrl = mpilib::deserialise<ControlPacket>(ctrl_data);

                    auto neighbour = state.neighbourhood.find(ctrl.id);
                    if (neighbour == state.neighbourhood.end()) {
                        /* Add node to neighbourhood information. */
                        Node n{ctrl.id, ctrl.current_slot, ctrl.occupied_slots, ctrl.gateway_distance};
                        state.neighbourhood[ctrl.id] = n;
                    } else {
                        /* Update information on neighbour. */
                        auto &node = neighbour->second;
                        node.current_slot = ctrl.current_slot;
                        node.occupied_slots = ctrl.occupied_slots;
                        if (node.gateway_distance > ctrl.gateway_distance) {
                            /* Update routing information. */
                            node.gateway_distance = ctrl.gateway_distance;
                        }
                    }

                    if (ctrl.collision_slot == state.chosen_slot) {
                        state.phase = wait;
                        hardware::logger->info("moving to {}", phase_string[state.phase]);
                        state.wait_frames = gen_wait();
                        state.chosen_slot = NO_CHOSEN_SLOT;
                    } else {
                        auto colliding_slots = ctrl.occupied_slots & state.occupied_slots;
                        if (colliding_slots.count()) {
                            hardware::logger->info("collision detected: {}", colliding_slots);
                            for (soctet i = 0; i < colliding_slots.size(); ++i) {
                                if (colliding_slots[i]) {
                                    state.collision_slot = i;
                                }
                            }
                        }

                        state.occupied_slots |= ctrl.occupied_slots;

                        if (state.phase == init) {
                            state.next_phase = wait;
                        } else if (state.phase == active) {
                            if (ctrl.destination_id == id && ctrl.data_size > 0) {
                                /* Listen for packet. */
                                auto data = hardware::listen(800ms);
                                if (!data.empty()) {
                                    hardware::logger->info("received data packet");
                                    state.data_packet = data;
                                }
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
                state.chosen_slot = slot;
            }

        }
    }

    hardware::logger->info("chosen_slot={}, phase={}", state.chosen_slot, phase_string[state.phase]);
    hardware::deinit();

    return 0;
}