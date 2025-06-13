workspace "Cherry"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Cherry"
	location "Cherry"
	kind "SharedLib"
	language "C++"
	architecture "x64"

	targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDir .. "/%{prj.name}")

	pchheader "CHpch.h"
	pchsource "Cherry/src/CHpch.cpp"

	files {
	
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"Cherry/src",
		"Cherry/Vendor/spdlog/include"
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
			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. OutputDir .. "/Sandbox"
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

	targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDir .. "/%{prj.name}")

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