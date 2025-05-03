#######**Description:**

The package has the necessary codes to spawn up a turtlebot in the simulation and cover a given field. Other packages and libraries like Fields2Cover, GracefulController, Ublox_driver (for ROS), turtlebot (for robot model) were used. Another controller package (Regulater_pure_pursuit_controller) has been tried out.

######**Steps to launch the system:**

Launch the gazebo simulation with the turtlebot3 (burger model) spawned up in an empty world.

`roslaunch turtlebot3_gazebo turtlebot3_empty_world.launch`

Then in a new terminal launch the navigation stack which launches the localization node as well as a RViz display window.

`roslaunch navigation complete_launch.launch`

This would load up the saved map and the plan config files. 20 seconds (buffer time) after launching the navigation stack (actual time and not ros time) the system would start to follow the plan generated.  

#####**File tree:**

**Robot description:**

The robot's description files are in `turtlebot3 -> turtlebot3_description -> urdf`. Additional link and a joint was added in the _turtlebot3_burger.urdf.xacro_ urdf to simulate GPS sensor values. GPS plugin for gazebo was added in the file _turtlebot3_burger.gazebo.xacro_ with the latitude, longitude, update_rate, link_names specified.

**Localization:**

The localization of the system is done only using GPS and IMU. Robot_localization package was used to fuse data from GPS and IMU to get odometry message. _X,Y_ from GPS and _Yaw_ angle from IMU is fused together. Two nodes are run -navsat_transform_node and ekf node. To keep the system in accordance with _map->odom->base_footprint_ format, a dummy _odom->base_footprint_ is published. The launch file for localization is `_world_coordinates.launch_ (navigation/launch)`. And the parameter file to edit the parameters for the localization is `_gps.yaml_ (navigation/config)`.

**Ublox_driver:**

GPS data from ublox devices are sent to the main PC as a topic. The nodes to run are in the _ublox_ package. To launch the nodes after confirming the fix status in the rover board (through leds), run  `roslaunch ublox_gps ublox_device.launch`. Ensure that appropriate params are set in the `_zed_f9p.yaml_` file. With the existing params we get position,heading as a topic.

**Note:** Proper remapping of topics is required while using the actual GPS, IMU hardware for localization.
