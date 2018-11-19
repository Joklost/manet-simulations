#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <mpilib/httpclient.h>

HttpClient::HttpClient(std::string base_url) {
    this->host = std::move(base_url);
    this->console = spdlog::stdout_color_mt("httpclient");
}

cpr::Response HttpClient::post(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::Post(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    this->console->info("POST {}: {}", endpoint, response.status_code);
    return response;
}


cpr::AsyncResponse HttpClient::post_async(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::PostAsync(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    this->console->info("Async POST {}", endpoint);
    return response;
}

cpr::Response HttpClient::get(std::string endpoint) {
    auto response = cpr::Get(cpr::Url{this->host + endpoint});
    this->console->info("GET {}: {}", endpoint, response.status_code);
    return response;
}

cpr::AsyncResponse HttpClient::getAsync(std::string endpoint) {
    auto response = cpr::GetAsync(cpr::Url{this->host + endpoint});
    this->console->info("Async GET {}", endpoint);
    return response;
}
