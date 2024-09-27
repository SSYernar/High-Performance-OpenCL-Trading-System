cmake_minimum_required(VERSION 3.10)
project(TradingSystem)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include /usr/local/include /opt/homebrew/include)

# Find OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})

# Source files
add_executable(trading_system
    src/main.cpp
    src/order_book.cpp
    src/matching_engine.cpp
    src/market_data.cpp
)

target_link_libraries(trading_system ${OpenCL_LIBRARIES})
