#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <mpilib/httpclient.h>

HttpClient::HttpClient(std::string base_url) {
    this->host = std::move(base_url);
    this->console = spdlog::stdout_color_mt("console");
}

cpr::Response HttpClient::post(std::string endpoint, nlohmann::json payload) {
    auto response = cpr::Post(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    this->console->info("POST {}: {}", endpoint, response.status_code);
    return response;
}

