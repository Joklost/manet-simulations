#include <mpilib/httpclient.h>


httpclient::httpclient(const string host, const string port) : resolver(tcp::resolver{ioc}), socket(tcp::socket{ioc}) {
    this->host = host;
    this->port = port;
}

http::response<http::dynamic_body> httpclient::get(string url) {
    /* build request */
    http::request<http::string_body> req;
    req.method(http::verb::get);
    req.target(url);

    /* connect to url */
    auto const results = this->resolver.resolve(this->host, this->port);
    boost::asio::connect(this->socket, results.begin(), results.end());

    /* send request */
    http::write(this->socket, req);

    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> resp;
    http::read(this->socket, buffer, resp);

    /* close the connection */
    this->socket.shutdown(tcp::socket::shutdown_both);
    return resp;
}

void httpclient::post(string url, json payload) {
    /* build request */
    http::request<http::string_body> req;
    req.method(http::verb::post);
    req.set(http::field::content_type, "application/json");
    req.target(url);
    req.body() = payload.dump();

    /* connect to url */
    auto const results = this->resolver.resolve(this->host, this->port);
    boost::asio::connect(this->socket, results.begin(), results.end());

    /* send request */
    http::write(this->socket, req);

    /* close the connection */
    this->socket.shutdown(tcp::socket::shutdown_both);

}
