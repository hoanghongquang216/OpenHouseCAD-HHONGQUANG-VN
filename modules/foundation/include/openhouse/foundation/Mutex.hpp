#pragma once
#include <mutex>
#include <shared_mutex>
namespace openhouse::foundation {
using std::lock_guard;
using std::mutex;
using std::recursive_mutex;
using std::scoped_lock;
using std::shared_mutex;
using std::shared_lock;
using std::unique_lock;
}
