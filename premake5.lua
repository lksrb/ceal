
--CEAL Workspace

workspace "CEAL"
	architecture "x86_64"
	startproject "CEAL"
	toolset "v143"

	configurations
	{
		"Debug",
		"Release",
	}

	flags
	{
		"MultiProcessorCompile"
	}

--CEAL Project

project "CEAL"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	callingconvention "FastCall"
	floatingpoint "Fast"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs {
		"src",
	}
	links {
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"CEAL_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "CEAL_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "CEAL_RELEASE"
			runtime "Release"
			optimize "on"