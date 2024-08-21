cmake_minimum_required(VERSION 3.14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/CMake/cxx_flag_overrides.cmake)

project(agsappopenurl
		VERSION 0.2.0
		DESCRIPTION "AGS Plugin to open a URL in the system browser."
		LANGUAGES CXX
		)

set(CMAKE_CXX_STANDARD 11)

if(${CMAKE_BUILD_TYPE} MATCHES Release)
	message("CMAKE_CXX_FLAGS_RELEASE_INIT: ${CMAKE_CXX_FLAGS_RELEASE_INIT}")
elseif(${CMAKE_BUILD_TYPE} MATCHES Debug)
	message("CMAKE_CXX_FLAGS_DEBUG_INIT: ${CMAKE_CXX_FLAGS_DEBUG_INIT}")
endif()

if (WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "Windows builds only support 32bit for now")
endif()

include_directories(.)

if (APPLE)
	# macOS specific settings
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)

	if (NOT COREFOUNDATION_LIBRARY)
		message(FATAL_ERROR "CoreFoundation library not found")
	endif()

	if (NOT APPLICATIONSERVICES_LIBRARY)
		message(FATAL_ERROR "ApplicationServices library not found")
	endif()

	message("CoreFoundation_LIBRARY: ${COREFOUNDATION_LIBRARY}")
	message("ApplicationServices_LIBRARY: ${APPLICATIONSERVICES_LIBRARY}")
else ()
	find_package(SDL2 REQUIRED)

	message("SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
	message("SDL2_LIBRARIES: ${SDL2_LIBRARIES}")

	# fix issue in cmake 3.16.4 in Ubuntu 16.04
	string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
endif()

add_library(agsappopenurl SHARED
        agsappopenurl.cpp
        agsappopenurl.h 
        )

target_include_directories(agsappopenurl PRIVATE ../ags/Engine/)
target_include_directories(agsappopenurl PRIVATE ../ags/Common/)

if (APPLE)
	target_link_libraries(agsappopenurl PRIVATE
			${COREFOUNDATION_LIBRARY}
			${APPLICATIONSERVICES_LIBRARY}
			)
else ()
	target_include_directories(agsappopenurl PRIVATE ${SDL2_INCLUDE_DIRS})
	target_link_libraries(agsappopenurl PRIVATE ${SDL2_LIBRARIES})
endif ()


install(TARGETS agsappopenurl RUNTIME DESTINATION lib)