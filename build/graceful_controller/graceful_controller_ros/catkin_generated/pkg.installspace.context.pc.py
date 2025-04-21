# generated from catkin/cmake/template/pkg.context.pc.in
CATKIN_PACKAGE_PREFIX = ""
PROJECT_PKG_CONFIG_INCLUDE_DIRS = "${prefix}/include".split(';') if "${prefix}/include" != "" else []
PROJECT_CATKIN_DEPENDS = "base_local_planner;costmap_2d;dynamic_reconfigure;geometry_msgs;graceful_controller;nav_msgs;pluginlib;roscpp;std_msgs;tf2_geometry_msgs;tf2_ros".replace(';', ' ')
PKG_CONFIG_LIBRARIES_WITH_PREFIX = "-lgraceful_controller_ros".split(';') if "-lgraceful_controller_ros" != "" else []
PROJECT_NAME = "graceful_controller_ros"
PROJECT_SPACE_DIR = "/home/srikaanth/harvtech_ws/install"
PROJECT_VERSION = "0.4.8"
