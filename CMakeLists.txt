#------------------------------------------------------------------------
# ET:Legacy, http://www.etlegacy.com
# - Wolfenstein: Enemy Territory 2.60b compatible client/server
# - based on raedwulf-et: https://bitbucket.org/tcmreastwood/raedwulf-et
#
# Please use TABs to indent.
#
# TODO/FIXME
# - make uninstall doesn't delete pathes ('etlegacy' & subfolders - test case: INSTALL_DEFAULT_ dirs are equal)
#------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6)

project(ETLEGACY C CXX)
set(ETLEGACY_VERSION "2.71rc1-dirty")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(Build)
include(GetGitRevisionDescription)

# Used to store real system processor when we overwrite CMAKE_SYSTEM_PROCESSOR for cross-compile builds
set(ETLEGACY_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}) # See IS_64_BIT_BUILD_SYSTEM

# has to be set to "", otherwise CMake will pass -rdynamic resulting in a client crash
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

# this is hardly used .. we may delete that
add_definitions(-DBOTLIB)

# Options
option(BUILD_SERVER		"Build the dedicated server executable"				ON)
option(BUILD_CLIENT		"Build the client executable"						ON)
option(BUILD_MOD		"Build the mod libraries"							ON)
option(BUILD_MOD_PK3	"Pack the mod libraries into etl_bin.pk3"			OFF)
option(BUILD_PAK3_PK3	"Pack updated game scripts into pak3.pk3"			OFF)

option(BUILD_WITH_CSTDIO_INCLUDED "Build will include cstdio - it's required on some non 64bit systems see issue #123 "	OFF)

option(BUNDLED_SDL		"Use bundled SDL library instead of the system one."	OFF)
option(BUNDLED_CURL		"Use bundled CURL library instead of the system one."	OFF)
option(BUNDLED_JPEG		"Use bundled JPEG library instead of the system one."	OFF)
option(BUNDLED_LUA		"Use bundled LUA library instead of the system one."	OFF)
option(BUNDLED_OGG_VORBIS "Use bundled OGG library instead of the system one."	OFF)

# no support for 32 bit binaries on OpenBSD/amd64
if(CMAKE_SYSTEM MATCHES "OpenBSD*")
	option(CROSS_COMPILE32 "Compile 32-bit version of ET:L (64bit is incompatible with 2.60b)" OFF)
else()
	option(CROSS_COMPILE32 "Compile 32-bit version of ET:L (64bit is incompatible with 2.60b)" ON)
endif()

