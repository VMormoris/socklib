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
	cppdialect "C++17"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
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


project "tests"
	location "tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}


	includedirs
	{
		"socklib/src",
		"%{IncludeDirs.catch}",
	}

	links
	{
		"socklib"
	}

	filter "system:linux"
		links
		{
			"pthread"
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
