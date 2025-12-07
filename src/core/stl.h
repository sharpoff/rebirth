#pragma once

#include "EASTL/unique_ptr.h"
#include "EASTL/shared_ptr.h"
#include "EASTL/list.h"
#include "EASTL/vector.h"
#include "EASTL/array.h"
#include "EASTL/unordered_map.h"
#include "EASTL/map.h"
#include "EASTL/string.h"
#include "EASTL/set.h"
#include "EASTL/queue.h"

template <typename T>
using UniquePtr = eastl::unique_ptr<T>;

template <typename T>
using SharedPtr = eastl::shared_ptr<T>;

template <typename T, size_t N>
using Array = eastl::array<T, N>;

template <typename T>
using Vector = eastl::vector<T>;

template <typename T>
using List = eastl::list<T>;

template <typename Key, typename T>
using UnorderedMap = eastl::unordered_map<Key, T>;

template <typename Key, typename T>
using OrderedMap = eastl::map<Key, T>;

using String = eastl::string;

template <typename Key>
using Set = eastl::set<Key>;

template <typename T>
using Queue = eastl::queue<T>;