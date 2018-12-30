#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <mpilib/httpclient.h>
#if 0
mpilib::HttpClient::HttpClient(std::string base_url) {
    this->host = std::move(base_url);
}

cpr::Response mpilib::HttpClient::post(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::Post(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    return response;
}


cpr::AsyncResponse mpilib::HttpClient::post_async(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::PostAsync(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    return response;
}

cpr::Response mpilib::HttpClient::get(std::string endpoint) {
    auto response = cpr::Get(cpr::Url{this->host + endpoint});
    return response;
}

cpr::AsyncResponse mpilib::HttpClient::get_async(std::string endpoint) {
    auto response = cpr::GetAsync(cpr::Url{this->host + endpoint});
    return response;
}
#endif