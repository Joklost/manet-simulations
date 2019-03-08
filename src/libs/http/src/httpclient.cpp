#include <http/httpclient.h>

http::HttpClient::HttpClient(std::string base_url) {
    this->host = std::move(base_url);
}

cpr::Response http::HttpClient::post(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::Post(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    return response;
}


cpr::AsyncResponse http::HttpClient::post_async(std::string endpoint, nlohmann::json &payload) {
    auto response = cpr::PostAsync(cpr::Url{this->host + endpoint},
                              cpr::Body{payload.dump()},
                              cpr::Header{{"Content-Type", "application/json"}});
    return response;
}

cpr::Response http::HttpClient::get(std::string endpoint) {
    auto response = cpr::Get(cpr::Url{this->host + endpoint});
    return response;
}

cpr::AsyncResponse http::HttpClient::get_async(std::string endpoint) {
    auto response = cpr::GetAsync(cpr::Url{this->host + endpoint});
    return response;
}
