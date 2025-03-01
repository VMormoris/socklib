workspace "socklib"

	configurations
	{
		"Debug",
		"Release"
	}

	startproject "tests"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

IncludeDirs={}
IncludeDirs["catch"]="3rdParty/Catch2/include"

project "socklib"
	location "socklib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
	objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/socklib/**.h",
		"src/**.cpp",
	}

	includedirs
	{
		"include",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"DEBUG_BUILD"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"


project "socklib-tests"
	location "tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
	objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"tests/src/**.h",
		"tests/src/**.cpp"
	}


	includedirs
	{
		"include",
		"%{IncludeDirs.catch}",
	}

	links
	{
		"socklib"
	}

	filter "system:linux"
	    defines
	    {
            "CATCH_CONFIG_NO_POSIX_SIGNALS",
        }
		links
		{
		}

		toolset("clang")

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"DEBUG_BUILD"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"