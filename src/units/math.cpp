#include "math.hpp"

#include <random>

namespace xen {
std::mt19937 const random_generator(std::random_device{}());

template <typename T>
T Math::random(T min, T max)
{
    std::uniform_real_distribution<T> dist(min, max);
    return dist(random_generator);
}

template <typename T>
T Math::random_normal(T standart_deviation, T mean)
{
    std::normal_distribution<T> dist(mean, standart_deviation);
    return dist(random_generator);
}

template <typename T>
T Math::random_log(T min, T max)
{
    auto const logLower = std::log(min);
    auto const logUpper = std::log(max);
    auto const raw = random(min, max);

    auto result = std::exp((raw * (logUpper - logLower)) + logLower);

    if (result < min) {
        result = min;
    }
    else if (result > max) {
        result = max;
    }

    return result;
}
}