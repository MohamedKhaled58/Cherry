workspace "Cherry"
	startproject "MyShell"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}
	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Include directories relative to root folder (solution directory)
	IncludeDir = {}
	IncludeDir["GLFW"]	=	"Cherry/Vendor/GLFW/include"
	IncludeDir["Glad"]	=	"Cherry/Vendor/Glad/include"
	IncludeDir["ImGui"]	=	"Cherry/Vendor/imgui"
	IncludeDir["glm"]	=	"Cherry/Vendor/glm"

	-- Include GLFW project first

group "Dependencies"
	include "Cherry/vendor/GLFW"
	include "Cherry/vendor/Glad"
	include "Cherry/vendor/imgui"
group ""
	-- Cherry project
	
	project "Cherry"
		location "Cherry"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		pchheader "CHpch.h"
		pchsource "Cherry/src/CHpch.cpp"

		files {
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/vendor/glm/glm/**.hpp",
			"%{prj.name}/vendor/glm/glm/**.inl"
		}

		includedirs {
			"Cherry/src",
			"Cherry/Vendor/spdlog/include",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.Glad}",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.glm}"
		}

		links { 
			"GLFW",
			"Glad",
			"ImGui",
			"opengl32.lib"
		}

		filter "system:windows"
			cppdialect "c++20"
			systemversion "latest"

		defines {
			"CH_PLATFORM_WINDOWS",
			"CH_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		filter "system:windows"
			systemversion "latest"
			buildoptions "/utf-8"
			
		filter "configurations:Debug"
			defines "CH_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "CH_RELEASE"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines "CH_DIST"
			runtime "Release"
			optimize "on"

	-- MyShell project
	project "MyShell"
		location "MyShell"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++20"
		staticruntime "off"


		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files {
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs {
			"Cherry/Vendor/spdlog/include",
			"Cherry/src",
			"%{IncludeDir.glm}",
			"Cherry/vendor"
		}

		links {
			"Cherry"
		}
		dependson { "Cherry" }

		filter "system:windows"
			cppdialect "c++20"
			systemversion "latest"

		defines {
			"CH_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "CH_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "CH_RELEASE"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines "CH_DIST"
			runtime "Release"
			optimize "on"

		filter {"system:windows","configurations:Debug"}
			buildoptions "/utf-8"