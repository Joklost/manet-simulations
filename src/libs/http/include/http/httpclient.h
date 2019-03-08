#ifndef HTTP_HTTPCLIENT_H
#define HTTP_HTTPCLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace http {

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

#endif //HTTP_HTTPCLIENT_H
