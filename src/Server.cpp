#include "Server.hpp"

using namespace srv;

Server::Server(int port, size_t threads)
    : acceptor(io_context,
               asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      thread_count(threads == 0 ? 4 : threads),
      thread_pool(thread_count)
{}

void Server::response(asio::ip::tcp::socket& socket,
                      const std::string& data)
{
    asio::error_code ec;
    asio::write(socket, asio::buffer(data), ec);

    if (ec) {
        std::cerr << "Write error: " << ec.message() << std::endl;
    }
}

void Server::handleClient(asio::ip::tcp::socket socket)
{
    try {
        char buffer[4096];
        asio::error_code ec;

        size_t bytes = socket.read_some(asio::buffer(buffer), ec);
        if (ec) {
            std::cerr << "Read error: " << ec.message() << std::endl;
            return;
        }

        std::string raw(buffer, bytes);

        Request req;
        req.deserializing(raw);

        thread_pool.enqueue(
            [this, socket = std::move(socket), req]() mutable {
                try {
                    if (!router.hasRoute(req.method, req.target)) {
                        Response res;
                        res.status = 404;
                        Struct data;
                        data.set("message", "Not Found");
                        response(socket, res.json(data));
                        return;
                    }

                    Handler handler = router.getHandler(req.method, req.target);
                    if (!handler) {
                        Response res;
                        res.status = 500;
                        Struct data;
                        data.set("message", "Handler not found");
                        response(socket, res.json(data));
                        return;
                    }

                    Response res;
                    res.status = 200;
                    Struct result = handler(req, res);
                    response(socket, res.json(result));

                } catch (const std::exception& e) {
                    Response res;
                    res.status = 500;
                    Struct data;
                    data.set("message", e.what());
                    response(socket, res.json(data));
                }
            }
        );

    } catch (const std::exception& e) {
        std::cerr << "Client handling error: " << e.what() << std::endl;
    }
}

void Server::run(const std::string& text)
{
    std::cout << text << std::endl;
    std::cout << "Server running with "
              << thread_count
              << " worker threads" << std::endl;

    while (true) {
        asio::ip::tcp::socket socket(io_context);
        asio::error_code ec;

        acceptor.accept(socket, ec);
        if (ec) {
            std::cerr << "Accept error: " << ec.message() << std::endl;
            continue;
        }

        auto endpoint = socket.remote_endpoint(ec);
        if (!ec) {
            std::cout << "New connection from "
                      << endpoint.address().to_string()
                      << ":" << endpoint.port() << std::endl;
        }

        handleClient(std::move(socket));
    }
}
