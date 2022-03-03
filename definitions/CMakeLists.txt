cmake_minimum_required(VERSION 3.17)
project(ZsomeIpDefinitions)

add_library(ZsomeIpDefinitions
  include/zsomeip-defs/AgentDefinition.h
  include/zsomeip-defs/TopicDefinition.h
  include/zsomeip-defs/MethodDefinition.h
  src/TopicDefinition.cpp
  src/MethodDefinition.cpp)

target_include_directories(ZsomeIpDefinitions
  PRIVATE
    src
    include/zsomeip-defs
  PUBLIC
    include)

target_link_libraries(ZsomeIpDefinitions PRIVATE ZserioCppRuntime vsomeip3)