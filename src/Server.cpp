#include "Server.hpp"
#include <stdexcept>

using namespace srv;

// HTTP Constructor
Server::Server(int port, size_t threads)
    : acceptor(io_context,
               asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      thread_count(threads == 0 ? std::thread::hardware_concurrency() : threads),
      thread_pool(thread_count),
      use_https(false) {}

// Constructor with SSL context (for derived classes)
Server::Server(int port,
               std::shared_ptr<SSLContext> ssl_context,
               size_t threads)
    : acceptor(io_context,
               asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      thread_count(threads == 0 ? std::thread::hardware_concurrency() : threads),
      thread_pool(thread_count),
      ssl_context(ssl_context),
      use_https(true) {}

void Server::response(asio::ip::tcp::socket& socket,
                      const std::string& data) {
    asio::error_code ec;
    asio::write(socket, asio::buffer(data), ec);
    if (ec) {
        std::cerr << "Write error: " << ec.message() << std::endl;
    }
}

std::string Server::createResponse(const Request& req, const std::string& body, 
                                   int status, 
                                   const std::unordered_map<std::string, std::string>& headers) {
    std::string response = "HTTP/1.1 " + std::to_string(status) + " " + 
                           (status == 200 ? "OK" : status == 404 ? "Not Found" : "Internal Server Error") + "\r\n";
    
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Connection: keep-alive\r\n";
    response += "Server: C++ Server\r\n";
    
    for (const auto& [key, value] : headers) {
        response += key + ": " + value + "\r\n";
    }
    
    response += "\r\n";
    response += body;
    
    return response;
}

void Server::processRequest(asio::ip::tcp::socket& socket, const Request& req) {
    try {
        if (!router.hasRoute(req.method, req.target)) {
            Response res;
            res.status = 404;
            Struct data;
            res.render("/external/core/src/notfound.html");
            std::string notFoundHtml = res.load();
            data.set("message", std::string("Not Found"));
            std::string response_body = res.send(data, notFoundHtml);
            std::string response = createResponse(req, response_body, 404);
            response(socket, response);
            return;
        }

        Response res;
        Middleware middleware = router.getMiddleware(req.method, req.target);

        res.status = 200;
        bool resultMiddleware = middleware(req, res);

        if(!resultMiddleware){
            if(res.status == 200) res.status = 400;
            Struct data;
            data.set("message", std::string("Request rejected by middleware"));
            std::string response_body = res.json(data);
            std::string response = createResponse(req, response_body, res.status);
            response(socket, response);
            return;
        }

        Handler handler = router.getHandler(req.method, req.target);
        if (!handler) {
            res.status = 500;
            Struct data;
            data.set("message", std::string("Handler not found"));
            std::string response_body = res.json(data);
            std::string response = createResponse(req, response_body, res.status);
            response(socket, response);
            return;
        }

        res.status = 200;
        std::optional<Struct> result = handler(req, res);

        if(res.hasPage){
            res.headers.set("Content-Type", "text/html");
            res.headers.set("charset", "utf-8");
            if(res.checkPageError()){
                res.hasPage = false;
            }
            std::string html = res.load();
            std::string response_body;
            if (result.has_value()) {
                response_body = res.send(result.value(), "\n" + html + "\n");
            } else {
                Struct emptyStruct;
                response_body = res.send(emptyStruct, "\n" + html + "\n");
            }
            std::string response = createResponse(req, response_body, res.status, 
                                                  res.headers.getAll());
            response(socket, response);
            return;
        }
        
        if(!result.has_value()){
            std::cerr << "you must at least return something as your body or your page that you want." << std::endl;
            return; 
        }

        std::string response_body = res.json(result.value());
        std::string response = createResponse(req, response_body, res.status, 
                                              res.headers.getAll());
        response(socket, response);

    } catch (const std::exception& e) {
        Response res;
        res.status = 500;
        Struct data;
        data.set("message", std::string(e.what()));
        std::string response_body = res.json(data);
        std::string response = createResponse(req, response_body, res.status);
        response(socket, response);
    }
}

void Server::handleClient(asio::ip::tcp::socket socket) {
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
                processRequest(socket, req);
            }
        );

    } catch (const std::exception& e) {
        std::cerr << "Client handling error: " << e.what() << std::endl;
    }
}

void Server::run(const std::string& text) {
    try {
        std::cout << text << std::endl;
        std::cout << "HTTP Server running with "
                  << thread_count
                  << " worker threads on port "
                  << acceptor.local_endpoint().port() << std::endl;
   
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
                std::cout << "New HTTP connection from "
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

void Server::stop() {
    io_context.stop();
}