#ifndef MANETSIMS_HELPERS_H
#define MANETSIMS_HELPERS_H

#define LENGTH 8

template<typename T>
void log_packet(std::shared_ptr<spdlog::logger> &c, std::vector<T> *packet) {
    for (auto i = 0; i < (packet->size() / LENGTH) + 1; ++i) {
        if (((LENGTH * i + LENGTH) > packet->size())) {
            c->info("---{}", std::vector<T>{packet->begin() + (LENGTH * i), packet->end()});
        } else {
            c->info("---{}", std::vector<T>{packet->begin() + (LENGTH * i), packet->begin() + ((LENGTH * i) + LENGTH)});
        }
    }
}

#endif /* MANETSIMS_HELPERS_H */
