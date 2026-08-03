#pragma once

#include <cstdint>

namespace openhouse::document {

// A stable identifier for an Entity within a Document, independent of
// its position in Document::Entities() (which can change once Document
// supports deleting individual entities -- see Document.hpp's own
// TODO(Spiral5) on this). IDs are assigned monotonically increasing by
// Document::Add(), starting at 1; 0 is reserved as "no entity"
// (kInvalidEntityId), matching this project's existing convention of a
// dedicated invalid sentinel (see foundation::kInvalid<T>) rather than
// overloading a valid-looking value like -1 or the max representable ID.
using EntityId = std::uint64_t;

inline constexpr EntityId kInvalidEntityId = 0;

}
