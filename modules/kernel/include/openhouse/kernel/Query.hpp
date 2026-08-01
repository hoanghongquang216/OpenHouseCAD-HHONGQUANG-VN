#pragma once

// Public query entry point for kernel consumers.
//
// Higher layers should depend on this facade instead of
// individual query implementation headers.
//
// Available operations:
//   Query::ForEach(store, callback)
//   Query::FindAll<T>(store)
//   Query::FindIf<T>(store, predicate)
//
// Internal query implementation may evolve without changing
// this public API boundary.

#include <openhouse/kernel/ObjectStoreFilter.hpp>
#include <openhouse/kernel/ObjectStorePredicate.hpp>
#include <openhouse/kernel/ObjectStoreQuery.hpp>

namespace openhouse::kernel::Query
{

using openhouse::kernel::ForEach;
using openhouse::kernel::FindAll;
using openhouse::kernel::FindIf;

}
