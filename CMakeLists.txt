add_library(core STATIC 
    ./src/Server.cpp
    ./src/threads/ThreadPool.cpp
    ./src/structs/Struct.cpp
    ./src/Request.cpp
    ./src/Router.cpp
    ./src/Response.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/structs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/type
    ${CMAKE_CURRENT_SOURCE_DIR}/include/handler
    ${CMAKE_CURRENT_SOURCE_DIR}/include/serializer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/deserializer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/threads

)

target_compile_definitions(core PUBLIC ASIO_STANDALONE)
target_link_libraries(core PUBLIC pthread)


