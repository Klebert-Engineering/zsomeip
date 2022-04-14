cmake_minimum_required(VERSION 3.17)
project(zsomeip)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ZSOMEIP_VERSION 1.0.0)

find_package(Threads REQUIRED)

if (NOT TARGET vsomeip)
  add_subdirectory(deps/vsomeip)
endif ()

if (NOT TARGET ZserioCppRuntime)
  add_subdirectory(deps/zserio/compiler/extensions/cpp/runtime/src)
endif()

set(ZSERIO_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/deps/zserio")

if (NOT TARGET zserio-cmake-helper)
  add_subdirectory(deps/zserio-cmake-helper)
endif()

add_subdirectory(definitions)

option(ZSOMEIP_USE_ZSERIO_2_5_0_SERVICE_INTERFACE ON)
if (NOT ZSOMEIP_USE_ZSERIO_2_5_0_SERVICE_INTERFACE)
  add_compile_definitions(ZSERIO_2_5_0_PRE1_SERVICE_INTERFACE)
endif()

add_library(ZsomeIpService
  client-service/ZsomeIpService.cpp
  client-service/ZsomeIpService.h)

target_include_directories(ZsomeIpService
  PUBLIC client-service
  PRIVATE ${ZSERIO_RUNTIME_DIR})

target_link_libraries(ZsomeIpService ZsomeIpDefinitions ZserioCppRuntime vsomeip3)

add_library(ZsomeIpPubsub
  publish-subscribe/ZsomeIpPubsub.cpp
  publish-subscribe/ZsomeIpPubsub.h)

target_include_directories(ZsomeIpPubsub
  PUBLIC publish-subscribe
  PRIVATE ${ZSERIO_RUNTIME_DIR})
target_link_libraries(ZsomeIpPubsub ZsomeIpDefinitions ZserioCppRuntime vsomeip3)


add_zserio_library(weather-example
  ROOT "${CMAKE_CURRENT_LIST_DIR}/weather-example"
  TOP_LEVEL_PKG weather
  ENTRY Weather.zs)

add_executable(zsomeip_service_demo
  weather-example/client-service.cpp)

target_link_libraries(zsomeip_service_demo
  weather-example
  ZsomeIpDefinitions
  ZsomeIpService
  ${CMAKE_THREAD_LIBS_INIT})

add_executable(zsomeip_pubsub_demo
  weather-example/publish-subscribe.cpp)

target_link_libraries(zsomeip_pubsub_demo
  weather-example
  ZsomeIpDefinitions
  ZsomeIpPubsub
  ${CMAKE_THREAD_LIBS_INIT})
