#pragma once
#include "Request.hpp"
#include "Response.hpp"
#include <functional>

//a simple type structured to create a guard like to protect all routes
using Middleware = std::function<bool(const Request&, Response&)>;
