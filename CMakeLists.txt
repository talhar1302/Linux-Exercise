cmake_minimum_required(VERSION 3.0)
project(linuxEx2 VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
find_library(libzip zip)

set(SOURCE_FILES stocks.cpp)

add_executable(${PROJECT_NAME} stocks.cpp src/csvExport.cpp include/csvExport.h get_stocks_data.sh src/stockData.cpp include/stockData.h include/utils.h src/utils.cpp ./get_stocks_data.sh)
target_link_libraries(${PROJECT_NAME} PUBLIC "${libzip}")
target_link_libraries(${PROJECT_NAME} PUBLIC "${json-c}")
target_include_directories(${PROJECT_NAME} PUBLIC)