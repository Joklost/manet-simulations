#ifndef MANETSIMS_HTTPCLIENT_H
#define MANETSIMS_HTTPCLIENT_H
#if 0
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>

namespace mpilib {

    class HttpClient {
        std::string host;

    public:
        explicit HttpClient(std::string base_url);

        cpr::Response get(std::string endpoint);

        cpr::AsyncResponse get_async(std::string endpoint);

        cpr::Response post(std::string endpoint, nlohmann::json &payload);

        cpr::AsyncResponse post_async(std::string endpoint, nlohmann::json &payload);
    };

}
#endif

#endif //MANETSIMS_HTTPCLIENT_H
