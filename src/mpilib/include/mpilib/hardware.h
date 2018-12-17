#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <iostream>

#include "../../src/interface.h"

template<typename T>
static Interface<T> interface{};

template<typename T>
void init_hardware() {
    interface<T>.init();
}

template<typename T>
void deinit_hardware() {
    interface<T>.deinit();
}

template<typename T>
void tx(T &packet) {
    interface<T>.tx(packet);
}

template<typename T>
std::vector<T> rx(unsigned long time) {
    return interface<T>.rx(time);
}

template<typename T>
void sleep(unsigned long time) {
    interface<T>.sleep(time);
}


#endif /* MANETSIMS_HW_H */
