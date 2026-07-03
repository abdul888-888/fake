cmake_minimum_required(VERSION 3.20)

project(FacebookQtDesktop VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(FacebookQtDesktop
    src/main.cpp
    src/Controllers/BackendService.h
    src/Controllers/BackendService.cpp
    src/Views/MainWindow.h
    src/Views/MainWindow.cpp
    src/Widgets/PostCard.h
    src/Widgets/PostCard.cpp
    src/Widgets/UserCard.h
    src/Widgets/UserCard.cpp
    src/Utils/UiHelpers.h
    src/Utils/UiHelpers.cpp
    Forms/LoginPage.ui
    Forms/SignupPage.ui
    resources.qrc
)

target_include_directories(FacebookQtDesktop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Backend
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(FacebookQtDesktop PRIVATE Qt6::Widgets)

if (WIN32)
    set_target_properties(FacebookQtDesktop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
