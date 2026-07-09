#pragma once

#include "SSLContext.hpp"
#include "SSLServer.hpp"
#include <memory>

namespace srv {

class SSLFactory {
public:
    // Create SSL server with certificate files
    static std::unique_ptr<SSLServer> createServer(
        int port,
        const std::string& cert_file,
        const std::string& key_file,
        size_t threads = std::thread::hardware_concurrency()
    ) {
        return std::make_unique<SSLServer>(port, cert_file, key_file, threads);
    }

    // Create SSL server with existing SSL context
    static std::unique_ptr<SSLServer> createServer(
        int port,
        std::shared_ptr<SSLContext> ssl_context,
        size_t threads = std::thread::hardware_concurrency()
    ) {
        return std::make_unique<SSLServer>(port, ssl_context, threads);
    }

    // Create SSL context
    static std::shared_ptr<SSLContext> createContext(
        const std::string& cert_file,
        const std::string& key_file
    ) {
        return std::make_shared<SSLContext>(cert_file, key_file);
    }

    // Generate self-signed certificate
    static void generateSelfSignedCert(
        const std::string& cert_file,
        const std::string& key_file,
        const std::string& common_name = "localhost"
    ) {
        SSLContext::generateSelfSigned(cert_file, key_file, common_name);
    }
};

} // namespace srv