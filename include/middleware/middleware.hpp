#pragma once
#include "Request.hpp"
#include "Response.hpp"
#include <functional>

using Middleware = std::function<bool(const Request&, Response&)>;