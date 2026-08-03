#pragma once
#include <thread>
namespace openhouse::foundation {
using std::jthread;
using std::thread;
namespace this_thread = std::this_thread;
using std::stop_source;
using std::stop_token;
}
