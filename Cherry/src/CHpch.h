#pragma once
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <regex>

// Add GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cherry/Core/Log.h"

#ifdef CH_PLATFORM_WINDOWS
#include <Windows.h>
#endif