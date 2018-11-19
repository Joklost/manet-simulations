#include <reachi/cholesky.h>
#include <algorithm>
#include <iostream>

/*LinkMap cholesky(LinkMap &linkmap) {
    LinkMap res;

    for (auto &item : linkmap) {
        auto keypair = item.first;
        auto sum = 0.0;
        //auto row_pairs1 = linkmap.get_keypairs(keypair.first, keypair.second.get_id());
        //auto row_pairs2 = linkmap.get_keypairs(keypair.second, keypair.second.get_id());


        *//* std::for_each(row_pairs1.begin(), row_pairs1.end(), [&sum, &res, &keypair](auto element) {
            auto t1 = res.get(keypair.first, element.first.second);
            auto t2 = res.get(keypair.second, element.first.second);
            // auto tmp = res.get(element.first.first, element.first.second);
            sum += t1 * t2;
        }); *//*

        *//*for (int i = 0; i < row_pairs1.size(); ++i) {
            auto t1 = row_pairs1[i].second;
            auto t2 = row_pairs2[i].second;
            sum += t1 * t2;

        }*//*

        if (keypair.first.get_id() == keypair.second.get_id()) {
            res.emplace(keypair.first, keypair.second,
                        std::sqrt(linkmap.get(keypair.first, keypair.second) - sum));
        } else {
            res.emplace(keypair.first, keypair.second, 1 / res.get(keypair.second, keypair.second) *
                                                       (linkmap.get(keypair.first, keypair.second) - sum));
        }
    }

    return res;
}*/

/*LinkMap cholesky(LinkMap &linkmap) {
    LinkMap res;
    auto size = (--linkmap.end())->first.first.get_id();

    for (uint32_t i = 0; i <= size; i++) {
        auto chunk = linkmap.get_keypairs(i);

        for (uint32_t j = 0; j < chunk.size(); ++j) {
            auto sum = 0.0;
            auto keypair = chunk.at(j).first;

            for (uint32_t k = 0; k < j; ++k) {
                auto t1 = res.get(keypair.first, chunk.at(k).first.second);
                auto t2 = res.get(keypair.second, chunk.at(k).first.second);
                sum += t1 * t2;
            }

            if (keypair.first.get_id() == keypair.second.get_id()) {
                auto math = std::sqrt(linkmap.get(keypair.first, keypair.second) - sum);
                res.emplace(keypair.first, keypair.second, math);
            } else {
                auto math = 1.0 / res.get(keypair.second, keypair.second) *
                            (linkmap.get(keypair.first, keypair.second) - sum);
                res.emplace(keypair.first, keypair.second, math);
            }

        }
    }
    return res;
}*/

LinkMap cholesky(LinkMap &linkmap) {
    LinkMap res;

    for (const auto &element : linkmap) {
        auto sum = 0.0;
        auto keypair = element.first;

        for (const auto &column : res.get_keypairs(keypair.first)) {
            sum += res.get(keypair.first, column) * res.get(keypair.second, column);
        }

        if (keypair.first == keypair.second) {
            auto value = std::sqrt(linkmap.get(keypair.first, keypair.second) - sum);
            res.emplace(keypair.first, keypair.second, value);
        } else {
            auto value = 1.0 / res.get(keypair.second, keypair.second) *
                         (linkmap.get(keypair.first, keypair.second) - sum);
            res.emplace(keypair.first, keypair.second, value);
        }
    }

    return res;
}

/* vecvec<T> slow_cholesky(const vecvec<T> matrix) {
    auto size = matrix.size();
    vecvec<T> vec{};
    vec.resize(size, std::vector<T>(size));

    for (auto i = 0; i < size; i++) {
        for (auto j = 0; j < (i + 1); j++) {
            T sum = (T) 0;
            for (auto k = 0; k < j; k++) {
                sum += vec[i][k] * vec[j][k];
            }

            if (i == j) {
                vec[i][j] = std::sqrt(matrix[i][i] - sum);
            } else {
                vec[i][j] = ((T) 1) / vec[j][j] * (matrix[i][j] - sum);
            }

        }
    }

    return vec;
}
*/