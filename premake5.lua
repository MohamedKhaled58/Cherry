workspace "Cherry"
	architecture "x64"
	startproject "MyShell"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Cherry/vendor/GLFW/include"
IncludeDir["Glad"] = "Cherry/vendor/Glad/include"
IncludeDir["ImGui"] = "Cherry/vendor/imgui"
IncludeDir["glm"] = "Cherry/vendor/glm"
IncludeDir["stb_image"] = "Cherry/vendor/stb_image"

group "Dependencies"
	include "Cherry/vendor/GLFW"
	include "Cherry/vendor/Glad"
	include "Cherry/vendor/imgui"

group ""

project "Cherry"
	location "Cherry"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "CHpch.h"
	pchsource "Cherry/src/CHpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

    -- Windows-specific settings
    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }

		defines
		{
			"CH_PLATFORM_WINDOWS",
			"CH_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
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

project "MyShell"
	location "MyShell"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Cherry/vendor/spdlog/include",
		"Cherry/src",
		"Cherry/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Cherry"
	}

    -- Windows-specific settings
    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }

		defines
		{
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