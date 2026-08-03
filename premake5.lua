workspace "tapi"
	configurations { "Debug", "Release" }
	platforms { "Win32", "x86_64", "ARM64" }
	location "vs"
	
	filter "platforms:Win32"
		architecture "x86"

	filter "platforms:x86_64"
		architecture "x86_64"

	filter "platforms:ARM64"
		architecture "ARM64"

project "tapi"
	kind "SharedLib"
	language "C"
	cdialect "C17"
	location "vs"

	files {
		"src/**.c",
		"src/**.h",
		"include/**.h"
	}

	includedirs {
		"include",
		"src/int",
		"vendor/capstone/include"
	}

	buildoptions { "/experimental:c11atomics" }
	defines { "_CRT_SECURE_NO_WARNINGS" }

	links {
		"capstone"
	}

	filter "platforms:Win32"
		libdirs { "vendor/capstone/buildwin32/Release" }
		targetdir "bin/win32/%{cfg.buildcfg}"
		objdir "build/win32/%{cfg.buildcfg}"
	
	filter "platforms:x86_64" 
		libdirs { "vendor/capstone/buildx64/Release" }
		targetdir "bin/win64/%{cfg.buildcfg}"
		objdir "build/win64/%{cfg.buildcfg}"
	
	filter "configurations:Debug"
		symbols "On"
		optimize "Off"

	filter "configurations:Release"
		symbols "Off"
		optimize "Speed"

	filter {}

project "tapi-integration"
	kind "ConsoleApp"
	language "C"
	cdialect "C17"
	location "vs"

	files {
		"tests/integration/**.c",
		"tests/integration/**.h"
	}

	includedirs {
		"include",
		"src",
		"src/int",
		"vendor/capstone/include"
	}

	links {
		"capstone",
		"tapi"
	}

	dependson {
		"tapi"
	}

	buildoptions { "/experimental:c11atomics" }
	defines { "_CRT_SECURE_NO_WARNINGS" }

	filter "platforms:Win32"
		libdirs { 
			"vendor/capstone/buildwin32/Release",
			"bin/win32/%{cfg.buildcfg}"
		}
		targetdir "bin/win32/%{cfg.buildcfg}/tests/integration"
		objdir "build/win32/%{cfg.buildcfg}/tests/integration"
		postbuildcommands { 
			"{COPYFILE} ../bin/win32/%{cfg.buildcfg}/tapi.dll %{cfg.targetdir}",
			"{COPYFILE} ../bin/win32/%{cfg.buildcfg}/tapi.pdb %{cfg.targetdir}"
		}
	
	filter "platforms:x86_64" 
		libdirs { 
			"vendor/capstone/buildwin64/Release",
			"bin/win64/%{cfg.buildcfg}"
		}
		targetdir "bin/win64/%{cfg.buildcfg}/tests/integration"
		objdir "build/win64/%{cfg.buildcfg}/tests/integration"
		postbuildcommands { 
			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.dll %{cfg.targetdir}",
			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.pdb %{cfg.targetdir}"
		}

project "tapi-unit"
	kind "ConsoleApp"
	language "C"
	cdialect "C17"
	location "vs"

	files {
		"tests/unit/**.c",
		"tests/unit/**.h",
		"src/int/det.c",
		"src/int/patch.c",
		"src/int/guard.c",
		"src/int/arch.c",
		"src/int/reloc.c",
		"src/int/sig.c",
		"src/int/map.c",
		"src/int/lnk.c",
		"src/dyna.c",
		"src/tapi.c",
		"src/mock.c",
		"src/capture.c",
		"src/sink.c"
	}

	includedirs {
		"include",
		"src",
		"src/int",
		"tests/unit/",
		"vendor/capstone/include"
	}

	links {
		"capstone"
	}

	buildoptions { "/experimental:c11atomics" }
	defines { "_CRT_SECURE_NO_WARNINGS" }

	filter "platforms:Win32"
		libdirs { "vendor/capstone/buildwin32/Release" }
		targetdir "bin/win32/%{cfg.buildcfg}/tests/unit"
		objdir "build/win32/%{cfg.buildcfg}/tests/unit"
	
	filter "platforms:x86_64" 
		libdirs { "vendor/capstone/buildx64/Release" }
		targetdir "bin/win64/%{cfg.buildcfg}/tests/unit"
		objdir "build/win64/%{cfg.buildcfg}/tests/unit"
