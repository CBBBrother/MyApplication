# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.31.4)

set(BOOST_MIN_VERSION "1.66.0")
set(Boost_USE_STATIC_LIBS ON)
set(boost_headers_DIR "./boost_1_87_0/inst/lib/cmake/boost_headers-1.87.0/")
set(boost_context_DIR "./boost_1_87_0/inst/lib/cmake/boost_context-1.87.0/")
set(boost_atomic_DIR "./boost_1_87_0/inst/lib/cmake/boost_atomic-1.87.0/")
set(boost_chrono_DIR "./boost_1_87_0/inst/lib/cmake/boost_chrono-1.87.0/")
set(boost_coroutine_DIR "./boost_1_87_0/inst/lib/cmake/boost_coroutine-1.87.0/")
set(boost_thread_DIR "./boost_1_87_0/inst/lib/cmake/boost_thread-1.87.0/")
set(boost_exception_DIR "./boost_1_87_0/inst/lib/cmake/boost_exception-1.87.0/")
set(boost_system_DIR "./boost_1_87_0/inst/lib/cmake/boost_system-1.87.0/")
set(Boost_DIR "./boost_1_87_0/inst/lib/cmake/Boost-1.87.0/")

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("myapplication")

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS context coroutine REQUIRED)

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

link_directories(./boost_1_87_0/inst/lib/)
include_directories(SYSTEM ./boost_1_87_0/inst/include)

set(base_DEPS ${CMAKE_DL_LIBS})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp)

include_directories(/usr/include)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
	boost_context
	boost_coroutine
        ${base_DEPS}
        android
        log)
