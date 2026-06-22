cmake_minimum_required(VERSION 3.24)
project(Fluent VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Fluent is built against Qt 6.11 or newer.
find_package(Qt6 6.11 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    DBus
)

qt_standard_project_setup(REQUIRES 6.11)

# Subdirectories
add_subdirectory(src)

# Tests (optional, can skip if not needed yet)
# if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
#     add_subdirectory(tests EXCLUDE_FROM_ALL)
# else()
#     add_subdirectory(tests)
# endif()
