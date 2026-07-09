#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <memory>
#include <string>
#include <iostream>
#include <functional>
#include <vector>

namespace srv {

class SSLContext {
public:
    enum class VerificationMode {
        NONE,
        PEER,
        FAIL_IF_NO_PEER_CERT,
        CLIENT_ONCE
    };

    // Default constructor - creates uninitialized context
    SSLContext();
    
    // Constructor with certificate and key files
    SSLContext(const std::string& cert_file, const std::string& key_file);
    
    // Constructor with certificate, key, and optional password callback
    SSLContext(const std::string& cert_file, 
               const std::string& key_file,
               std::function<std::string(std::size_t, asio::ssl::context::password_purpose)> password_callback);
    
    ~SSLContext() = default;

    // Load certificate and key files
    void loadCertificateChain(const std::string& cert_file);
    void loadPrivateKey(const std::string& key_file, 
                        std::function<std::string(std::size_t, asio::ssl::context::password_purpose)> password_callback = nullptr);
    void loadCertificateFile(const std::string& cert_file);
    void loadKeyFile(const std::string& key_file);
    
    // Load certificate and key from memory
    void loadCertificateChainFromMemory(const std::string& cert_data);
    void loadPrivateKeyFromMemory(const std::string& key_data);

    // Set cipher list
    void setCipherList(const std::string& ciphers);
    
    // Set verification mode
    void setVerifyMode(VerificationMode mode);
    
    // Set verification callback
    template<typename VerifyCallback>
    void setVerifyCallback(VerifyCallback callback) {
        if (context) {
            context->set_verify_callback(callback);
        }
    }

    // Set options
    void setOptions(asio::ssl::context::options options);
    void setDefaultOptions();

    // Get the underlying SSL context
    asio::ssl::context& get() { 
        if (!context) {
            throw std::runtime_error("SSL context not initialized");
        }
        return *context; 
    }
    
    const asio::ssl::context& get() const { 
        if (!context) {
            throw std::runtime_error("SSL context not initialized");
        }
        return *context; 
    }

    // Get native handle (for advanced OpenSSL operations)
    SSL_CTX* native_handle() {
        if (!context) {
            throw std::runtime_error("SSL context not initialized");
        }
        return context->native_handle();
    }

    // Check if SSL is initialized
    bool isInitialized() const { return initialized && context != nullptr; }

    // Get certificate and key file paths
    const std::string& getCertFile() const { return cert_file_; }
    const std::string& getKeyFile() const { return key_file_; }

    // Generate self-signed certificate for development
    static void generateSelfSigned(const std::string& cert_file, 
                                   const std::string& key_file,
                                   const std::string& common_name = "localhost",
                                   int days_valid = 365);
    
    // Generate self-signed certificate with additional SANs
    static void generateSelfSignedWithSAN(const std::string& cert_file,
                                         const std::string& key_file,
                                         const std::vector<std::string>& san_list,
                                         const std::string& common_name = "localhost",
                                         int days_valid = 365);

    // Set session cache settings (using OpenSSL native functions)
    void setSessionCacheSize(long size);
    void setSessionCacheMode(int mode);  // Using OpenSSL constants
    void setSessionIdContext(const std::string& context_str);

    // Set temporary DH parameters (for perfect forward secrecy)
    void setTmpDH(const std::string& dh_file);
    void setTmpDHFromMemory(const std::string& dh_data);

    // Set ECDH curve (for ECDHE ciphers)
    void setECDHCurve(const std::string& curve_name = "prime256v1");

    // Enable/disable client certificate verification
    void setClientVerification(bool require, bool fail_if_no_cert = false);
    
    // Set session timeout
    void setSessionTimeout(long seconds);

private:
    std::unique_ptr<asio::ssl::context> context;
    bool initialized = false;
    std::string cert_file_;
    std::string key_file_;

    void initContext();
    void loadDefaultCA();
    void setupDefaultOptions();
};

} // namespace srv