# Installation options
set(INSTALL_DEFAULT_BASEDIR	""					CACHE STRING "Should be CMAKE_INSTALL_PREFIX + INSTALL_DEFAULT_MODDIR")
set(INSTALL_DEFAULT_BINDIR	"bin"				CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")
set(INSTALL_DEFAULT_MODDIR	"data"	CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")

if(INSTALL_DEFAULT_BASEDIR)
	add_definitions(-DDEFAULT_BASEDIR=\"${INSTALL_DEFAULT_BASEDIR}\")
endif(INSTALL_DEFAULT_BASEDIR)

# Optional features
option(FEATURE_CURL			"Enable auto-download support using cURL (client)"			ON)
option(FEATURE_OGG_VORBIS	"Enable OGG Vorbis support (client)"						ON)
option(FEATURE_OPENAL		"Enable OpenAL sound backend (client)"						OFF)
option(FEATURE_FREETYPE		"Enable Freetype font library support (client)"				OFF)
option(FEATURE_TRACKBASE	"Enable extended server statistics (server)"				ON)
option(FEATURE_LUA			"Enable Lua support in game code"							ON)
option(FEATURE_MYSQL		"Enable MySQL support in server code" 					OFF)
option(FEATURE_MULTIVIEW	"Compile the mod files with multiview support (broken?!)"	OFF)
option(FEATURE_ANTICHEAT	"Enable server side anti-wallhack code (experimental)"		ON)
option(FEATURE_CURSES		"Enable PDCurses console"									OFF)
option(FEATURE_AUTOUPDATE	"Enable updater which downloads latest ET:L files"			ON)

if((WIN32 OR UNIX) AND NOT CMAKE_SYSTEM MATCHES "Darwin")
	option(FEATURE_OMNIBOT	"Enable omnibot build in mod code (Win & Linux server only!)"	OFF)
	option(INSTALL_OMNIBOT	"Install Omni-Bot - LINUX only"									OFF)
endif()

set(ET_FS_BASEPATH			""	CACHE STRING "Copy required genuine ET files from ET_FS_BASEPATH")
option(ET_KEY				"Copy etkey file (requires ET_FS_BASEPATH set)"					OFF)
	
#-----------------------------------------------------------------
# Platform-specific settings
#-----------------------------------------------------------------

if(UNIX AND CROSS_COMPILE32) # 32-bit build
	message(STATUS "System: ${CMAKE_SYSTEM} (${ETLEGACY_SYSTEM_PROCESSOR})")
	set(CMAKE_SYSTEM_PROCESSOR i386)
	message(STATUS "Forcing ${CMAKE_SYSTEM_PROCESSOR} to cross compile 32bit")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m32")
endif(UNIX AND CROSS_COMPILE32)

if(UNIX)

	# optimization/debug flags
	# Note/FIXME: check this for Darwin, explore Win optimizations options
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -ffast-math")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -ffast-math")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")


	if(CMAKE_SYSTEM MATCHES "OpenBSD*")
		set(OS_LIBRARIES m pthread)
		set(LIB_SUFFIX ".mp.obsd.")
	elseif(CMAKE_SYSTEM MATCHES "Darwin")
		set(OS_LIBRARIES dl m)
		set(CMAKE_EXE_LINKER_FLAGS "-lobjc -framework Cocoa -framework IOKit -framework CoreFoundation")
		set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem") # These flags will cause error with older Xcode
		set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem")
		if(BUILD_CLIENT)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Quartz -framework AudioUnit -framework Carbon")
		endif()
		set(LIB_SUFFIX "_mac")
		set(CMAKE_SHARED_MODULE_SUFFIX "")
	else()
		set(OS_LIBRARIES dl m rt pthread)
		set(LIB_SUFFIX ".mp.")
	endif()
	
	if(NOT MSYS)
		include(CheckCCompilerFlag)
		check_c_compiler_flag("-fvisibility=hidden" SUPPORT_VISIBILITY)
		if(SUPPORT_VISIBILITY)
			# note: visibility support starts with  __GNUC__ >= 4
			message(STATUS "Compiling with -fvisibility=hidden")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
		endif(SUPPORT_VISIBILITY)
	endif(NOT MSYS)
	
elseif(WIN32)
	add_definitions(-DWINVER=0x501)
	set(OS_LIBRARIES ws2_32 psapi winmm)
	set(LIB_SUFFIX "_mp_")
	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS) # Do not show CRT warnings
	endif(MSVC)
endif()

# Get the system architecture
if(NOT CMAKE_SYSTEM MATCHES "Darwin")
	if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86")
		if(WIN32)
			set(ARCH "x86")
		else()
			set(ARCH "i386")
		endif()
	elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
		set(ARCH "x86_64")
	else()
		set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
		message(STATUS "Warning: processor architecture not recognised (${CMAKE_SYSTEM_PROCESSOR})")
	endif()
endif(NOT CMAKE_SYSTEM MATCHES "Darwin")

#-----------------------------------------------------------------
# Sources
#-----------------------------------------------------------------

FILE(GLOB COMMON_SRC
	"src/qcommon/*.c"
	"src/qcommon/*.h"
)

FILE(GLOB COMMON_SRC_REMOVE
	"src/qcommon/dl_main_curl.c"
	"src/qcommon/dl_main_stubs.c"
	"src/botlib/botlib_stub.c"
)

LIST(REMOVE_ITEM COMMON_SRC ${COMMON_SRC_REMOVE})

# Platform specific code for server and client
if(UNIX)
	if(CMAKE_SYSTEM MATCHES "Darwin")
		LIST(APPEND PLATFORM_SRC "src/sys/sys_osx.m")
		SET_SOURCE_FILES_PROPERTIES("src/sys/sys_osx.m" PROPERTIES LANGUAGE C)
	endif(CMAKE_SYSTEM MATCHES "Darwin")

	LIST(APPEND PLATFORM_SRC "src/sys/sys_unix.c")
	LIST(APPEND PLATFORM_SRC "src/sys/con_tty.c")
elseif(WIN32)
	LIST(APPEND PLATFORM_SRC "src/sys/sys_win32.c")
	LIST(APPEND PLATFORM_SRC "src/sys/sys_win32_con.c")
	LIST(APPEND PLATFORM_SRC "src/sys/con_win32.c")
	LIST(APPEND PLATFORM_SRC "src/sys/win_resource.rc")
endif()

FILE(GLOB SERVER_SRC
	"src/server/*.c"
	"src/server/*.h"
	"src/null/*.c"
	"src/null/*.h"
	"src/botlib/be*.c"
	"src/botlib/be*.h"
	"src/botlib/l_*.c"
	"src/botlib/l_*.h"
	"src/sys/sys_main.c"
	"src/sys/con_log.c"
	"src/qcommon/dl_main_stubs.c"
)

if(MYSQL_FOUND)
	LIST(APPEND SERVER_SRC "src/qcommon/q_mysql.c")
endif(MYSQL_FOUND)

FILE(GLOB CLIENT_SRC
	"src/server/*.c"
	"src/server/*.h"
	"src/client/*.c"
	"src/client/*.h"
	"src/botlib/be*.c"
	"src/botlib/be*.h"
	"src/botlib/l_*.c"
	"src/botlib/l_*.h"
	"src/sys/sys_main.c"
	"src/sys/con_log.c"
	"src/renderer/*.c"
	"src/renderer/*.h"
	"src/sdl/*.c"
	"src/sdl/*.h"
)

FILE(GLOB CGAME_SRC
	"src/cgame/*.c"
	"src/cgame/*.h"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
	"src/ui/ui_shared.c"
	"src/game/bg_*.c"
)

FILE(GLOB QAGAME_SRC
	"src/game/*.c"
	"src/game/*.h"
	"src/game/sha-1/sha1.c"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
)

FILE(GLOB UI_SRC
	"src/ui/*.c"
	"src/ui/*.h"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
	"src/game/bg_campaign.c"
	"src/game/bg_classes.c"
	"src/game/bg_misc.c"
)

FILE(GLOB RENDERER_FILES
	"src/renderer/*.c"
)

FILE(GLOB CLIENT_FILES
	"src/client/*.c"
)

FILE(GLOB SERVER_FILES
	"src/server/*.c"
)

FILE(GLOB SYSTEM_FILES
	"src/sys/sys_main.c"
	"src/sys/con_log.c"
)

FILE(GLOB SDL_FILES
	"src/sdl/*.c"
)

FILE(GLOB BOTLIB_FILES
	"src/botlib/be*.c"
	"src/botlib/l_*.c"
)

# Version generation
git_describe(GIT_DESCRIBE)
git_describe(GIT_DESCRIBE_TAG "--abbrev=0")
if(GIT_DESCRIBE)
	set(ETL_CMAKE_VERSION ${GIT_DESCRIBE})
	set(ETL_CMAKE_VERSION_SHORT ${GIT_DESCRIBE_TAG})
else() # Not using source from git repo
	set(ETL_CMAKE_VERSION ${ETLEGACY_VERSION})
	set(ETL_CMAKE_VERSION_SHORT ${ETLEGACY_VERSION})
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/git_version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/git_version.h" @ONLY)
list(APPEND COMMON_SRC "${CMAKE_CURRENT_BINARY_DIR}/include/git_version.h")

# If we change architecture we need to force rescan of libraries
if(NOT OLD_CROSS_COMPILE32 STREQUAL CROSS_COMPILE32)
	force_rescan_library(SDL32)
	force_rescan_library(CURL)
	force_rescan_library(JPEG)
	# TODO: recheck optional libs
	set(OLD_CROSS_COMPILE32 ${CROSS_COMPILE32} CACHE INTERNAL "Previous value for CROSS_COMPILE32")
	message(STATUS "Libraries rescanned")
endif(NOT OLD_CROSS_COMPILE32 STREQUAL CROSS_COMPILE32)

# Use bundled libraries
if(BUNDLED_SDL OR BUNDLED_CURL AND FEATURE_CURL OR BUNDLED_JPEG OR BUNDLED_LUA AND FEATURE_LUA OR FEATURE_OGG_VORBIS AND NOT MINGW)
	if(EXISTS "${CMAKE_SOURCE_DIR}/libs/CMakeLists.txt")
		message(STATUS "Using bundled libraries located at ${CMAKE_SOURCE_DIR}/libs")
		add_subdirectory(libs)
	else()
		message(STATUS "======================================================")
		message(STATUS "Bundled libraries were not found on your system!")
		message(STATUS "======================================================")
		message(STATUS "You need the *multilib* package to crosscompile ET:L on a 64bit system.")
		message(STATUS "Alternatively clone etlegacy repository and then run")
		message(STATUS "        'git submodule init && git submodule update'")
		message(STATUS "and enable BUNDLED_* in CMake configuration.")
		message(FATAL_ERROR "Build stopped because of missing libraries.")
	endif()
endif(BUNDLED_SDL OR BUNDLED_CURL AND FEATURE_CURL OR BUNDLED_JPEG OR BUNDLED_LUA AND FEATURE_LUA OR FEATURE_OGG_VORBIS AND NOT MINGW)

#-----------------------------------------------------------------
# Client features
#-----------------------------------------------------------------
if(BUILD_CLIENT)
	find_package(OpenGL REQUIRED)
	list(APPEND CLIENT_LIBRARIES ${OPENGL_LIBRARIES})
	include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})

	if(NOT BUNDLED_SDL)
		find_package(SDL32 REQUIRED) # FindSDL doesn't detect 32bit lib when crosscompiling
		if(WIN32 OR CMAKE_SYSTEM MATCHES "Darwin")
			list(APPEND CLIENT_LIBRARIES ${SDL32MAIN_LIBRARY})
		endif(WIN32 OR CMAKE_SYSTEM MATCHES "Darwin")
		list(APPEND CLIENT_LIBRARIES ${SDL32_LIBRARIES})
		include_directories(SYSTEM ${SDL32_INCLUDE_DIR})
	else()
		list(APPEND CLIENT_LIBRARIES ${SDL32_BUNDLED_LIBRARIES})
		include_directories(SYSTEM ${SDL32_BUNDLED_INCLUDE_DIR})
		add_definitions(-DBUNDLED_SDL)
	endif()

	if(NOT BUNDLED_JPEG)
		find_package(JPEG 8 REQUIRED)
		list(APPEND CLIENT_LIBRARIES ${JPEG_LIBRARIES})
		include_directories(SYSTEM ${JPEG_INCLUDE_DIR})

		# Check for libjpeg v8
		include(CheckFunctionExists)
		set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
		set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARY})
		# FIXME: function is checked, but HAVE_JPEG_MEM_SRC is empty. Why?
		check_function_exists("jpeg_mem_src" HAVE_JPEG_MEM_SRC)
	else()
		list(APPEND CLIENT_LIBRARIES ${JPEG_BUNDLED_LIBRARIES})
		include_directories(SYSTEM ${JPEG_BUNDLED_INCLUDE_DIR})
	endif()

	if(FEATURE_CURL)
		if(NOT BUNDLED_CURL)
			find_package(CURL REQUIRED)
			list(APPEND CLIENT_LIBRARIES ${CURL_LIBRARIES})
			include_directories(SYSTEM ${CURL_INCLUDE_DIR})
			if(MINGW)
				add_definitions(-DCURL_STATICLIB)
			endif(MINGW)
		else() # BUNDLED_CURL
			list(APPEND CLIENT_LIBRARIES ${CURL_BUNDLED_LIBRARY}) # NOTE: LIBRARY not LIBRARIES
			include_directories(SYSTEM ${CURL_BUNDLED_INCLUDE_DIR})
			add_definitions(-DCURL_STATICLIB)
		endif()
		set(CLIENT_SRC ${CLIENT_SRC} "src/qcommon/dl_main_curl.c")
	else(FEATURE_CURL)
		set(CLIENT_SRC ${CLIENT_SRC} "src/qcommon/dl_main_stubs.c")
	endif(FEATURE_CURL)

	if(FEATURE_AUTOUPDATE)
		add_definitions(-DFEATURE_AUTOUPDATE)
	endif(FEATURE_AUTOUPDATE)

	if(FEATURE_FREETYPE)
		find_package(Freetype REQUIRED)
		list(APPEND CLIENT_LIBRARIES ${FREETYPE_LIBRARIES})
		include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
		add_definitions(-DFEATURE_FREETYPE)
	endif(FEATURE_FREETYPE)

	if(FEATURE_OPENAL)
		find_package(OpenAL REQUIRED)
		list(APPEND CLIENT_LIBRARIES ${OPENAL_LIBRARIES})
		include_directories(SYSTEM ${OPENAL_INCLUDE_DIR})
		add_definitions(-DFEATURE_OPENAL)
		add_definitions(-DFEATURE_OPENAL_DLOPEN)
	endif(FEATURE_OPENAL)

	if(FEATURE_OGG_VORBIS)
		if(NOT BUNDLED_OGG_VORBIS)
			find_package(Vorbis REQUIRED)
			list(APPEND CLIENT_LIBRARIES ${VORBIS_FILE_LIBRARY}) # This links only libvorbisfile
			include_directories(SYSTEM ${VORBIS_INCLUDE_DIR})
		else() # BUNDLED_OGG_VORBIS
			list(APPEND CLIENT_LIBRARIES ${OGG_VORBIS_BUNDLED_LIBRARIES} ${OGG_BUNDLED_LIBRARIES})
			include_directories(SYSTEM ${OGG_VORBIS_BUNDLED_INCLUDE_DIR} ${OGG_BUNDLED_INCLUDE_DIR})
		endif()
		add_definitions(-DFEATURE_OGG_VORBIS)
	endif(FEATURE_OGG_VORBIS)

