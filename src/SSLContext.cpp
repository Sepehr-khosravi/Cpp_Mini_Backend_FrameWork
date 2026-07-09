#include "SSLContext.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <cstdlib>
#include <memory>
#include <iostream>

namespace srv {

// Default constructor
SSLContext::SSLContext() 
    : context(nullptr),
      initialized(false) {
    initContext();
}

// Constructor with certificate and key files
SSLContext::SSLContext(const std::string& cert_file, const std::string& key_file)
    : context(nullptr),
      initialized(false),
      cert_file_(cert_file),
      key_file_(key_file) {
    initContext();
    loadCertificateChain(cert_file);
    loadPrivateKey(key_file);
}

// Constructor with certificate, key, and password callback
SSLContext::SSLContext(const std::string& cert_file, 
                       const std::string& key_file,
                       std::function<std::string(std::size_t, asio::ssl::context::password_purpose)> password_callback)
    : context(nullptr),
      initialized(false),
      cert_file_(cert_file),
      key_file_(key_file) {
    initContext();
    loadCertificateChain(cert_file);
    loadPrivateKey(key_file, password_callback);
}

void SSLContext::initContext() {
    try {
        // Create context with TLS 1.2 support
        context = std::make_unique<asio::ssl::context>(asio::ssl::context::tlsv12_server);
        
        // Set default options
        setupDefaultOptions();
        
        // Set secure ciphers by default
        setCipherList("ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:ECDHE+AES:!aNULL:!MD5:!DSS:!RC4");
        
        // Set default ECDH curve
        setECDHCurve("prime256v1");
        
        initialized = true;
        
        std::cout << "SSL Context initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to initialize SSL context: " + std::string(e.what()));
    }
}

void SSLContext::setupDefaultOptions() {
    if (!context) return;
    
    context->set_options(
        asio::ssl::context::default_workarounds |
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 |
        asio::ssl::context::single_dh_use |
        asio::ssl::context::no_tlsv1 |
        asio::ssl::context::no_tlsv1_1
    );
}

void SSLContext::loadCertificateChain(const std::string& cert_file) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_certificate_chain_file(cert_file);
        std::cout << "Loaded certificate chain from: " << cert_file << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load certificate chain from " + cert_file + ": " + std::string(e.what()));
    }
}

void SSLContext::loadPrivateKey(const std::string& key_file, 
                                std::function<std::string(std::size_t, asio::ssl::context::password_purpose)> password_callback) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        if (password_callback) {
            context->set_password_callback(password_callback);
        }
        context->use_private_key_file(key_file, asio::ssl::context::pem);
        std::cout << "Loaded private key from: " << key_file << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load private key from " + key_file + ": " + std::string(e.what()));
    }
}

void SSLContext::loadCertificateFile(const std::string& cert_file) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_certificate_file(cert_file, asio::ssl::context::pem);
        std::cout << "Loaded certificate from: " << cert_file << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load certificate from " + cert_file + ": " + std::string(e.what()));
    }
}

void SSLContext::loadKeyFile(const std::string& key_file) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_private_key_file(key_file, asio::ssl::context::pem);
        std::cout << "Loaded key from: " << key_file << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load key from " + key_file + ": " + std::string(e.what()));
    }
}

void SSLContext::loadCertificateChainFromMemory(const std::string& cert_data) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_certificate_chain(asio::buffer(cert_data));
        std::cout << "Loaded certificate chain from memory" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load certificate chain from memory: " + std::string(e.what()));
    }
}

void SSLContext::loadPrivateKeyFromMemory(const std::string& key_data) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_private_key(asio::buffer(key_data), asio::ssl::context::pem);
        std::cout << "Loaded private key from memory" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load private key from memory: " + std::string(e.what()));
    }
}

void SSLContext::setCipherList(const std::string& ciphers) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    if (!SSL_CTX_set_cipher_list(context->native_handle(), ciphers.c_str())) {
        throw std::runtime_error("Failed to set cipher list: " + ciphers);
    }
    
    std::cout << "Cipher list set to: " << ciphers << std::endl;
}

void SSLContext::setVerifyMode(VerificationMode mode) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    asio::ssl::verify_mode verify_mode = asio::ssl::verify_none;
    
    switch (mode) {
        case VerificationMode::NONE:
            verify_mode = asio::ssl::verify_none;
            break;
        case VerificationMode::PEER:
            verify_mode = asio::ssl::verify_peer;
            break;
        case VerificationMode::FAIL_IF_NO_PEER_CERT:
            verify_mode = asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert;
            break;
        case VerificationMode::CLIENT_ONCE:
            verify_mode = asio::ssl::verify_peer | asio::ssl::verify_client_once;
            break;
    }
    
    context->set_verify_mode(verify_mode);
    std::cout << "SSL verification mode set to: " << static_cast<int>(mode) << std::endl;
}

void SSLContext::setOptions(asio::ssl::context::options options) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    context->set_options(options);
}

void SSLContext::setDefaultOptions() {
    setupDefaultOptions();
}

void SSLContext::setSessionCacheSize(long size) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    SSL_CTX_sess_set_cache_size(context->native_handle(), size);
    std::cout << "Session cache size set to: " << size << std::endl;
}

void SSLContext::setSessionCacheMode(int mode) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    SSL_CTX_set_session_cache_mode(context->native_handle(), mode);
    std::cout << "Session cache mode set to: " << mode << std::endl;
}

