#ifndef MANETSIMS_OSTR_H
#define MANETSIMS_OSTR_H

#include <ostream>
#include <vector>
#include <mpilib/defines.h>

std::ostream &operator<<(std::ostream &os, std::vector<octet> buffer);

std::ostream &operator<<(std::ostream &os, std::vector<octet> *buffer);


#endif /* MANETSIMS_OSTR_H */
