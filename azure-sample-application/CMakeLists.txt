# Leveraged CMakeLists.txt files from IoT Hub Client Samples by Microsoft

Option(edison "set edison to ON to build the DM edison sample" OFF)

cmake_minimum_required(VERSION 2.8.11)

project(azure_app)

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    if (NOT IN_OPENWRT)
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

compileAsC99()

LIST(GET CMAKE_FIND_ROOT_PATH 0 SYSROOT_PATH)

set(AZUREIOT_INC_FOLDER ".")

include_directories(${AZUREIOT_INC_FOLDER})
include_directories(${SYSROOT_PATH}/usr/include/azureiot)

set(azure_app_c_files
    azure_app.c
    azure_driver.c
)
set(azure_app_h_files
    azure_driver.h
)

add_executable(azure_app ${azure_app_c_files} ${azure_app_h_files})

target_link_libraries(azure_app
    iothub_client
    iothub_client_mqtt_transport
    serializer
    aziotsharedutil
    umqtt
    pthread
    curl
    m
    uamqp
    uhttp
    prov_auth_client
    hsm_security_client
    parson
    ssl
    crypto
    uuid
)