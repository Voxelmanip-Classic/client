cmake_minimum_required(VERSION 3.5)

include(cmake/add_pch.cmake)

# Set policies up to 3.9 since we want to enable the IPO option
if(${CMAKE_VERSION} VERSION_LESS 3.9)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
	cmake_policy(VERSION 3.9)
endif()

# This can be read from ${PROJECT_NAME} after project() is called
project(voxelmanip_classic)
set(PROJECT_NAME_CAPITALIZED "Voxelmanip Classic")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(GCC_MINIMUM_VERSION "7.5")
set(CLANG_MINIMUM_VERSION "6.0")

# You should not need to edit these manually, use util/bump_version.sh
set(VERSION_MAJOR 2023)
set(VERSION_MINOR 12)
set(VERSION_PATCH 23)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

# Change to false for releases
set(DEVELOPMENT_BUILD FALSE)

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
if(VERSION_EXTRA)
	set(VERSION_STRING "${VERSION_STRING}-${VERSION_EXTRA}")
elseif(DEVELOPMENT_BUILD)
	set(VERSION_STRING "${VERSION_STRING}-dev")
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	# Append "-debug" to version string
	set(VERSION_STRING "${VERSION_STRING}-debug")
endif()

message(STATUS "*** Will build version ${VERSION_STRING} ***")

set(RUN_IN_PLACE FALSE CACHE BOOL "Run directly in source directory structure")

set(BUILD_CLIENT TRUE CACHE BOOL "Build client")
set(BUILD_SERVER FALSE CACHE BOOL "Build server")

set(WARN_ALL TRUE CACHE BOOL "Enable -Wall for Release build")

if(NOT CMAKE_BUILD_TYPE)
	# Default to release
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type: Debug or Release" FORCE)
endif()

# Included stuff
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Load default options for Android
if(ANDROID)
	cmake_minimum_required(VERSION 3.20)
	include(MinetestAndroidLibs)
endif()


