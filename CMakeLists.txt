cmake_minimum_required(VERSION 3.8)
project(motor)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_library(GPIOD_LIBRARY gpiod)
find_package(robot_interfaces REQUIRED)  

add_executable(motor src/driver.cpp)
add_executable(lines src/multi_line.cpp)


ament_target_dependencies(motor rclcpp std_msgs robot_interfaces)  #для сообщений ${msg_files}
ament_target_dependencies(lines)

target_link_libraries(${PROJECT_NAME} "${cpp_typesupport_target}" ${GPIOD_LIBRARY})
target_link_libraries(lines "${cpp_typesupport_target}" ${GPIOD_LIBRARY})


#ament_export_dependencies(rosidl_default_runtime)

install(TARGETS
  motor
  lines
  DESTINATION lib/${PROJECT_NAME})


ament_package()
