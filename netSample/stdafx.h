#pragma once

#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/pool/pool.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>

#include <memory>
#include <mutex>
#include <string>
#include <string.h>
#include <vector>


#ifdef _DEBUG
#pragma comment(lib, "seedArmNetD.lib")
#else
#pragma comment(lib, "seedArmNet.lib")
#endif // DEBUG