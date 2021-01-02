# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/unit-test-app/components")

if(${ESP_PLATFORM} AND WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlongcalls -mtext-section-literals")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlongcalls -mtext-section-literals")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-rfid-lapcounter)