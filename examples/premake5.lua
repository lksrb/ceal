--
-- CEAL Examples
--
workspace "ceal_examples"
	architecture "x86_64"
	startproject "example_win32"
	toolset "v143"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile", 
	}

include "example_win32"
