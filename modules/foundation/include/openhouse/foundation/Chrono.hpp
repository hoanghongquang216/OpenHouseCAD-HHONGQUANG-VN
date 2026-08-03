#pragma once

#include <chrono>

namespace openhouse::foundation {

// Curated per ADR-0001 (openhouse::foundation wraps std:: facilities via
// explicit named `using` declarations, not blanket namespace imports).
// This header previously used `using namespace std::chrono;`, which
// pulled every name in std::chrono (including generic-sounding ones like
// `days`, `weekday`, `year`, `month`) into openhouse::foundation
// unfiltered -- a real risk of future name collisions, and inconsistent
// with every other header in this module. Fixed to curate explicitly,
// matching Filesystem.hpp/Ranges.hpp's namespace-alias pattern and the
// rest of this module's individual `using std::X;` pattern.

using std::chrono::duration;
using std::chrono::time_point;

using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;

using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;

using std::chrono::duration_cast;
using std::chrono::time_point_cast;

// The chrono literal suffixes (e.g. `5s`, `100ms`) are intentionally
// exposed via a namespace-level `using namespace`, not individually --
// unlike named types, these are operator"" overloads meant to be used
// unqualified at the point of a literal; curating them one by one would
// defeat their purpose without meaningfully reducing collision risk
// (they're all suffix operators, not generic names).
using namespace std::chrono_literals;

}
