#include <iostream>
#include <chrono>

#include "node.h"

void Node::start() {
    std::cout << "Starting Node " << this->id << std::endl;
    this->protocol_thread = std::thread(&Node::protocol, this);
}

void Node::protocol() {

    if (this->gateway) {
        std::cout << "Sending Node " << this->id << std::endl;
        auto *p = new LmacPacket();
        p->message = this->id;
        this->hardware.broadcast(p);
    } else {
        std::cout << "Listening Node " << this->id << std::endl;
        auto packets = this->hardware.listen(1);
        for (auto *packet : packets) {
            std::cout << "Receiving Node " << this->id << std::endl;
            std::cout << packet->message << std::endl;
            delete packet;
        }
    }

    std::cout << "Returning Node " << this->id << std::endl;


    /*if (!this->gateway) {
        auto packets = this->hardware.listen(100);
        for (LmacPacket *packet : packets) {
            std::cout << "Receiving Node " << this->id << std::endl;
            std::cout << packet->message << std::endl;
            delete packet;
        }

    } else {

        std::cout << "Gateway Node " << this->id << std::endl;

        for (int i = 0; i < 1; i++) {
            auto *p = new LmacPacket();
            p->message = 42;
            this->hardware.broadcast(p);
        }
    }*/
}

Node::Node(int id, Hardware<LmacPacket> hardware, Location location, bool gateway) {
    this->hardware = std::move(hardware);
    this->id = id;
    //this->id = register_node(Location.latitude, Location.longitude);
    this->location = location;
    this->state = initialization;
    this->gateway = gateway;
}

void Node::join() {
    this->protocol_thread.join();
}

