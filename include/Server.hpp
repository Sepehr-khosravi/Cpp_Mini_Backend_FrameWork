#pragma once

#include <asio.hpp>
#include <iostream>
#include <thread>

#include "Request.hpp"
#include "Router.hpp"
#include "handler.hpp"
#include "Value.hpp"
#include "Struct.hpp"
#include "ThreadPool.hpp"

namespace srv {

class Server {
private:
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;

    size_t thread_count;
    ThreadPool thread_pool;

    void response(asio::ip::tcp::socket& socket, const std::string& data);
    void handleClient(asio::ip::tcp::socket socket);

public:
    Router router;

    explicit Server(int port = 8080,
                    size_t threads = std::thread::hardware_concurrency());

    void run(const std::string& text);
};

}
