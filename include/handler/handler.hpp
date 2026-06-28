#pragma once
// #include "Router.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "Struct.hpp"
#include<functional>
class Router;

using Handler = std::function<std::optional<Struct>(const Request&, Response&)>;