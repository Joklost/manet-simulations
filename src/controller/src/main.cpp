
#include <vector>

#include <mpilib/mpi.h>
#include <iostream>

class Controller {
    bool work = true;

    int world_size{};
    int world_rank{};
    int name_len{};
    char processor_name[MPI_MAX_PROCESSOR_NAME]{};
    std::shared_ptr<spdlog::logger> c;
    std::vector<std::vector<octet> *> packets{};

    bool handshake();

    void die(status_t &status);

public:

    void run();
};

void Controller::run() {
    mpi_init(&world_size, &world_rank, &name_len, processor_name);

    this->c = spdlog::stdout_color_st(processor_name + std::to_string(world_rank));
    this->c->info("controller started, rank {} out of {} processors.", world_rank, world_size);

    if (!this->handshake()) {
        mpi_deinit();
        return;
    }

    while (this->work) {
        status_t status = mpi_probe_any();
        c->info("probe(source={}, tag={})", status.MPI_SOURCE, status.MPI_TAG);

        if (status.MPI_TAG == DIE) {
            this->die(status);
        } else if (status.MPI_TAG == TX_PKT) {
            //this->handle_tx(buffer, status.MPI_SOURCE, status.MPI_TAG);
            this->c->info("handle_tx(source={}, tag={})", status.MPI_SOURCE, TX_PKT);

            auto buffer = new std::vector<octet>{};
            mpi_recv(buffer, status.MPI_SOURCE, TX_PKT);

            this->packets.push_back(buffer);
            this->c->info("< rx packet of size {}:{}", buffer->size(), buffer);

        } else if (status.MPI_TAG == RX_PKT) {
            //this->handle_rx(status.MPI_SOURCE, status.MPI_TAG);
            this->c->info("handle_rx(source={}, tag={})", status.MPI_SOURCE, RX_PKT);

            unsigned long time_units;
            mpi_recv(&time_units, status.MPI_SOURCE, RX_PKT);

            unsigned long packet_count = this->packets.size();
            this->c->info("source={} listening for {} time units, responding with {} packets", status.MPI_SOURCE,
                          time_units,
                          packet_count);
            mpi_send(packet_count, status.MPI_SOURCE, RX_PKT);

            for (auto &packet : this->packets) {
                this->c->info("> tx packet of size {}:{}", packet->size(), packet);

                mpi_send(packet, status.MPI_SOURCE, RX_PKT);
            }

        } else if (status.MPI_TAG == SLEEP) {

        }
    }


    for (auto &p : this->packets) {
        delete p;
    }

    mpi_deinit();
}

bool Controller::handshake() {
    for (auto i = 1; i < world_size; ++i) {
        c->info("Handshaking with {}{}", processor_name, i);
        mpi_send(i, i, HANDSHAKE);
        int response;
        mpi_recv(&response, i, HANDSHAKE);
        //MPI_Recv(&response, 1, MPI_INT, i, HANDSHAKE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (response == i) {
            c->info("Handshaking with {}{} successful", processor_name, i);
        } else {
            c->info("Handshaking with {}{} failed", processor_name, i);
            return false;
        }
    }

    return true;
}

void Controller::die(status_t &status) {
    int reason;
    mpi_recv(&reason, status.MPI_SOURCE, DIE);
    c->info("Received DIE signal from {}{} with reason {}", processor_name, status.MPI_SOURCE, reason);
    this->work = false;
}

int main(int argc, char *argv[]) {
    Controller ctrl{};
    ctrl.run();
}

/*
void
handle_tx(const std::shared_ptr<spdlog::logger> &c, int source, int tag, std::vector<std::vector<octet>> &packets) {
    c->info("handle_tx(source={}, tag={})", source, tag);
    MPI_Status status{};
    MPI_Probe(source, tag, MPI_COMM_WORLD, &status);

    int count;
    MPI_Get_count(&status, MPI_BYTE, &count);

    auto &buffer = packets.back();

    //std::vector<octet> buffer{};
    buffer.reserve(static_cast<unsigned long>(count));

    MPI_Recv(&buffer.front(), count, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    c->info("< rx packet of size {}:{}", count, hex(buffer.data(), static_cast<unsigned long>(count)));

}

void
handle_rx(const std::shared_ptr<spdlog::logger> &c, int source, int tag, std::vector<std::vector<octet>> &packets) {
    c->info("handle_rx(source={}, tag={})", source, tag);
    unsigned long time_units;
    MPI_Recv(&time_units, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    unsigned long packet_count = packets.size();
    c->info("source={} listening for {} time units, responding with {} packets", source, time_units, packet_count);
    MPI_Send(&packet_count, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD);

    for (auto &item : packets) {
        c->info("> tx packet of size {}:{}", item.size(), hex(item.data(), item.size()));

        MPI_Send(&item.front(), static_cast<int>(item.size()), MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    std::shared_ptr<spdlog::logger> c;
    std::string controller_name{"controller"};

    MPI_Init(nullptr, nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);
    c = spdlog::stdout_color_st(processor_name + std::to_string(world_rank));
    c->info("{} started, rank {} out of {} processors.", controller_name, world_rank, world_size);



    bool loop = true;

    std::vector<std::vector<octet>> packets{};
    packets.reserve(16);

    while (loop) {
        MPI_Status status{};
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        c->info("probe(source={}, tag={})", status.MPI_SOURCE, status.MPI_TAG);

        switch (status.MPI_TAG) {
            case TX_PKT:
                packets.emplace_back();
                handle_tx(c, status.MPI_SOURCE, status.MPI_TAG, packets);
                for (auto &item : packets) {
                    c->info("> switch {}:{}", item.size(), hex(item.data(), item.size()));
                }
                break;
            case RX_PKT:
                handle_rx(c, status.MPI_SOURCE, status.MPI_TAG, packets);
                break;
            case SLEEP:
                break;
            case DIE:
                int reason;
                MPI_Recv(&reason, 1, MPI_INT, status.MPI_SOURCE, DIE, MPI_COMM_WORLD, &status);
                c->info("Received DIE signal from {}{} with reason {}", processor_name, status.MPI_SOURCE, reason);
                loop = false;
                break;
            default:
                break;
        }
    }

    MPI_Finalize();
}
*/