void SSLContext::setSessionIdContext(const std::string& context_str) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    SSL_CTX_set_session_id_context(context->native_handle(), 
                                   reinterpret_cast<const unsigned char*>(context_str.c_str()),
                                   context_str.length());
}

void SSLContext::setTmpDH(const std::string& dh_file) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_tmp_dh_file(dh_file);
        std::cout << "Loaded DH parameters from: " << dh_file << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load DH parameters: " + std::string(e.what()));
    }
}

void SSLContext::setTmpDHFromMemory(const std::string& dh_data) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    try {
        context->use_tmp_dh(asio::buffer(dh_data));
        std::cout << "Loaded DH parameters from memory" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load DH parameters from memory: " + std::string(e.what()));
    }
}

void SSLContext::setECDHCurve(const std::string& curve_name) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    auto handle = context->native_handle();
    if (handle) {
        int nid = OBJ_sn2nid(curve_name.c_str());
        if (nid == NID_undef) {
            throw std::runtime_error("Unknown curve name: " + curve_name);
        }
        
        #if OPENSSL_VERSION_NUMBER >= 0x10002000L
            if (SSL_CTX_set_ecdh_auto(handle, 1) != 1) {
                std::cerr << "Warning: Failed to set ECDH auto" << std::endl;
            }
        #else
            EC_KEY* ecdh = EC_KEY_new_by_curve_name(nid);
            if (ecdh) {
                SSL_CTX_set_tmp_ecdh(handle, ecdh);
                EC_KEY_free(ecdh);
            } else {
                throw std::runtime_error("Failed to create EC key for curve: " + curve_name);
            }
        #endif
        
        std::cout << "ECDH curve set to: " << curve_name << std::endl;
    }
}

void SSLContext::setClientVerification(bool require, bool fail_if_no_cert) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    int mode = SSL_VERIFY_PEER;
    if (require) {
        mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    }
    if (fail_if_no_cert) {
        mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    }
    
    SSL_CTX_set_verify(context->native_handle(), mode, nullptr);
    std::cout << "Client verification set: require=" << require 
              << ", fail_if_no_cert=" << fail_if_no_cert << std::endl;
}

void SSLContext::setSessionTimeout(long seconds) {
    if (!context) {
        throw std::runtime_error("SSL context not initialized");
    }
    
    SSL_CTX_set_timeout(context->native_handle(), seconds);
    std::cout << "Session timeout set to: " << seconds << " seconds" << std::endl;
}

void SSLContext::generateSelfSigned(const std::string& cert_file, 
                                    const std::string& key_file,
                                    const std::string& common_name,
                                    int days_valid) {
    std::string command = "openssl req -x509 -newkey rsa:2048 -nodes "
                          "-keyout " + key_file + " "
                          "-out " + cert_file + " "
                          "-days " + std::to_string(days_valid) + " "
                          "-subj \"/CN=" + common_name + "\" 2>/dev/null";
    
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to generate self-signed certificate. Make sure openssl is installed.");
    }
    
    std::cout << "Self-signed certificate generated:" << std::endl;
    std::cout << "  Certificate: " << cert_file << std::endl;
    std::cout << "  Private Key: " << key_file << std::endl;
}

void SSLContext::generateSelfSignedWithSAN(const std::string& cert_file,
                                          const std::string& key_file,
                                          const std::vector<std::string>& san_list,
                                          const std::string& common_name,
                                          int days_valid) {
    std::string key_command = "openssl genrsa -out " + key_file + " 2048 2>/dev/null";
    if (std::system(key_command.c_str()) != 0) {
        throw std::runtime_error("Failed to generate private key");
    }
    
    std::string config_content = "[req]\n";
    config_content += "distinguished_name = req_distinguished_name\n";
    config_content += "req_extensions = v3_req\n";
    config_content += "prompt = no\n\n";
    config_content += "[req_distinguished_name]\n";
    config_content += "CN = " + common_name + "\n\n";
    config_content += "[v3_req]\n";
    config_content += "keyUsage = keyEncipherment, dataEncipherment\n";
    config_content += "extendedKeyUsage = serverAuth\n";
    config_content += "subjectAltName = @alt_names\n\n";
    config_content += "[alt_names]\n";
    
    for (size_t i = 0; i < san_list.size(); ++i) {
        config_content += "DNS." + std::to_string(i + 1) + " = " + san_list[i] + "\n";
    }
    
    std::string config_file = "/tmp/openssl_san_config.$$";
    std::ofstream config(config_file);
    if (!config) {
        throw std::runtime_error("Failed to create temporary config file");
    }
    config << config_content;
    config.close();
    
    std::string cert_command = "openssl req -new -x509 -days " + std::to_string(days_valid) +
                               " -key " + key_file + 
                               " -out " + cert_file +
                               " -config " + config_file +
                               " -extensions v3_req 2>/dev/null";
    
    if (std::system(cert_command.c_str()) != 0) {
        std::system(("rm -f " + config_file).c_str());
        throw std::runtime_error("Failed to generate certificate with SAN");
    }
    
    std::system(("rm -f " + config_file).c_str());
    
    std::cout << "Self-signed certificate with SAN generated:" << std::endl;
    std::cout << "  Certificate: " << cert_file << std::endl;
    std::cout << "  Private Key: " << key_file << std::endl;
    std::cout << "  SANs: ";
    for (const auto& san : san_list) {
        std::cout << san << " ";
    }
    std::cout << std::endl;
}

} // namespace srv