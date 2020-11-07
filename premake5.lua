workspace "ModernFileDialogs"
architecture "x86_64"
	startproject "ModernFileDialogs"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "ModernFileDialogs"
	location "ModernFileDialogs"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	cppdialect "C++17"
	systemversion "latest"

	targetdir ("../bin/" .. outputdir .. "/%{prj.group}/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.group}/%{prj.name}")

	--Add all source and header files
	files
	{
		"**.hpp",
		"**.cpp"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
