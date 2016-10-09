#pragma once

#include <mstd/error_or.hpp>

#include "types.hpp"

namespace cloudabi {

template<typename T> using error_or  = mstd::error_or<T, error>;

}
