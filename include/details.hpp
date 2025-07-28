#pragma once

namespace details 
{
template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

constexpr double EPSILON = 1e-10;
}  // namespace details