endif(BUILD_CLIENT)

#-----------------------------------------------------------------
# Mod features
#-----------------------------------------------------------------
if(BUILD_MOD)
	if(FEATURE_MULTIVIEW)
		add_definitions(-DFEATURE_MULTIVIEW)
	endif(FEATURE_MULTIVIEW)

	if(FEATURE_LUA)
		if(NOT BUNDLED_LUA)
			find_package(Lua 5.1 REQUIRED)
			list(APPEND MOD_LIBRARIES ${LUA_LIBRARIES})
			include_directories(SYSTEM ${LUA_INCLUDE_DIR})

			if(${LUA_VERSION_STRING} MATCHES "^5.2")
				add_definitions(-DLUA_5_2)
			elseif(NOT LUA_VERSION_STRING)
				message(WARNING "Unable to detect Lua version, assuming 5.1")
			endif()
		else() # BUNDLED_LUA
			list(APPEND MOD_LIBRARIES ${LUA_BUNDLED_LIBRARIES})
			include_directories(SYSTEM ${LUA_BUNDLED_INCLUDE_DIR})
			add_definitions(-DBUNDLED_LUA)
		endif()
		add_definitions(-DFEATURE_LUA)
	endif(FEATURE_LUA)

	if(FEATURE_OMNIBOT)	# 32bit WIN & Linux only
		# note: 'Get the system architecture' above sets ARCH for cross compile
		if(("${ARCH}" STREQUAL "i386" OR "${ARCH}" STREQUAL "x86") AND NOT CMAKE_SYSTEM MATCHES "Darwin")
			LIST(APPEND QAGAME_SRC "src/game/g_etbot_interface.cpp")
			LIST(APPEND QAGAME_SRC "src/Omnibot/Common/BotLoadLibrary.cpp")
			add_definitions(-DFEATURE_OMNIBOT)

			# FIXME: Darwin/WIN64 ?
			if("${ETLEGACY_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR BUILD_WITH_CSTDIO_INCLUDED)
				message(STATUS "Omni-Bot BotLoadLibrary includes cstdio")
				add_definitions(-DIS_64_BIT_BUILD_SYSTEM)
			endif()
		else()
			message(STATUS "Warning: omni-bot support is 32bit Win & Linux only - build skipped")
		endif()
	endif(FEATURE_OMNIBOT)
endif(BUILD_MOD)

#-----------------------------------------------------------------
# Server/Common features
#-----------------------------------------------------------------
if(FEATURE_TRACKBASE)
	add_definitions(-DFEATURE_TRACKBASE)
endif(FEATURE_TRACKBASE)

if(FEATURE_ANTICHEAT)
	add_definitions(-DFEATURE_ANTICHEAT)
endif(FEATURE_ANTICHEAT)

if(FEATURE_MYSQL)
	find_package(Mysql REQUIRED)
	list(APPEND SERVER_LIBRARIES ${MYSQL_LIBRARY})
	list(APPEND SERVER_LIBRARIES ${MYSQL_EXTRA_LIBRARIES})
	list(APPEND SERVER_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
	add_definitions(-DFEATURE_MYSQL)
	message("-- Added MySQL Includes and Client Lib to Server.")
endif(FEATURE_MYSQL)

if(FEATURE_CURSES)
	find_package(Curses REQUIRED)
	set(CURSES_NEED_NCURSES 1) # Tells FindCurses that ncurses is required
	list(APPEND CLIENT_LIBRARIES ${CURSES_LIBRARIES})
	list(APPEND SERVER_LIBRARIES ${CURSES_LIBRARIES})
	include_directories(SYSTEM ${CURSES_INCLUDE_DIR})
	list(APPEND COMMON_SRC "src/sys/con_curses.c")
	add_definitions(-DFEATURE_CURSES)
endif(FEATURE_CURSES)

#-----------------------------------------------------------------
# Build
#-----------------------------------------------------------------
include_directories(${PROJECT_BINARY_DIR}/include) # git_version.h

if(BUILD_CLIENT)
	add_executable(etl WIN32 ${COMMON_SRC} ${CLIENT_SRC} ${PLATFORM_SRC})
	source_group("Source Files\\Renderer" FILES ${RENDERER_FILES})
	source_group("Source Files\\Client" FILES ${CLIENT_FILES})
	source_group("Source Files\\Server" FILES ${SERVER_FILES})
	source_group("Source Files\\System" FILES ${SYSTEM_FILES})
	source_group("Source Files\\SDL" FILES ${SDL_FILES})
	source_group("Source Files\\Botlib" FILES ${BOTLIB_FILES})
	source_group("Source Files\\Common" FILES ${COMMON_SRC})
	source_group("Source Files\\Platform" FILES ${PLATFORM_SRC})
	if(BUNDLED_SDL)
		add_dependencies(etl bundled_sdl)
	endif(BUNDLED_SDL)
	if(BUNDLED_CURL)
		add_dependencies(etl bundled_curl)
	endif(BUNDLED_CURL)
	if(BUNDLED_JPEG)
		add_dependencies(etl bundled_jpeg)
	endif(BUNDLED_JPEG)
	if(BUNDLED_OGG_VORBIS)
		add_dependencies(etl bundled_ogg bundled_ogg_vorbis)
	endif(BUNDLED_OGG_VORBIS)
	target_link_libraries(etl
		${OS_LIBRARIES} # Has to go after cURL and SDL
		${CLIENT_LIBRARIES}
	)
	set_target_properties(etl
		PROPERTIES COMPILE_DEFINITIONS "USE_ICON")
	install(TARGETS etl
		RUNTIME DESTINATION "${INSTALL_DEFAULT_BINDIR}")
endif(BUILD_CLIENT)

if(BUILD_SERVER)
	message("-- Server Incl Dirs: ${SERVER_INCLUDE_DIRS}")
	message("-- Server Libraries: ${SERVER_LIBRARIES}")
	include_directories(${SERVER_INCLUDE_DIRS})
	add_executable(etlded WIN32 ${COMMON_SRC} ${SERVER_SRC} ${PLATFORM_SRC})
	target_link_libraries(etlded ${OS_LIBRARIES} ${SERVER_LIBRARIES})
	set_target_properties(etlded PROPERTIES COMPILE_DEFINITIONS "DEDICATED")
	install(TARGETS etlded RUNTIME DESTINATION "${INSTALL_DEFAULT_BINDIR}")
endif(BUILD_SERVER)

if(BUILD_MOD)
	#
	# cgame
	#
	add_library(cgame${LIB_SUFFIX}${ARCH} MODULE ${CGAME_SRC})
	set_target_properties(cgame${LIB_SUFFIX}${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL"
		PREFIX ""
		LIBRARY_OUTPUT_DIRECTORY "legacy"
	)

	#
	# qagame
	#
	add_library(qagame${LIB_SUFFIX}${ARCH} MODULE ${QAGAME_SRC})
	if(FEATURE_LUA)
		if(BUNDLED_LUA)
			add_dependencies(qagame${LIB_SUFFIX}${ARCH} bundled_lua)
		endif(BUNDLED_LUA)
		target_link_libraries(qagame${LIB_SUFFIX}${ARCH} ${MOD_LIBRARIES})
	endif(FEATURE_LUA)

	set_target_properties(qagame${LIB_SUFFIX}${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "GAMEDLL"
		PREFIX ""
		LIBRARY_OUTPUT_DIRECTORY "legacy"
	)

	#
	# ui
	#
	add_library(ui${LIB_SUFFIX}${ARCH} MODULE ${UI_SRC})
	set_target_properties(ui${LIB_SUFFIX}${ARCH}
		PROPERTIES
		PREFIX ""
		LIBRARY_OUTPUT_DIRECTORY "legacy"
	)
                        
	# install bins of cgame, ui and qgame 
	install(TARGETS cgame${LIB_SUFFIX}${ARCH} qagame${LIB_SUFFIX}${ARCH} ui${LIB_SUFFIX}${ARCH}
		LIBRARY DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy"
		ARCHIVE DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy"
	)
 
	#
	# etl_bin.pk3
	#
	if(BUILD_MOD_PK3)
		add_custom_target(mod_pk3 ALL DEPENDS legacy/etl_bin_${ETL_CMAKE_VERSION_SHORT}.pk3)
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/legacy/etl_bin_${ETL_CMAKE_VERSION_SHORT}.pk3
			COMMAND zip etl_bin_${ETL_CMAKE_VERSION_SHORT}.pk3 cgame${LIB_SUFFIX}${ARCH}* ui${LIB_SUFFIX}${ARCH}*
			DEPENDS cgame${LIB_SUFFIX}${ARCH} ui${LIB_SUFFIX}${ARCH}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/legacy/
		)

		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/legacy/etl_bin_${ETL_CMAKE_VERSION_SHORT}.pk3
			DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy"
		)
	endif()
endif(BUILD_MOD)

if(BUILD_PAK3_PK3)
	add_custom_target(
		pak3_pk3 ALL
		COMMAND zip -r ${CMAKE_CURRENT_BINARY_DIR}/legacy/pak3_${ETL_CMAKE_VERSION_SHORT}.pk3 *
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/etmain/
	)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/legacy/pak3_${ETL_CMAKE_VERSION_SHORT}.pk3
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy"
	)
endif(BUILD_PAK3_PK3)

if(INSTALL_OMNIBOT)
	if(UNIX AND NOT CMAKE_SYSTEM MATCHES "Darwin")
		message(STATUS "Installing Omni-Bot")

		# Note: used tar (16MB) doesn't contain windows, incomplete nav- and other unwanted files
		set(ETLEGACY_OMNIBOT_L_DL_URL "http://mirror.etlegacy.com/omnibot/omnibot-linux-latest.tar.gz")

		message(STATUS "Downloading Omni-Bot to ${CMAKE_CURRENT_BINARY_DIR}/omnibot-linux-latest.tar.gz")
		file(DOWNLOAD ${ETLEGACY_OMNIBOT_L_DL_URL} "${CMAKE_CURRENT_BINARY_DIR}/omnibot-linux-latest.tar.gz")

		message(STATUS "Unzipping Omni-Bot to ${CMAKE_CURRENT_BINARY_DIR}/omnibot")
		execute_process(
			COMMAND tar -xf ${CMAKE_CURRENT_BINARY_DIR}/omnibot-linux-latest.tar.gz
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)

		message(STATUS "Adding Omni-Bot to installer scripts")
			install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/omnibot/"
			DESTINATION "${INSTALL_DEFAULT_MODDIR}/legacy/omni-bot"
		)
	else()
		message(STATUS "Installing Omni-Bot is Linux only")
	endif(UNIX AND NOT CMAKE_SYSTEM MATCHES "Darwin")
