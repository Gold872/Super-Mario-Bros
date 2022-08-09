#pragma once

#include <functional>

template <typename T>
using Supplier = std::function<T()>;

using BooleanSupplier = Supplier<bool>;
