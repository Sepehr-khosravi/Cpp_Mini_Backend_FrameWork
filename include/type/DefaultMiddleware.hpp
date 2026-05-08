#pragma once
#include "Response.hpp"
#include "Request.hpp"
#include "middleware.hpp"

Middleware DefaultMiddleware = [](const Request &req, Response &res)->bool{
    return true;
};



