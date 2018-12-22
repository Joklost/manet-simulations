#include <string>

#include <mpilib/hwglobals.h>

unsigned long hardware::my_time{};
bool hardware::initialized{};
int hardware::world_size{};
int hardware::world_rank{};
std::string hardware::processor_name{};
