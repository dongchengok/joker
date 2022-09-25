#pragma once
#include "EASTL/hash_map.h"
#include "EASTL/fixed_hash_map.h"

namespace joker
{

template <typename Key, typename T, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>, typename Allocator = EASTLAllocatorType,
          bool bCacheHashCode = false>
using hash_map = eastl::hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>;

template <typename Key, typename T, size_t nodeCount, size_t bucketCount = nodeCount + 1, bool bEnableOverflow = true, typename Hash = eastl::hash<Key>,
          typename Predicate = eastl::equal_to<Key>, bool bCacheHashCode = false, typename OverflowAllocator = EASTLAllocatorType>
using fixed_hash_map = eastl::fixed_hash_map<Key, T, nodeCount, bucketCount, bEnableOverflow, Hash, Predicate, bCacheHashCode, OverflowAllocator>;

}