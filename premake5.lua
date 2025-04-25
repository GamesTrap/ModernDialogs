workspace "ModernDialogs"
	configurations { "Debug32", "Release32", "Debug64", "Release64" }

	startproject "Example"

	flags
	{
		"MultiProcessorCompile"
	}

	filter "configurations:*32"
    	architecture "x86"

    filter "configurations:*64"
    	architecture "x86_64"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

newoption {
	trigger = "std",
	value = "C++XX",
	description = "Select the C++ standard to use for compilation, defaults to C++-17",
	allowed = {
		{ "C++latest", "Latest C++ standard availble for the toolset"},
		{ "C++17", "ISO C++17 standard"},
		{ "C++2a", "ISO C++20 draft"},
		{ "C++20", "ISO C++20 standard"},
		{ "C++2b", "ISO C++23 draft"},
		{ "C++23", "ISO C++23 standard"},
	},
	default = "C++17"
}

project "ModernDialogs"
	location "ModernDialogs"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	systemversion "latest"
	warnings "Extra"

	cppdialect (_OPTIONS["std"] or "C++17")

	targetdir ("bin/" .. outputdir .. "/%{prj.group}/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.group}/%{prj.name}")

	--Add all source and header files
	files
	{
		"ModernDialogs/**.h",
		"ModernDialogs/**.cpp"
	}

	filter "configurations:Debug*"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release*"
		runtime "Release"
		optimize "On"

project "Example"
	location "Example"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	cppdialect "C++17"
	systemversion "latest"
	warnings "Extra"

	targetdir ("bin/" .. outputdir .. "/%{prj.group}/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.group}/%{prj.name}")

	--Add all source and header files
	files
	{
		"Example/**.h",
		"Example/**.cpp"
	}

	includedirs
	{
		"ModernDialogs/"
	}

	links
	{
		"ModernDialogs"
	}

	filter "configurations:Debug*"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release*"
		runtime "Release"
		optimize "On"