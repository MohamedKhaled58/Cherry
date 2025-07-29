workspace "Cherry"
    architecture "x64"
    startproject "MyShell"
    
    configurations {
        "Debug",
        "Release",
        "Dist"
    }
    
    -- Global flags and defines
    flags {
        "MultiProcessorCompile"
    }
    
    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }
    
    -- Output directory pattern
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    -- Include directories relative to workspace root
    IncludeDir = {}
    IncludeDir["GLFW"] = "Cherry/vendor/GLFW/include"
    IncludeDir["Glad"] = "Cherry/vendor/Glad/include"
    IncludeDir["ImGui"] = "Cherry/vendor/imgui"
    IncludeDir["glm"] = "Cherry/vendor/glm"
    IncludeDir["stb_image"] = "Cherry/vendor/stb_image"
    IncludeDir["spdlog"] = "Cherry/vendor/spdlog/include"

-- ===== DEPENDENCIES =====
group "Dependencies"
    include "Cherry/vendor/GLFW"
    include "Cherry/vendor/Glad"
    include "Cherry/vendor/imgui"
group ""

-- ===== CHERRY ENGINE =====
project "Cherry"
    location "Cherry"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    -- Precompiled headers
    pchheader "CHpch.h"
    pchsource "Cherry/src/CHpch.cpp"
    
    -- Source files
    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
    }
    
    -- Include directories
    includedirs {
        "%{prj.name}/src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}"
    }
    
    -- Link libraries
    links {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }
    
    -- Windows-specific settings
    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }
        
        defines {
            "CH_PLATFORM_WINDOWS",
            "CH_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }
    
    -- Configuration-specific settings
    filter "configurations:Debug"
        defines { "CH_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "CH_RELEASE" }
        runtime "Release"
        optimize "On"
    
    filter "configurations:Dist"
        defines { "CH_DIST" }
        runtime "Release"
        optimize "On"

-- ===== MYSHELL APPLICATION =====
project "MyShell"
    location "MyShell"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    -- Source files
    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }
    
    -- Include directories
    includedirs {
        "Cherry/vendor/spdlog/include",
		"Cherry/src",
		"Cherry/vendor",
		"%{IncludeDir.glm}"
    }
    
    -- Link libraries
    links {
        "Cherry"
    }
    
    -- Dependencies
    dependson {
        "Cherry"
    }
    
    -- Windows-specific settings
    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }
        
        defines {
            "CH_PLATFORM_WINDOWS"
        }
    
    -- Configuration-specific settings
    filter "configurations:Debug"
        defines { "CH_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "CH_RELEASE" }
        runtime "Release"
        optimize "On"
    
    filter "configurations:Dist"
        defines { "CH_DIST" }
        runtime "Release"
        optimize "On"