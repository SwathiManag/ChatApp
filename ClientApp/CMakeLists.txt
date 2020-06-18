cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(ChatAppClient)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17 -pthread")

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Find all executables
file(GLOB project_SRCS src/*.cpp src/*.h)

# Add project executable
add_executable(ChatAppClient ${project_SRCS})
target_link_libraries(ChatAppClient ${wxWidgets_LIBRARIES})
target_include_directories(ChatAppClient PRIVATE ${wxWidgets_INCLUDE_DIRS})
