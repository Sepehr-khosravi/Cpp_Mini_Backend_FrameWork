#pragma once

#include <asio.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <functional>

#include "Request.hpp"
#include "Router.hpp"
#include "handler.hpp"
#include "Value.hpp"
#include "Struct.hpp"
#include "ThreadPool.hpp"
#include "SSLContext.hpp"

namespace srv {

class Server {
protected:
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    
    size_t thread_count;
    ThreadPool thread_pool;
    
    // SSL support (protected so derived class can access)
    std::shared_ptr<SSLContext> ssl_context;
    bool use_https = false;
    
    // Virtual methods for derived classes to override
    virtual void response(asio::ip::tcp::socket& socket, const std::string& data);
    virtual void handleClient(asio::ip::tcp::socket socket);
    virtual void processRequest(asio::ip::tcp::socket& socket, const Request& req);
    
    // Helper to create response
    std::string createResponse(const Request& req, const std::string& body, 
                               int status = 200, 
                               const std::unordered_map<std::string, std::string>& headers = {});

public:
    Router router;

    // HTTP constructor
    explicit Server(int port = 8080,
                    size_t threads = std::thread::hardware_concurrency());

    // Constructor with SSL context (for derived classes)
    Server(int port,
           std::shared_ptr<SSLContext> ssl_context,
           size_t threads = std::thread::hardware_concurrency());

    virtual ~Server() = default;

    // Run HTTP server
    virtual void run(const std::string& text);
    
    // Stop the server
    virtual void stop();

    // Check if using HTTPS
    bool isHTTPS() const { return use_https; }
    
    // Get SSL context
    std::shared_ptr<SSLContext> getSSLContext() const { return ssl_context; }
};

} // namespace srv