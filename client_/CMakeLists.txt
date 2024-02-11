cmake_minimum_required(VERSION 3.5)

set(PROJ_NAME client)
project(${PROJ_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread REQUIRED)


add_executable(${PROJ_NAME}
    client.cpp
)
target_include_directories(${PROJ_NAME} PRIVATE ${Boost_INCLUDE_DIR})

target_link_libraries(
  ${PROJ_NAME} PUBLIC
    pthread # required
    ${Boost_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
)