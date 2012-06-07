#------------------------------------------------------------------------
# ET:Legacy, http://www.etlegacy.com
# - Wolfenstein: Enemy Territory 2.60b compatible client/server
# - based on raedwulf-et: https://bitbucket.org/tcmreastwood/raedwulf-et
#------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6)

project(ETLEGACY C)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release)")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(ETLEGACY_VERSION_MAJOR  "2")
set(ETLEGACY_VERSION_MINOR  "7")
set(ETLEGACY_VERSION_PATCH  "0")
set(ETLEGACY_VERSION "${ETLEGACY_VERSION_MAJOR}.${ETLEGACY_VERSION_MINOR}${ETLEGACY_VERSION_PATCH}")

# has to be set to "", otherwise CMake will pass -rdynamic resulting in a client crash
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
add_definitions(-DBOTLIB)

# Options
option(BUILD_SERVER	"Build the dedicated server executable"                                 OFF)
option(BUILD_CLIENT	"Build the client executable"                                           ON)
option(BUILD_MOD	"Build the mod libraries"                                               ON)
option(CROSS_COMPILE32  "Compile 32-bit version of ET:L (64bit is incompatible with 2.60b)"     ON)

# Optional features
option(SMP_SUPPORT 		"Enable SMP (multicore support) (client only)"			ON)
option(USE_CURL 		"Enable auto-download support using cURL (client only)" 	ON)
option(USE_CODEC_VORBIS 	"Enable OGG Vorbis support (client only)" 			OFF)
option(USE_OPENAL		"Enable OpenAL sound backend (client only)" 			OFF)
option(USE_FREETYPE		"Enable Freetype font library support (client only)" 		OFF)
option(GUIDMASTER_SUPPORT 	"Enable client registration with guidmaster (client only)" 	ON)
option(TRACKBASE_SUPPORT 	"Enable extended server statistics (server only)" 		ON)

#-----------------------------------------------------------------
# Platform-specific settings
#-----------------------------------------------------------------

if(CROSS_COMPILE32) # 32-bit build
    set(CMAKE_SYSTEM_PROCESSOR i386)
    set(CMAKE_SYSTEM_LIBRARY_PATH /lib32 /usr/lib32 /usr/local/lib32)
    set(FIND_LIBRARY_USE_LIB64_PATHS OFF)
    set(CMAKE_IGNORE_PATH /lib /usr/lib /usr/local/lib)
    set(CMAKE_C_FLAGS "-m32")
    set(CMAKE_EXE_LINKER_FLAGS "-m32")
    set(CMAKE_SHARED_LINKER_FLAGS "-m32")
    set(CMAKE_MODULE_LINKER_FLAGS "-m32")
endif(CROSS_COMPILE32)

if(UNIX)
        set(OS_LIBRARIES dl m)
elseif(WIN32)
        add_definitions(-DWINVER=0x501)
        set(OS_LIBRARIES ws2_32 psapi winmm)
endif()

# Get the system architecture
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
if(WIN32)
        set(ARCH "x86")
else()
        set(ARCH "i386")
endif()
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "sparc" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "powerpc")
        set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
else()
        set(ARCH "none")
endif()

#-----------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------
Find_Package(SDL32 REQUIRED) # FindSDL doesn't detect 32bit lib when crosscompiling
Find_Package(OpenGL REQUIRED)
Find_Package(JPEG REQUIRED)

#-----------------------------------------------------------------
# Sources
#-----------------------------------------------------------------

FILE(GLOB COMMON_SRC
	"src/qcommon/*.c"
)

FILE(GLOB COMMON_SRC_REMOVE
	"src/qcommon/dl_main_curl.c"
	"src/qcommon/dl_main_stubs.c"
	"src/botlib/botlib_stub.c"
)
LIST(REMOVE_ITEM COMMON_SRC ${COMMON_SRC_REMOVE})

FILE(GLOB SERVER_SRC
	"src/server/*.c"
	"src/null/*.c"
	"src/botlib/be*.c"
	"src/botlib/l_*.c"
	"src/sys/sys_*.c"
	"src/sys/con_log.c"
	"src/sys/con_tty.c"
	"src/qcommon/dl_main_stubs.c"
)

