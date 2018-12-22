#include <string>

#include <mpilib/hwglobals.h>

std::shared_ptr<spdlog::logger> hardware::logger = nullptr;
unsigned long hardware::localtime{};
bool hardware::initialized{};
int hardware::world_size{};
int hardware::world_rank{};
std::string hardware::processor_name{};
