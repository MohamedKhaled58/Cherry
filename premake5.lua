workspace "Cherry"
	startproject "Sandbox"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Include directories relative to root folder (solution directory)
	IncludeDir = {}
	IncludeDir["GLFW"] = "Cherry/Vendor/GLFW/include"
	IncludeDir["Glad"] = "Cherry/Vendor/Glad/include"
	IncludeDir["ImGui"] = "Cherry/Vendor/imgui"
	IncludeDir["glm"] = "Cherry/Vendor/glm"

	-- Include GLFW project first

group "Dependencies"
	include "Cherry/vendor/GLFW"
	include "Cherry/vendor/Glad"
	include "Cherry/vendor/imgui"
group ""
	-- Cherry project
	
	project "Cherry"
		location "Cherry"
		kind "SharedLib"
		language "C++"
		staticruntime "off" -- Use static runtime library to avoid linking with the dynamic CRT"
		architecture "x64"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		pchheader "CHpch.h"
		pchsource "Cherry/src/CHpch.cpp"

		files {
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/vendor/glm/glm/**.hpp",
			"%{prj.name}/vendor/glm/glm/**.cppm",
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
			cppdialect "c++17"
			systemversion "latest"

			defines {
				"CH_PLATFORM_WINDOWS",
				"CH_BUILD_DLL",
				"GLFW_INCLUDE_NONE"
			}

			postbuildcommands {
				("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
			}

		filter "configurations:Debug"
			defines "CH_DEBUG"
			runtime "Debug"		--MultiThreaded Debug Dll
			symbols "On"

		filter "configurations:Release"
			defines "CH_RELEASE"
			runtime "Release"	--MultiThreaded Dll
			optimize "On"

		filter "configurations:Dist"
			defines "CH_DIST"
			runtime "Release"
			optimize "On"

		filter {"system:windows","configurations:Debug"}
			buildoptions "/utf-8"

	-- Sandbox project
	project "Sandbox"
		location "Sandbox"
		kind "ConsoleApp"
		language "C++"
		staticruntime "off" -- Use static runtime library to avoid linking with the dynamic CRT"


		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files {
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs {
			"Cherry/Vendor/spdlog/include",
			"Cherry/src",
			"%{IncludeDir.glm}"
		}

		links {
			"Cherry"
		}
		dependson { "Cherry" }

		filter "system:windows"
			cppdialect "c++17"
			systemversion "latest"

			defines {
				"CH_PLATFORM_WINDOWS"
			}

		filter "configurations:Debug"
			defines "CH_DEBUG"
			runtime "Debug"	--MultiThreaded Debug Dll
			symbols "On"

		filter "configurations:Release"
			defines "CH_RELEASE"
			runtime "Release"
			optimize "On"

		filter "configurations:Dist"
			defines "CH_DIST"
			runtime "Release"
			optimize "On"

		filter {"system:windows","configurations:Debug"}
			buildoptions "/utf-8"