#include <reachi/cholesky.h>
#include <algorithm>

linkmap cholesky(const linkmap &matrix) {
    linkmap res;

    std::for_each(matrix.cbegin(), matrix.cend(), [&res](auto element) {

    });

    // step 1: create std::map. key = link.id : val = link
    // step 2: if link distance is larger than threshold, remove it
    // step 3: sequential iteration, doing calculations

    return res;
}

