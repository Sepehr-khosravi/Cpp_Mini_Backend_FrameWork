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



void Router::Get (const std::string path, const Middleware &middleware,const Handler &handler){
    routes["GET"][path]["MIDDLEWARE"] = middleware;
    routes["GET"][path]["HANDLER"] = handler;
};

void Router::Post(const std::string path, const Middleware &middleware,const Handler &handler){
    routes["POST"][path]["MIDDLEWARE"] = middleware;
    routes["POST"][path]["HANDLER"] = handler;
};

void Router::Put(const std::string path, const Middleware &middleware,const Handler &handler){
    routes["PUT"][path]["MIDDLEWARE"] = middleware;
    routes["PUT"][path]["HANDLER"] = handler;
};


void Router::Patch(const std::string path, const Middleware &middleware,const Handler &handler){
    routes["PATCH"][path]["MIDDLEWARE"] = middleware;
    routes["PATCH"][path]["HANDLER"] = handler;
};

void Router::Delete(const std::string path, const Middleware &middleware,const Handler &handler){
    routes["DELETE"][path]["MIDDLEWARE"] = middleware;
    routes["DELETE"][path]["HANDLER"] = handler;
};

bool Router::hasRoute(const std::string &method, const std::string path) const{
    if(routes.count(method) == 0) return false;
    return routes.at(method).count(path) != 0; 
};

Handler Router::getHandler(const std::string &method, const std::string path){
    return std::get<Handler>(routes[method][path]["HANDLER"]);
};


void Router::printRoutes() const {
    for(auto &[key, value] : this->routes){
        std::cout << "Routes in " << key << ": " << std::endl;
        for(auto &[keyTwo, valueTwo] : value){
            std::cout << keyTwo << std::endl;
        };
    }
};

Middleware Router::getMiddleware(const std::string &method, const std::string path){
    return std::get<Middleware>(routes[method][path]["MIDDLEWARE"]);
};
