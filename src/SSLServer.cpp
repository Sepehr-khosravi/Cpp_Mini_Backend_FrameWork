#include "SSLServer.hpp"
#include <stdexcept>

namespace srv {

// Constructor with certificate files
SSLServer::SSLServer(int port,
                     const std::string& cert_file,
                     const std::string& key_file,
                     size_t threads)
    : Server(port, std::make_shared<SSLContext>(cert_file, key_file), threads) {
    
    // Get native SSL context from SSLContext
    auto ssl_ctx = std::dynamic_pointer_cast<SSLContext>(ssl_context);
    if (!ssl_ctx) {
        throw std::runtime_error("Failed to create SSL context");
    }
    
    ssl_native_context = std::make_shared<asio::ssl::context>(ssl_ctx->get());
    use_https = true;
    
    std::cout << "SSLServer initialized with certificate: " << cert_file << std::endl;
}

// Constructor with existing SSL context
SSLServer::SSLServer(int port,
                     std::shared_ptr<SSLContext> ssl_context,
                     size_t threads)
    : Server(port, ssl_context, threads) {
    
    if (!ssl_context || !ssl_context->isInitialized()) {
        throw std::runtime_error("Invalid SSL context provided");
    }
    
    ssl_native_context = std::make_shared<asio::ssl::context>(ssl_context->get());
    use_https = true;
    
    std::cout << "SSLServer initialized with custom SSL context" << std::endl;
}

// Constructor with native SSL context
SSLServer::SSLServer(int port,
                     std::shared_ptr<asio::ssl::context> ssl_context,
                     size_t threads)
    : Server(port, nullptr, threads),
      ssl_native_context(ssl_context) {
    
    if (!ssl_native_context) {
        throw std::runtime_error("Invalid native SSL context provided");
    }
    
    // Create wrapper SSLContext
    ssl_context = std::make_shared<SSLContext>();
    use_https = true;
    
    std::cout << "SSLServer initialized with native SSL context" << std::endl;
}

void SSLServer::response(asio::ip::tcp::socket& socket, const std::string& data) {
    // This should not be called for SSL connections
    // But we keep it for compatibility
    asio::error_code ec;
    asio::write(socket, asio::buffer(data), ec);
    if (ec) {
        std::cerr << "Write error: " << ec.message() << std::endl;
    }
}

void SSLServer::responseSSL(SSLStream& ssl_stream, const std::string& data) {
    asio::error_code ec;
    asio::write(ssl_stream, asio::buffer(data), ec);
    if (ec) {
        std::cerr << "SSL write error: " << ec.message() << std::endl;
    }
}

bool SSLServer::performHandshake(SSLStream& ssl_stream) {
    asio::error_code ec;
    ssl_stream.handshake(asio::ssl::stream_base::server, ec);
    
    if (ec) {
        std::cerr << "SSL handshake failed: " << ec.message() << std::endl;
        return false;
    }
    
    std::cout << "SSL handshake successful" << std::endl;
    return true;
}

bool SSLServer::onSSLHandshake(SSLStream& ssl_stream) {
    // You can add custom handshake logic here
    // For example: certificate verification, client authentication, etc.
    return true;
}

SSLServer::SSLStream SSLServer::createSSLStream(asio::ip::tcp::socket socket) {
    return SSLStream(std::move(socket), *ssl_native_context);
}

void SSLServer::processSSLRequest(SSLStream& ssl_stream, const Request& req) {
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
            responseSSL(ssl_stream, response);
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
            responseSSL(ssl_stream, response);
            return;
        }

        Handler handler = router.getHandler(req.method, req.target);
        if (!handler) {
            res.status = 500;
            Struct data;
            data.set("message", std::string("Handler not found"));
            std::string response_body = res.json(data);
            std::string response = createResponse(req, response_body, res.status);
            responseSSL(ssl_stream, response);
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
            responseSSL(ssl_stream, response);
            return;
        }
        
        if(!result.has_value()){
            std::cerr << "you must at least return something as your body or your page that you want." << std::endl;
            return; 
        }

        std::string response_body = res.json(result.value());
        std::string response = createResponse(req, response_body, res.status, 
                                              res.headers.getAll());
        responseSSL(ssl_stream, response);

    } catch (const std::exception& e) {
        Response res;
        res.status = 500;
        Struct data;
        data.set("message", std::string(e.what()));
        std::string response_body = res.json(data);
        std::string response = createResponse(req, response_body, res.status);
        responseSSL(ssl_stream, response);
    }
}

void SSLServer::handleSSLClient(SSLStream ssl_stream) {
    try {
        // Perform SSL handshake
        if (!performHandshake(ssl_stream)) {
            return;
        }

        // Call custom handshake handler
        if (!onSSLHandshake(ssl_stream)) {
            std::cerr << "Handshake rejected by custom handler" << std::endl;
            return;
        }

        // Read request
        char buffer[8192];
        asio::error_code ec;
        size_t bytes = ssl_stream.read_some(asio::buffer(buffer), ec);

        if (ec) {
            std::cerr << "SSL read error: " << ec.message() << std::endl;
            return;
        }

        std::string raw(buffer, bytes);
        Request req;
        req.deserializing(raw);

        // Process request in thread pool
        thread_pool.enqueue(
            [this, ssl_stream = std::move(ssl_stream), req]() mutable {
                processSSLRequest(ssl_stream, req);
            }
        );

    } catch (const std::exception& e) {
        std::cerr << "SSL client handling error: " << e.what() << std::endl;
    }
}

void SSLServer::handleClient(asio::ip::tcp::socket socket) {
    // Override base class method to handle SSL
    if (use_https && ssl_native_context) {
        auto ssl_stream = createSSLStream(std::move(socket));
        handleSSLClient(std::move(ssl_stream));
    } else {
        // Fallback to HTTP if SSL not available
        Server::handleClient(std::move(socket));
    }
}

void SSLServer::processRequest(asio::ip::tcp::socket& socket, const Request& req) {
    // Override base class method - should not be called for SSL
    std::cerr << "Warning: processRequest called on SSL server without SSL stream" << std::endl;
    Server::processRequest(socket, req);
}

void SSLServer::run(const std::string& text) {
    runSSL(text);
}

void SSLServer::runSSL(const std::string& text) {
    if (!use_https || !ssl_native_context) {
        throw std::runtime_error("SSL not initialized properly");
    }

    try {
        std::cout << text << std::endl;
        std::cout << "HTTPS Server running with "
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
                std::cout << "New HTTPS connection from "
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

} // namespace srv