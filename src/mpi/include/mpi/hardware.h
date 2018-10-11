#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <vector>
#include <memory>
#include <iostream>
#include "../src/hardware_impl.h"

template<class P>
class Hardware {
public:
    explicit Hardware();

    ~Hardware();

    // Move
    Hardware(Hardware<P> &&rhs) noexcept = default;

    // Move assignment
    Hardware<P> &operator=(Hardware<P> &&rhs) noexcept = default;

    // Copy
    Hardware(const Hardware<P> &rhs);

    // Copy assignment
    Hardware<P> &operator=(const Hardware<P> &rhs);

    void broadcast(P *packet) const;

    std::vector<P *> listen(int time) const;

    void sleep(int time) const;

private:
    const HardwareImpl<P> *p_impl() const { return this->impl.get(); }

    HardwareImpl<P> *p_impl() { return this->impl.get(); };

    std::unique_ptr<HardwareImpl<P>> impl;
};


///////////////////////////////////////////////////////


template<class P>
Hardware<P>::Hardware() : impl(new HardwareImpl<P>()) {}

template<class P>
Hardware<P>::Hardware(const Hardware &rhs) : impl(new HardwareImpl(*rhs.impl)) {}

template<class P>
Hardware<P> &Hardware<P>::operator=(const Hardware &rhs) {
    if (this != &rhs) {
        this->impl.reset(new HardwareImpl(*rhs.impl));
    }

    return *this;
}

template<class P>
void Hardware<P>::broadcast(P *packet) const {
    this->p_impl()->broadcast(packet);
}

template<class P>
std::vector<P *> Hardware<P>::listen(int time) const {
    return this->p_impl()->listen(time);
}

template<class P>
void Hardware<P>::sleep(int time) const {
    this->p_impl()->sleep(time);
}

template<class P>
Hardware<P>::~Hardware() = default;

///////////////////////////////////////////////////////


#endif /* MANETSIMS_HW_H */
