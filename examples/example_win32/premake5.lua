--
-- Windows
--

project "example_win32"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	callingconvention "FastCall"
	floatingpoint "Fast"

    outputdir = "%{cfg.buildcfg}";
	targetdir (outputdir)
	objdir (outputdir)

	files {
		"../../src/*.h",
		"../../src/*.cpp",
		"**.h",
		"**.cpp",
		"../libs/ceal_window/**.h",
		"../libs/ceal_window/**.cpp"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs {
		"../../",
		"../../src",
        "../libs/ceal_window",
        "../libs/imgui",
        "../"
	}
	links {
        "../libs/imgui/lib/ImGui"
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