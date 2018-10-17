#ifndef MANETSIMS_HTTPCLIENT_H
#define MANETSIMS_HTTPCLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <nlohmann/json.hpp>

using std::string;
using nlohmann::json;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class httpclient {
    string host;
    string port;
    tcp::socket socket;
    tcp::resolver resolver;
    boost::asio::io_context ioc{};

    httpclient(string host, string port);

    http::response<http::dynamic_body> get(string url);

    void post(string url, json payload);
};

#endif //MANETSIMS_HTTPCLIENT_H
