@echo off
REM This batch file does an out-of-source CMake build in ../build_windows
REM This is for users who like to configure & build Blender with a single command.

set BLENDER_DIR=%~dp0
set BUILD_DIR=%BLENDER_DIR%..\build_windows
set BUILD_TYPE=Release
set BUILD_CMAKE_ARGS=

REM Sanity Checks
where /Q msbuild
if %ERRORLEVEL% NEQ 0 (
	echo Error: "MSBuild" command not in the PATH.
	echo You must have MSVC installed and run this from the "Developer Command Prompt"
	echo ^(available from Visual Studio's Start menu entry^), aborting!
	goto EOF
)
where /Q cmake
if %ERRORLEVEL% NEQ 0 (
	echo Error: "CMake" command not in the PATH.
	echo You must have CMake installed and added to your PATH, aborting!
	goto EOF
)
if NOT EXIST %BLENDER_DIR%..\lib\nul (
	echo Error: Path to libraries not found "%BLENDER_DIR%..\lib\"
	echo This is needed for building, aborting!
	goto EOF
)


:argv_loop
if NOT "%1" == "" (

	REM Help Message
	if "%1" == "--help" (
		echo.
		echo Convenience targets
		echo - debug
		echo - full
		echo - lite
		echo - headless
		echo - cycles
		echo - bpy
		echo.
		echo Utilities ^(not associated with building^)
		echo - update
		goto EOF
	)

	REM Build Types
	if "%1" == "debug" (
		set BUILD_DIR=%BUILD_DIR%_debug
		set BUILD_TYPE=Debug

	REM Build Configurations
	) else if "%1" == "full" (
		set BUILD_DIR=%BUILD_DIR%_full
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%BLENDER_DIR%\build_files\cmake\config\blender_full.cmake"
	) else if "%1" == "lite" (
		set BUILD_DIR=%BUILD_DIR%_lite
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%BLENDER_DIR%\build_files\cmake\config\blender_lite.cmake"
	) else if "%1" == "cycles" (
		set BUILD_DIR=%BUILD_DIR%_cycles
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%BLENDER_DIR%\build_files\cmake\config\cycles_standalone.cmake"
	) else if "%1" == "headless" (
		set BUILD_DIR=%BUILD_DIR%_headless
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%BLENDER_DIR%\build_files\cmake\config\blender_headless.cmake"
	) else if "%1" == "bpy" (
		set BUILD_DIR=%BUILD_DIR%_bpy
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%BLENDER_DIR%\build_files\cmake\config\bpy_module.cmake"

	REM Non-Build Commands
	) else if "%1" == "update" (
		svn up ../lib/*
		git pull --rebase
		git submodule foreach git pull --rebase origin master
		goto EOF
	) else (
		echo Command "%1" unknown, aborting!
		goto EOF
	)

	shift /1
	goto argv_loop
)

if "%PROCESSOR_ARCHITECTURE%" == "x86" (
	set WINDOWS_ARCH=
) else (
	set WINDOWS_ARCH=Win64
)

set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -G "Visual Studio 12 2013 %WINDOWS_ARCH%"
if NOT EXIST %BUILD_DIR%\nul (
	mkdir %BUILD_DIR%
)

cmake ^
	%BUILD_CMAKE_ARGS% ^
	-H%BLENDER_DIR% ^
	-B%BUILD_DIR% ^
	%BUILD_CMAKE_ARGS%

if %ERRORLEVEL% NEQ 0 (
	echo "Configuration Failed"
	goto EOF
)

msbuild ^
	%BUILD_DIR%\Blender.sln ^
	/target:build ^
	/property:Configuration=%BUILD_TYPE% ^
	/maxcpucount ^
	/verbosity:minimal

if %ERRORLEVEL% NEQ 0 (
	echo "Build Failed"
	goto EOF
)

msbuild ^
	%BUILD_DIR%\INSTALL.vcxproj ^
	/property:Configuration=%BUILD_TYPE% ^
	/verbosity:minimal

echo.
echo Your build is complete at: "%BUILD_DIR%\bin\%BUILD_TYPE%"
echo.

:EOF