FILE(GLOB CLIENT_SRC
	"src/server/*.c"
	"src/client/*.c"
	"src/botlib/be*.c"
	"src/botlib/l_*.c"
	"src/sys/sys_*.c"
	"src/sys/con_log.c"
	"src/sys/con_tty.c"
	"src/renderer/*.c"
	"src/sdl/*.c"
)

FILE(GLOB CGAME_SRC
	"src/cgame/*.c"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
	"src/ui/ui_shared.c"
	"src/game/bg_*.c"
)

FILE(GLOB QAGAME_SRC
	"src/game/*.c"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
)

FILE(GLOB UI_SRC
	"src/ui/*.c"
	"src/qcommon/q_math.c"
	"src/qcommon/q_shared.c"
	"src/game/bg_campaign.c"
	"src/game/bg_classes.c"
	"src/game/bg_misc.c"
)

#-----------------------------------------------------------------
# User selected features
#-----------------------------------------------------------------
if(USE_CURL)
	add_definitions(-DUSE_CURL)
	find_package(CURL REQUIRED)
	set(CLIENT_SRC ${CLIENT_SRC} "src/qcommon/dl_main_curl.c")
else(USE_CURL)
	set(CLIENT_SRC ${CLIENT_SRC} "src/qcommon/dl_main_stubs.c")
endif(USE_CURL)

if(USE_FREETYPE)
#	add_definitions(-DUSE_FREETYPE)
# 	find_package(Freetype REQUIRED)
endif(USE_FREETYPE)

if(USE_OPENAL)
#	add_definitions(-DUSE_OPENAL)
# 	add_definitions(-DUSE_OPENAL_DLOPEN)
# 	TODO: find_package(OpenAL REQUIRED)
endif(USE_OPENAL)

if(USE_CODEC_VORBIS)
#	add_definitions(-DUSE_CODEC_VORBIS)
# 	TODO: find_package(Vorbis REQUIRED)
endif(USE_CODEC_VORBIS)

if(SMP_SUPPORT)
	add_definitions(-DSMP)
endif(SMP_SUPPORT)

if(GUIDMASTER_SUPPORT)
	add_definitions(-DGUIDMASTER_SUPPORT)
endif(GUIDMASTER_SUPPORT)

if(TRACKBASE_SUPPORT)
	add_definitions(-DTRACKBASE_SUPPORT)
endif(TRACKBASE_SUPPORT)

#-----------------------------------------------------------------
# Build
#-----------------------------------------------------------------
if(BUILD_CLIENT)
	add_executable(etl ${COMMON_SRC} ${CLIENT_SRC})
	include_directories(
		${CURL_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${JPEG_INCLUDE_DIR}
                ${SDL32_INCLUDE_DIR}
	)
	target_link_libraries(etl
		${OS_LIBRARIES}
                ${SDL32_LIBRARIES}
		#SDLmain - Windows?
		${OPENGL_LIBRARY}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
	)
	set_target_properties(etl
                PROPERTIES COMPILE_DEFINITIONS "USE_ICON")
        install(TARGETS etl DESTINATION build/linux-i386)
endif(BUILD_CLIENT)

if(BUILD_SERVER)
	add_executable(etlded ${COMMON_SRC} ${SERVER_SRC})
	target_link_libraries(etlded
		${OS_LIBRARIES})
	set_target_properties(etlded
		PROPERTIES COMPILE_DEFINITIONS "DEDICATED")
	install(TARGETS etlded DESTINATION build/linux-i386)
endif(BUILD_SERVER)

if(BUILD_MOD)
	add_library(cgame.mp.${ARCH} SHARED ${CGAME_SRC})
	set_target_properties(cgame.mp.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL"
			PREFIX ""
			LIBRARY_OUTPUT_DIRECTORY "etmain")
	add_library(qagame.mp.${ARCH} SHARED ${QAGAME_SRC})
	set_target_properties(qagame.mp.${ARCH}
		PROPERTIES COMPILE_DEFINITIONS "GAMEDLL"
			PREFIX ""
			LIBRARY_OUTPUT_DIRECTORY "etmain")
	add_library(ui.mp.${ARCH} SHARED ${UI_SRC})
	set_target_properties(ui.mp.${ARCH}
		PROPERTIES
			PREFIX ""
			LIBRARY_OUTPUT_DIRECTORY "etmain")
endif(BUILD_MOD)