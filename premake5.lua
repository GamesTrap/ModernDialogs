workspace "ModernFileDialogs"
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

project "ModernFileDialogs"
	location "ModernFileDialogs"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	cppdialect "C++17"
	systemversion "latest"

	targetdir ("bin/" .. outputdir .. "/%{prj.group}/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.group}/%{prj.name}")

	--Add all source and header files
	files
	{
		"ModernFileDialogs/**.h",
		"ModernFileDialogs/**.cpp"
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
	staticruntime "on"
	cppdialect "C++17"
	systemversion "latest"

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
		"ModernFileDialogs/"
	}

	links
	{
		"ModernFileDialogs"
	}

	filter "configurations:Debug*"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release*"
		runtime "Release"
		optimize "On"