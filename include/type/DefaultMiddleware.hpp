#pragma once
#include "Response.hpp"
#include "Request.hpp"

extern bool DefaultMiddleware(const Request &req, Response &res){
  return true;
};
