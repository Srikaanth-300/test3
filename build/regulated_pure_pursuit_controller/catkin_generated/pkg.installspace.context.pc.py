# generated from catkin/cmake/template/pkg.context.pc.in
CATKIN_PACKAGE_PREFIX = ""
PROJECT_PKG_CONFIG_INCLUDE_DIRS = "${prefix}/include".split(';') if "${prefix}/include" != "" else []
PROJECT_CATKIN_DEPENDS = "pluginlib;roscpp;base_local_planner;costmap_2d;nav_core;nav_msgs;pluginlib;ddynamic_reconfigure;tf;mbf_msgs;mbf_costmap_core".replace(';', ' ')
PKG_CONFIG_LIBRARIES_WITH_PREFIX = "-lregulated_pure_pursuit_controller".split(';') if "-lregulated_pure_pursuit_controller" != "" else []
PROJECT_NAME = "regulated_pure_pursuit_controller"
PROJECT_SPACE_DIR = "/home/srikaanth/harvtech_ws/install"
PROJECT_VERSION = "0.0.1"