set(IRRLICHTMT_BUILD_DIR "" CACHE PATH "Path to IrrlichtMt build directory.")
if(NOT "${IRRLICHTMT_BUILD_DIR}" STREQUAL "")
	find_package(IrrlichtMt QUIET
		PATHS "${IRRLICHTMT_BUILD_DIR}"
		NO_DEFAULT_PATH
	)

	if(NOT TARGET IrrlichtMt::IrrlichtMt)
		# find_package() searches certain subdirectories. ${PATH}/cmake is not
		# the only one, but it is the one where IrrlichtMt is supposed to export
		# IrrlichtMtConfig.cmake
		message(FATAL_ERROR "Could not find IrrlichtMtConfig.cmake in ${IRRLICHTMT_BUILD_DIR}/cmake.")
	endif()
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/irrlichtmt")
	message(STATUS "Using user-provided IrrlichtMt at subdirectory 'lib/irrlichtmt'")
	if(BUILD_CLIENT)
		# tell IrrlichtMt to create a static library
		set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library" FORCE)
		add_subdirectory(lib/irrlichtmt EXCLUDE_FROM_ALL)
		unset(BUILD_SHARED_LIBS CACHE)

		if(NOT TARGET IrrlichtMt)
			message(FATAL_ERROR "IrrlichtMt project is missing a CMake target?!")
		endif()
	else()
		add_library(IrrlichtMt::IrrlichtMt INTERFACE IMPORTED)
		set_target_properties(IrrlichtMt::IrrlichtMt PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/irrlichtmt/include")
	endif()
else()
	find_package(IrrlichtMt QUIET)
	if(NOT TARGET IrrlichtMt::IrrlichtMt)
		string(CONCAT explanation_msg
			"The Minetest team has forked Irrlicht to make their own customizations. "
			"It can be found here: https://github.com/minetest/irrlicht\n"
			"For example use: git clone --depth=1 https://github.com/minetest/irrlicht lib/irrlichtmt\n")
		if(BUILD_CLIENT)
			message(FATAL_ERROR "IrrlichtMt is required to build the client, but it was not found.\n${explanation_msg}")
		endif()

		include(MinetestFindIrrlichtHeaders)
		if(NOT IRRLICHT_INCLUDE_DIR)
			message(FATAL_ERROR "IrrlichtMt headers are required to build the server, but none found.\n${explanation_msg}")
		endif()
		message(STATUS "Found IrrlichtMt headers: ${IRRLICHT_INCLUDE_DIR}")
		add_library(IrrlichtMt::IrrlichtMt INTERFACE IMPORTED)
		# Note that we can't use target_include_directories() since that doesn't work for IMPORTED targets before CMake 3.11
		set_target_properties(IrrlichtMt::IrrlichtMt PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${IRRLICHT_INCLUDE_DIR}")
	endif()
endif()

if(BUILD_CLIENT AND TARGET IrrlichtMt::IrrlichtMt)
	# retrieve version somehow
	if(NOT IrrlichtMt_VERSION)
		get_target_property(IrrlichtMt_VERSION IrrlichtMt VERSION)
	endif()
	message(STATUS "Found IrrlichtMt ${IrrlichtMt_VERSION}")

	set(TARGET_VER_S 1.9.0mt12)
	string(REPLACE "mt" "." TARGET_VER ${TARGET_VER_S})
	if(IrrlichtMt_VERSION VERSION_LESS ${TARGET_VER})
		message(FATAL_ERROR "At least IrrlichtMt ${TARGET_VER_S} is required to build")
	elseif(NOT DEVELOPMENT_BUILD AND IrrlichtMt_VERSION VERSION_GREATER ${TARGET_VER})
		message(FATAL_ERROR "IrrlichtMt ${TARGET_VER_S} is required to build")
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${GCC_MINIMUM_VERSION}")
		message(FATAL_ERROR "Insufficient gcc version, found ${CMAKE_CXX_COMPILER_VERSION}. "
			"Version ${GCC_MINIMUM_VERSION} or higher is required.")
	endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${CLANG_MINIMUM_VERSION}")
		message(FATAL_ERROR "Insufficient clang version, found ${CMAKE_CXX_COMPILER_VERSION}. "
			"Version ${CLANG_MINIMUM_VERSION} or higher is required.")
	endif()
endif()

# Installation

if(WIN32)
	set(SHAREDIR ".")
	if(RUN_IN_PLACE)
		set(BINDIR "bin")
	else()
		set(BINDIR ".")
	endif()
	set(EXAMPLE_CONF_DIR ".")
	set(LOCALEDIR "locale")
elseif(APPLE)
	set(BUNDLE_NAME ${PROJECT_NAME}.app)
	set(BUNDLE_PATH "${BUNDLE_NAME}")
	set(BINDIR ${BUNDLE_NAME}/Contents/MacOS)
	set(SHAREDIR ${BUNDLE_NAME}/Contents/Resources)
	set(LOCALEDIR "${SHAREDIR}/locale")
elseif(UNIX) # Linux, BSD etc
	if(RUN_IN_PLACE)
		set(SHAREDIR ".")
		set(BINDIR "bin")
		set(MANDIR "unix/man")
		set(XDG_APPS_DIR "unix/applications")
		set(ICONDIR "unix/icons")
		set(LOCALEDIR "locale")
	else()
		include(GNUInstallDirs)
		set(SHAREDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
		set(BINDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
		set(MANDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}")
		set(XDG_APPS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications")
		set(ICONDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons")
		set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALEDIR}")
	endif()
endif()

set(CUSTOM_SHAREDIR "" CACHE STRING "Directory to install data files into")
if(NOT CUSTOM_SHAREDIR STREQUAL "")
	set(SHAREDIR "${CUSTOM_SHAREDIR}")
	message(STATUS "Using SHAREDIR=${SHAREDIR}")
endif()

set(CUSTOM_BINDIR "" CACHE STRING "Directory to install binaries into")
if(NOT CUSTOM_BINDIR STREQUAL "")
	set(BINDIR "${CUSTOM_BINDIR}")
	message(STATUS "Using BINDIR=${BINDIR}")
endif()

set(CUSTOM_MANDIR "" CACHE STRING "Directory to install manpages into")
if(NOT CUSTOM_MANDIR STREQUAL "")
	set(MANDIR "${CUSTOM_MANDIR}")
	message(STATUS "Using MANDIR=${MANDIR}")
endif()

set(CUSTOM_XDG_APPS_DIR "" CACHE STRING "Directory to install .desktop files into")
if(NOT CUSTOM_XDG_APPS_DIR STREQUAL "")
	set(XDG_APPS_DIR "${CUSTOM_XDG_APPS_DIR}")
	message(STATUS "Using XDG_APPS_DIR=${XDG_APPS_DIR}")
endif()

set(CUSTOM_ICONDIR "" CACHE STRING "Directory to install icons into")
if(NOT CUSTOM_ICONDIR STREQUAL "")
	set(ICONDIR "${CUSTOM_ICONDIR}")
	message(STATUS "Using ICONDIR=${ICONDIR}")
endif()

set(CUSTOM_LOCALEDIR "" CACHE STRING "Directory to install l10n files into")
if(NOT CUSTOM_LOCALEDIR STREQUAL "")
	set(LOCALEDIR "${CUSTOM_LOCALEDIR}")
	message(STATUS "Using LOCALEDIR=${LOCALEDIR}")
endif()


install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/builtin" DESTINATION "${SHAREDIR}")

if(BUILD_CLIENT)
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/client/shaders" DESTINATION "${SHAREDIR}/client")
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/textures/base/pack" DESTINATION "${SHAREDIR}/textures/base")

	if(UNIX AND NOT APPLE)
		install(FILES "misc/voxelmanip_classic.desktop" DESTINATION "${XDG_APPS_DIR}")
		install(FILES "misc/icon-128.png"
			DESTINATION "${ICONDIR}/hicolor/128x128/apps"
			RENAME "voxelmanip_classic.png")
	endif()

	if(APPLE)
		install(FILES "misc/minetest-icon.icns" DESTINATION "${SHAREDIR}")
		install(FILES "misc/Info.plist" DESTINATION "${BUNDLE_PATH}/Contents")
	endif()
endif()

# Library pack
find_package(GMP REQUIRED)
find_package(Json REQUIRED)
find_package(Lua REQUIRED)

# Subdirectories
# Be sure to add all relevant definitions above this
add_subdirectory(src)

# CPack

if(WIN32)
	# Include all dynamically linked runtime libraries such as MSVCRxxx.dll
	include(InstallRequiredSystemLibraries)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win64")
	else()
		set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win32")
	endif()

	set(CPACK_GENERATOR ZIP)
elseif(APPLE)
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-osx")
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-linux")
	set(CPACK_GENERATOR TGZ)
	set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)
