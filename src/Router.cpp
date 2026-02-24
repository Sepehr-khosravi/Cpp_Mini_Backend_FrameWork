#include "Router.hpp"
#include <iostream>
using namespace std;
#include <string>
#include <asio.hpp>
// #include "Response.hpp"
// #include "Request.hpp"
#include <functional>
#include <unordered_map>
// #include "handler.hpp"



void Router::Get (const std::string path,const Handler &handler){
    routes["GET"][path] = handler;
};

void Router::Post(const std::string path,const Handler &handler){
    routes["POST"][path] = handler;
};

void Router::Put(const std::string path,const Handler &handler){
    routes["PUT"][path] = handler;
};


void Router::Patch(const std::string path,const Handler &handler){
    routes["PATCH"][path] = handler;
};

void Router::Delete(const std::string path,const Handler &handler){
    routes["DELETE"][path] = handler;
};

bool Router::hasRoute(const std::string &method, const std::string path) const{
    if(routes.count(method) == 0) return false;
    return routes.at(method).count(path) != 0; 
};

Handler Router::getHandler(const std::string &method, const std::string path){
    return routes[method][path];
}


void Router::printRoutes() const {
    for(auto &[key, value] : this->routes){
        std::cout << "Routes in " << key << ": " << std::endl;
        for(auto &[keyTwo, valueTwo] : value){
            std::cout << keyTwo << std::endl;
        };
    }
}