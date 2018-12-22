#ifndef MANETSIMS_REACHI_OSTR_H
#define MANETSIMS_REACHI_OSTR_H

#include <ostream>

#include <mpilib/ostr.h>

#include "node.h"

std::ostream &operator<<(std::ostream &os, const std::pair <Node, Node> &obj);

#endif /* MANETSIMS_REACHI_OSTR_H */
