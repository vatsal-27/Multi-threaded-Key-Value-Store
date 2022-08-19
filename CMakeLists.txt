# Minimum CMake required
cmake_minimum_required(VERSION 3.5.1)

# Project
project(stringreverse)

#Here change common.cmake path acoriding to your installtion.
include(/Users/jaiminchauhan/grpc/examples/cpp/cmake/common.cmake)

# Proto file
get_filename_component(hw_proto "keyval.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(CMAKE_CXX_STANDARD 17)
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/keyval.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/keyval.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/keyval.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/keyval.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Targets (client|server)
foreach(_target
  client server)
  add_executable(${_target} "${_target}.cc"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    "interface.h"
    "interface2_new.cc")
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()