endif(INSTALL_OMNIBOT)

#-----------------------------------------------------------------
# Installer/Package generation
#-----------------------------------------------------------------

# install adds ...

# misc/etmain/ adds
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/misc/etmain/"
	DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
)

# misc adds
if(INSTALL_OMNIBOT)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/etl_bot.sh"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}"
		PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/etlded_bot.sh"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}"
		PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif(INSTALL_OMNIBOT)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/etl.ico"
	DESTINATION "${INSTALL_DEFAULT_MODDIR}"
)

# project adds
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
	DESTINATION "${INSTALL_DEFAULT_MODDIR}"
)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt"
	DESTINATION "${INSTALL_DEFAULT_MODDIR}"
)

# copy required genuine files
if(ET_FS_BASEPATH)
	message(STATUS "Installing genuine ET files")

	install(FILES "${ET_FS_BASEPATH}/etmain/mp_bin.pk3"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	)

	install(FILES "${ET_FS_BASEPATH}/etmain/pak0.pk3"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	)

	install(FILES "${ET_FS_BASEPATH}/etmain/pak1.pk3"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	)

	install(FILES "${ET_FS_BASEPATH}/etmain/pak2.pk3"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	)

	# personal data (owner only)
	install(FILES "${ET_FS_BASEPATH}/etmain/video/etintro.roq"
		DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain/video"
		PERMISSIONS OWNER_WRITE OWNER_READ
	)
	
	if(ET_KEY)
		install(FILES "${ET_FS_BASEPATH}/etmain/etkey"
			DESTINATION "${INSTALL_DEFAULT_MODDIR}/etmain"
			PERMISSIONS OWNER_WRITE OWNER_READ
		)
	endif(ET_KEY)
