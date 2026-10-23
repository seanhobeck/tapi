workspace "tapi"
	configurations { "Debug" }
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
	staticruntime "Off"
	symbols "On"
	optimize "Off"
	runtime "Debug"

	links {
		"capstone"
	}

	filter "platforms:Win32"
		libdirs { "vendor/capstone/buildwin32/Release" }
		targetdir "bin/win32/%{cfg.buildcfg}"
		objdir "build/win32/%{cfg.buildcfg}"
	
	filter "platforms:x86_64" 
		libdirs { "vendor/capstone/buildwin64/Release" }
		targetdir "bin/win64/%{cfg.buildcfg}"
		objdir "build/win64/%{cfg.buildcfg}"

	filter {}

-- creating our unit, integration and smoke tests, we need a executable for each one.
local unit_tests = os.matchfiles("tests/unit/test_*.c")
local integration_tests = os.matchfiles("tests/integration/test_*.c")
local smoke_tests = os.matchfiles("tests/smoke/test_*.c")

for _, test in ipairs(unit_tests) do
    test_name = path.getbasename(test)

    -- create a project...
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
    	staticruntime "Off"
    	symbols "On"
    	optimize "Off"
    	runtime "Debug"

    	filter "platforms:Win32"
    		libdirs { "vendor/capstone/buildwin32/Release" }
    		targetdir "bin/win32/%{cfg.buildcfg}/tests/unit"
    		objdir "build/win32/%{cfg.buildcfg}/tests/unit" .. test_name

    	filter "platforms:x86_64"
    		libdirs { "vendor/capstone/buildwin64/Release" }
    		targetdir "bin/win64/%{cfg.buildcfg}/tests/unit"
    		objdir "build/win64/%{cfg.buildcfg}/tests/unit" .. test_name

    	filter {}
end

for _, test in ipairs(integration_tests) do
    test_name = path.getbasename(test)

    -- create a project...
    project (test_name)
    	kind "ConsoleApp"
    	language "C"
    	cdialect "C17"
    	location "vs"

    	files {
    		test,
    		"tests/integration/def.h"
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
    	staticruntime "Off"
    	symbols "On"
    	optimize "Off"
    	runtime "Debug"

    	filter "platforms:Win32"
    		libdirs {
    			"vendor/capstone/buildwin32/Release",
    			"bin/win32/%{cfg.buildcfg}"
    		}
    		targetdir "bin/win32/%{cfg.buildcfg}/tests/integration/"
    		objdir "build/win32/%{cfg.buildcfg}/tests/integration" .. test_name
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
    		objdir "build/win64/%{cfg.buildcfg}/tests/integration" .. test_name
    		postbuildcommands {
    			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.dll %{cfg.targetdir}",
    			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.pdb %{cfg.targetdir}"
    		}

    	filter {}
end

for _, test in ipairs(smoke_tests) do
    test_name = path.getbasename(test)

    -- create a project...
    project (test_name)
    	kind "ConsoleApp"
    	language "C"
    	cdialect "C17"
    	location "vs"

    	files {
    		"tests/smoke/**.c",
    		"tests/smoke/**.h"
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
    	staticruntime "Off"
    	symbols "On"
    	optimize "Off"
    	runtime "Debug"

    	filter "platforms:Win32"
    		libdirs {
    			"vendor/capstone/buildwin32/Release",
    			"bin/win32/%{cfg.buildcfg}"
    		}
    		targetdir "bin/win32/%{cfg.buildcfg}/tests/smoke"
    		objdir "build/win32/%{cfg.buildcfg}/tests/smoke" .. test_name
    		postbuildcommands {
    			"{COPYFILE} ../bin/win32/%{cfg.buildcfg}/tapi.dll %{cfg.targetdir}",
    			"{COPYFILE} ../bin/win32/%{cfg.buildcfg}/tapi.pdb %{cfg.targetdir}"
    		}

    	filter "platforms:x86_64"
    		libdirs {
    			"vendor/capstone/buildwin64/Release",
    			"bin/win64/%{cfg.buildcfg}"
    		}
    		targetdir "bin/win64/%{cfg.buildcfg}/tests/smoke"
    		objdir "build/win64/%{cfg.buildcfg}/tests/smoke" .. test_name
    		postbuildcommands {
    			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.dll %{cfg.targetdir}",
    			"{COPYFILE} ../bin/win64/%{cfg.buildcfg}/tapi.pdb %{cfg.targetdir}"
    		}

    	filter {}
end