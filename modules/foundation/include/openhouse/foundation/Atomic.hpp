#pragma once

#include <atomic>

namespace openhouse::foundation {

using std::atomic;
using std::atomic_flag;
using std::memory_order;
using std::memory_order_relaxed;
using std::memory_order_acquire;
using std::memory_order_release;
using std::memory_order_acq_rel;
using std::memory_order_seq_cst;

}
