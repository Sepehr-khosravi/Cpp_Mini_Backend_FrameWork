#include "Server.hpp"

//error exceptions
#include <stdexcept>

//getting path
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
                        res.render("/external/core/src/notfound.html");
                        std::string notFoundHtml = res.load();
                        // FIX: Convert string literal to std::string
                        data.set("message", std::string("Not Found"));
                        response(socket, res.send(data, notFoundHtml));
                        return;
                    }

                    Response res;
                    Middleware middleware = router.getMiddleware(req.method, req.target);

                    res.status = 200;
                    bool resultMiddleware = middleware(req, res);

                    if(!resultMiddleware){
                        if(res.status == 200) res.status = 400;
                        Struct data;
                        // FIX: Convert string literal to std::string
                        data.set("message", std::string("Request rejected by middleware"));
                        response(socket, res.json(data));
                        return;
                    }

                    Handler handler = router.getHandler(req.method, req.target);
                    if (!handler) {
                        res.status = 500;
                        Struct data;
                        // FIX: Convert string literal to std::string
                        data.set("message", std::string("Handler not found"));
                        response(socket, res.json(data));
                        return;
                    }

                    res.status = 200;
                    std::optional<Struct> result = handler(req, res);

                    // Checking does res have page for loading?
                    if(res.hasPage){
                        res.headers.set("Content-Type", "text/html");
                        res.headers.set("charset", "utf-8");
                        if(res.checkPageError()){
                            res.hasPage = false;
                        }
                        std::string html = res.load();
                        // FIX: Check if result has value before sending
                        if (result.has_value()) {
                            response(socket, res.send(result.value(), "\n" + html + "\n"));
                        } else {
                            // Create empty struct if no result

                            Struct emptyStruct;
                            response(socket, res.send(emptyStruct, "\n" + html + "\n"));
                        }
                        return;
                    }
                    
                    if(!result.has_value()){
                        std::cerr << "you must at least return something as your body or your page that you want." << std::endl;
                        return; 
                    }

                    response(socket, res.json(result.value()));

                } catch (const std::exception& e) {
                    Response res;
                    res.status = 500;
                    Struct data;
                    data.set("message", std::string(e.what()));
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
    try {
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
    catch(const std::runtime_error &e){
        std::cerr << "Error : " << e.what() << std::endl;
        throw;
    }
}
