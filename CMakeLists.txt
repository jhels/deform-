cmake_minimum_required(VERSION 3.19)
project(Min-Externals)

string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")

if (APPLE)
    if (${CMAKE_GENERATOR} MATCHES "Xcode")
            if (${XCODE_VERSION} VERSION_LESS 10)
                message(STATUS "Xcode 10 or higher is required. Please install from the Mac App Store.")
                return ()
            elseif(${XCODE_VERSION} VERSION_GREATER_EQUAL 12)
                set(C74_BUILD_FAT YES)
            endif ()
    endif ()

    if (NOT CMAKE_OSX_ARCHITECTURES)
        if(C74_BUILD_FAT)
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS architecture" FORCE)
        else()
            set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "macOS architecture" FORCE)
        endif()
        message("CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif()
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/min-devkit)

# # Misc setup and subroutines
# include(${CMAKE_CURRENT_SOURCE_DIR}/min-devkit/source/min-api/script/min-package.cmake)


# # Add the Lib, if it exists
# if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/min-devkit/source/min-lib/CMakeLists.txt")
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/min-devkit/source/min-lib)
# endif ()

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
foreach (project_dir ${PROJECT_DIRS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir}/CMakeLists.txt")
        message("Generating: ${project_dir}")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/${project_dir})
    endif ()
endforeach ()
