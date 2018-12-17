#ifndef MANETSIMS_INTERFACE_H
#define MANETSIMS_INTERFACE_H

#include <vector>
#include <mpilib/mpi.h>

template<typename T>
class Interface {
private:
    bool initialized{};

    int world_size{};
    int world_rank{};
    std::string processor_name;
    std::shared_ptr<spdlog::logger> c;

    /* TODO: Remove temporary packet storage. */
    std::vector<octetarray<T>> p{};

public:

    Interface() = default;

    void init() {
        if (initialized) {
            return;
        }
        initialized = true;

        MPI_Init(nullptr, nullptr);

        MPI_Comm_size(MPI_COMM_WORLD, &this->world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &this->world_rank);

        int name_len{};
        char name[MPI_MAX_PROCESSOR_NAME];
        MPI_Get_processor_name(name, &name_len);
        this->processor_name = name + std::to_string(world_rank);
        this->c = spdlog::stdout_color_st(processor_name);
        this->c->info("{} started, rank {} out of {} processors.", processor_name, world_rank, world_size);

        /* Handshake */
        int number;
        MPI_Recv(&number, 1, MPI_INT, CONTROLLER, HANDSHAKE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (MPI_Send(&number, 1, MPI_INT, CONTROLLER, HANDSHAKE, MPI_COMM_WORLD) != MPI_SUCCESS) {
            this->c->info("Failed initialization handshake!");
            deinit();
            return;
        }
    }

    void deinit() {
        if (!initialized) {
            return;
        }

        /* TODO: Remove */
        int reason = 1;
        MPI_Send(&reason, 1, MPI_INT, CONTROLLER, DIE, MPI_COMM_WORLD);

        MPI_Finalize();

        initialized = false;
    }

    void tx(T &packet) {
        /* Step 1: Serialise packet. */
        /* Step 2: Send packet to controller. */

        octetarray<T> buffer = serialise(packet);
        this->c->debug("> tx->ctrl packet of size {}", buffer.size());

        MPI_Send(&buffer.front(), static_cast<int>(buffer.size()),
                 MPI_UNSIGNED_CHAR, CONTROLLER, TX_PKT, MPI_COMM_WORLD);
    }

    std::vector<T> rx(unsigned long time) {
        /* Step 1: Message controller; I'm ready to receive. */
        /* Step 2: Receive messages from controller, if any. */
        /* Step 3: Deserialise messages, and return them. */

        std::vector<T> packets{};

        MPI_Send(&time, 1, MPI_UNSIGNED_LONG, CONTROLLER, RX_PKT, MPI_COMM_WORLD);

        unsigned long packet_count = packets.size();
        MPI_Recv(&packet_count, 1, MPI_UNSIGNED_LONG, CONTROLLER, RX_PKT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (auto i = 0; i < packet_count; ++i) {
            octetarray<T> buffer{};
            T packet{};
            MPI_Recv(&buffer.front(), static_cast<int>(buffer.size()),
                     MPI_UNSIGNED_CHAR, CONTROLLER, RX_PKT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            c->debug("< rx<-ctrl packet of size {}", buffer.size());
            packets.emplace_back(deserialise<T>(buffer, packet));
        }

/*
        for (const auto &data : this->p) {
            this->c->info("< rx packet of size {}", data.size());
            T item{};
            deserialise(data, item);
            packets.emplace_back(item);
        }

        this->p.clear();
*/

        return packets;
    }

    void sleep(unsigned long time) {

    }

};


#endif //MANETSIMS_INTERFACE_H
