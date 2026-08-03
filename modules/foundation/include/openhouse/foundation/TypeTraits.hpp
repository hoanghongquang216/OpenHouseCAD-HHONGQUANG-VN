#pragma once

#include <type_traits>

namespace openhouse::foundation {

using std::conditional_t;
using std::decay_t;
using std::enable_if_t;
using std::is_base_of_v;
using std::is_enum_v;
using std::is_integral_v;
using std::is_floating_point_v;
using std::is_same_v;
using std::remove_cvref_t;
using std::underlying_type_t;

}
