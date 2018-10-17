#include <future>

#include "node.h"

int main() {
    Hardware<LmacPacket> hw{};

    LmacNode node1{1, hw, {57.01266813458001, 9.994625734716218}, true};
    LmacNode node2{2, hw, {57.01266813458001, 9.9929758}, false};
    LmacNode node3{3, hw, {57.0117698, 9.9929758}, false};
    LmacNode node4{4, hw, {57.0117698, 9.994625734716218}, false};

    node1.start();
    node2.start();
    node3.start();
    node4.start();

    node1.join();
    node2.join();
    node3.join();
    node4.join();

    /* auto packet_sequence = listen(3); *//* listen for 3 time units *//*
    auto packets = unpack(packet_sequence);
    delete[] packet_sequence;

    for (auto packet : packets) {

        printf("packet:: ");
        for (uint8_t i = 0; i < packet->size; i++) {
            printf("0x%02x, ", packet->data[i]);
        }
        printf("\n");

        delete[] packet->data;
        delete packet;
    }

    return 0;*/
}

