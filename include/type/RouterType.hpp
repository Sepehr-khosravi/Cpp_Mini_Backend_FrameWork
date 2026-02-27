#pragma once
#include "variant"
#include "middleware.hpp"
#include "handler.hpp"


using RouterType = std::variant<
    Middleware,
    Handler
>;