else()
	message(STATUS "***********************************************************")
	message(STATUS "Genuine ET files are not copied - ET:Legacy won't start !!!")
	message(STATUS "In order to start the game copy")
	message(STATUS "mp_bin.pk3, pak0.pk3, pak1.pk3 and pak2.pk3")
	message(STATUS "to ${INSTALL_DEFAULT_MODDIR}/etmain")
	message(STATUS "***********************************************************")
endif(ET_FS_BASEPATH)

# Uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake"
	IMMEDIATE @ONLY
)
add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake
)

# CPack general configuration
set(CPACK_PACKAGE_NAME					"etlegacy")
set(CPACK_PACKAGE_CONTACT				"mail@etlegacy.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"ET: Legacy is an online FPS game compatible with Wolfenstein: Enemy Territory 2.60b.")
set(CPACK_PACKAGE_DESCRIPTION			${CPACK_PACKAGE_DESCRIPTION_SUMMARY}) # TODO: expand
set(CPACK_PACKAGE_DESCRIPTION_FILE		"${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE			"${CMAKE_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_VERSION_MAJOR			${ETLEGACY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR			${ETLEGACY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH			${ETLEGACY_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION				${ETLEGACY_VERSION})
set(CPACK_PACKAGE_FILE_NAME				"${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${ARCH}")
set(CPACK_PROJECT_CONFIG_FILE			"${CMAKE_CURRENT_BINARY_DIR}/CPackOptions.cmake")

# CPack generator-specific configuration
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackOptions.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/CPackOptions.cmake"
	IMMEDIATE @ONLY
)

include(CPack) # Has to be included after the package configuration!
