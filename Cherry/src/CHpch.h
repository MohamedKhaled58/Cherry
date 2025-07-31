#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>
// Filesystem includes
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <filesystem>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stack>
#include <deque>
#include <type_traits>
#include <atomic>
#include <chrono>
#include <any>
#include <optional>
#include <variant>
#include <cstdint>
#include <cstring>
#include <cmath>

// Add GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Cherry/Core/Log.h"
#include "Cherry/Core/Core.h"

#ifdef CH_PLATFORM_WINDOWS
#include <Windows.h>
// Undefine Windows macros that conflict with our code
#ifdef PlaySoundW
#undef PlaySoundW
#endif
#ifdef PlaySoundA
#undef PlaySoundA
#endif
#ifdef PlaySound
#undef PlaySound
#endif
// Additional Windows macro undefs for safety
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef CreateWindowA
#undef CreateWindowA
#endif
#ifdef CreateWindowW
#undef CreateWindowW
#endif
#ifdef NOMINMAX
#define NOMINMAX
#endif
#endif

// Third-party includes
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
