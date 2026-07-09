# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Core library with SSL support
add_library(core STATIC 
    ./src/Server.cpp
    ./src/SSLContext.cpp
    ./src/SSLServer.cpp
    ./src/threads/ThreadPool.cpp
    ./src/structs/Struct.cpp
    ./src/Request.cpp
    ./src/Router.cpp
    ./src/Response.cpp
    ./src/Path.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/health
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/structs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/type
    ${CMAKE_CURRENT_SOURCE_DIR}/include/handler
    ${CMAKE_CURRENT_SOURCE_DIR}/include/serializer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/deserializer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/threads
    ${CMAKE_CURRENT_SOURCE_DIR}/include/middleware
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ssl
    ${OPENSSL_INCLUDE_DIR}
)

target_compile_definitions(core PUBLIC 
    ASIO_STANDALONE
    USE_OPENSSL
)

target_link_libraries(core PUBLIC 
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Optional: Add SSL examples and tests
option(BUILD_SSL_EXAMPLES "Build SSL examples" ON)

if(BUILD_SSL_EXAMPLES)
    add_executable(ssl_example 
        examples/ssl_main.cpp
    )
    
    target_link_libraries(ssl_example
        core
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    )
    
    target_include_directories(ssl_example PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${OPENSSL_INCLUDE_DIR}
    )
endif()

# Optional: Generate self-signed certificate for development
add_custom_target(generate_cert
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/certs
    COMMAND openssl genrsa -out ${CMAKE_BINARY_DIR}/certs/server.key 2048
    COMMAND openssl req -new -key ${CMAKE_BINARY_DIR}/certs/server.key -out ${CMAKE_BINARY_DIR}/certs/server.csr -subj "/CN=localhost"
    COMMAND openssl x509 -req -days 365 -in ${CMAKE_BINARY_DIR}/certs/server.csr -signkey ${CMAKE_BINARY_DIR}/certs/server.key -out ${CMAKE_BINARY_DIR}/certs/server.crt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/certs/server.csr
    COMMENT "Generating self-signed SSL certificates for development"
)

# Copy certificates to build directory
add_custom_command(TARGET core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/certs
        ${CMAKE_BINARY_DIR}/bin/certs
    DEPENDS generate_cert
)