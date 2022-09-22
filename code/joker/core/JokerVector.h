#pragma once

#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>

namespace joker {
template <typename T, typename Allocator = EASTLAllocatorType>
using vector = eastl::vector<T, Allocator>;

template <typename T, size_t nodeCount, bool bEnableOverflow = true, typename OverflowAllocator = typename eastl::type_select<bEnableOverflow, EASTLAllocatorType, EASTLDummyAllocatorType>::type>
using fixed_vector = eastl::fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>;
}