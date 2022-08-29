#pragma once

#include <EASTL/vector.h>

namespace joker {
template <typename T, typename Allocator = EASTLAllocatorType>
using vector = eastl::vector<T, Allocator>;
}