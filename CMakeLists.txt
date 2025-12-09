cmake_minimum_required(VERSION 3.16)
project(Oracle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)

add_executable(Oracle
    main.cpp
    AURManager.cpp
    AURManager.h
    PasswordDialog.cpp
    PasswordDialog.h
    PackageWorker.cpp
    PackageWorker.h
)

target_link_libraries(Oracle PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)