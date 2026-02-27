#pragma once
#include <iostream>
#include <string>
#include <asio.hpp>
#include <functional>
#include <unordered_map>

class Request;
class Response;

#include "handler.hpp"
#include "RouterType.hpp"

class Router{
    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, RouterType>>> routes;
    
    public:
        void Get(const std::string path, const Middleware &middleware,const Handler &handler);

        void Post(const std::string path, const Middleware &middleware,const Handler &handler );

        void Put(const std::string path, const Middleware &middleware,const Handler &handler);

        void Patch(const std::string path, const Middleware &middleware,const Handler &handler);

        void Delete(const std::string path, const Middleware &middleware,const Handler &handler);


        bool hasRoute(const std::string &method,const std::string path)const;

        Handler getHandler(const std::string &method, const std::string path);
        
        Middleware getMiddleware(const std::string &method, const std::string path);

        void printRoutes() const;
        
};