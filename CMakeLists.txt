cmake_minimum_required(VERSION 2.8.3)

project(fast_minsnap)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -fPIC")

find_package(Eigen3 REQUIRED)

find_package(catkin REQUIRED COMPONENTS
  roscpp
)

include_directories(
    ${catkin_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
    include
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES image_receiver
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

add_executable(example1_node src/example1_node.cpp)

target_link_libraries(example1_node
  ${catkin_LIBRARIES}
)

add_executable(traj_node src/TrajGen_fast.cpp)

target_link_libraries(traj_node
  ${catkin_LIBRARIES}
)
