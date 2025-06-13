workspace "Cherry"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include directories relative to root folder (solution directory)Add commentMore actions
IncludeDir = {}
IncludeDir["GLFW"] = "Cherry/Vendor/GLFW/include"

include "Cherry/Vendor/GLFW"

project "Cherry"
	location "Cherry"
	kind "SharedLib"
	language "C++"
	architecture "x64"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "CHpch.h"
	pchsource "Cherry/src/CHpch.cpp"

	files {
	
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Cherry/src",
		"Cherry/Vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links { 
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "c++17"
		staticruntime "On"
		systemversion "latest"

		defines {
		
		"CH_PLATFORM_WINDOWS",
		"CH_BUILD_DLL"
		}

		postbuildcommands {
			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"
		}

	filter "configurations:Debug"
		defines "CH_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "CH_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "CH_DIST"
		optimize "On"

	filter {"system:windows","configurations:Debug"}
		buildoptions "/utf-8"



project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
	
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
	
		"Cherry/Vendor/spdlog/include",
		"Cherry/src"
	}

	links {
	
		"Cherry"
	}
	dependson { "Cherry" }

	filter "system:windows"
		cppdialect "c++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"CH_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "CH_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "CH_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "CH_DIST"
		optimize "On"

	filter {"system:windows","configurations:Debug"}
		buildoptions "/utf-8"