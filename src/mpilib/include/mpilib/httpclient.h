#ifndef MANETSIMS_HTTPCLIENT_H
#define MANETSIMS_HTTPCLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>


class HttpClient {
    std::string host;
    std::shared_ptr<spdlog::logger> console;

public:
    explicit HttpClient(std::string base_url);

    cpr::Response post(std::string endpoint, nlohmann::json &payload);

    void post_async(std::string endpoint, nlohmann::json &payload);
};


#endif //MANETSIMS_HTTPCLIENT_H
