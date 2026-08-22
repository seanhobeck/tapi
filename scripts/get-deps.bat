@echo off
setlocal

if exist vendor\capstone\buildwin32 (
	rmdir /s /q vendor\capstone\buildwin32
)

if exist vendor\capstone\buildwin64 (
	rmdir /s /q vendor\capstone\buildwin64
)

echo building capstone for win32...
cmake -S vendor\capstone -B vendor\capstone\buildwin32 -A Win32 ^
	-DCAPSTONE_BUILD_SHARED=OFF  ^
	-DCAPSTONE_BUILD_TESTS=OFF

if errorlevel 1 exit \b 1

cmake --build vendor\capstone\buildwin32 --config Release

if errorlevel 1 exit \b 1

echo building capstone for win64...
cmake -S vendor\capstone -B vendor\capstone\buildwin64 -A x64 ^
	-DCAPSTONE_BUILD_SHARED=OFF  ^
	-DCAPSTONE_BUILD_TESTS=OFF

if errorlevel 1 exit \b 1

cmake --build vendor\capstone\buildwin64 --config Release

if errorlevel 1 exit \b 1

echo.
echo capstone build completed!
endlocal