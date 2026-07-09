#pragma once

#include "Server.hpp"
#include <asio/ssl.hpp>

namespace srv {

class SSLServer : public Server {
private:
    using SSLStream = asio::ssl::stream<asio::ip::tcp::socket>;
    
    // Override base class methods
    void response(asio::ip::tcp::socket& socket, const std::string& data) override;
    void handleClient(asio::ip::tcp::socket socket) override;
    void processRequest(asio::ip::tcp::socket& socket, const Request& req) override;
    
    // SSL specific methods
    void handleSSLClient(SSLStream ssl_stream);
    void processSSLRequest(SSLStream& ssl_stream, const Request& req);
    void responseSSL(SSLStream& ssl_stream, const std::string& data);

    // SSL handshake callback
    bool onSSLHandshake(SSLStream& ssl_stream);
    
    // SSL stream wrapper for requests
    std::shared_ptr<asio::ssl::context> ssl_native_context;

public:
    // Constructor with certificate files
    SSLServer(int port,
              const std::string& cert_file,
              const std::string& key_file,
              size_t threads = std::thread::hardware_concurrency());
    
    // Constructor with existing SSL context
    SSLServer(int port,
              std::shared_ptr<SSLContext> ssl_context,
              size_t threads = std::thread::hardware_concurrency());
    
    // Constructor with native SSL context (for advanced use)
    SSLServer(int port,
              std::shared_ptr<asio::ssl::context> ssl_context,
              size_t threads = std::thread::hardware_concurrency());

    ~SSLServer() override = default;

    // Override run method for HTTPS
    void run(const std::string& text) override;
    
    // SSL specific run method (for clarity)
    void runSSL(const std::string& text);
    
    // Get SSL context
    std::shared_ptr<asio::ssl::context> getNativeSSLContext() const { 
        return ssl_native_context; 
    }

private:
    // Helper to create SSL stream
    SSLStream createSSLStream(asio::ip::tcp::socket socket);
    
    // SSL handshake helper
    bool performHandshake(SSLStream& ssl_stream);
};

} // namespace srv