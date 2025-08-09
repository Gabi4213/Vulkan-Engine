// entity.hpp
#pragma once
#include <cstdint>
#include <limits>

using Entity = uint32_t;
const Entity INVALID_ENTITY = std::numeric_limits<Entity>::max();
