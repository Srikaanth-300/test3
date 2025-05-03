#**Description:**

The package has the necessary codes to spawn up a turtlebot in the simulation and cover a given field. Other packages and libraries like Fields2Cover, GracefulController, Ublox_driver (for ROS), turtlebot (for robot model) were used. Another controller package (Regulater_pure_pursuit_controller) has been tried out.

##**Steps to launch the system:**

Launch the gazebo simulation with the turtlebot3 (burger model) spawned up in an empty world.

`roslaunch turtlebot3_gazebo turtlebot3_empty_world.launch`

Then in a new terminal launch the navigation stack which launches the localization node as well as a RViz display window.

`roslaunch navigation complete_launch.launch`

This would load up the saved map and the plan config files. 20 seconds (buffer time) after launching the navigation stack (actual time and not ros time) the system would start to follow the plan generated.  

###**File tree:**

**Robot description:**

The robot's description files are in `turtlebot3 -> turtlebot3_description -> urdf`. Additional link and a joint was added in the _turtlebot3_burger.urdf.xacro_ urdf to simulate GPS sensor values. GPS plugin for gazebo was added in the file _turtlebot3_burger.gazebo.xacro_ with the latitude, longitude, update_rate, link_names specified.

**Localization:**

The localization of the system is done only using GPS and IMU. Robot_localization package was used to fuse data from GPS and IMU to get odometry message. _X,Y_ from GPS and _Yaw_ angle from IMU is fused together. Two nodes are run -navsat_transform_node and ekf node. To keep the system in accordance with _map->odom->base_footprint_ format, a dummy _odom->base_footprint_ is published. The launch file for localization is `_world_coordinates.launch_ (navigation/launch)`. And the parameter file to edit the parameters for the localization is `_gps.yaml_ (navigation/config)`.

**Mapping:**

Once we get odometry from GPS, the boundary points are saved as a CSV file to be used for planning. The script `**_save_xy_points.py_**` currently subscribes to _odometry/filtered_ and saves the points only when the node is killed at the current directory where the script is run. This CSV file can be used with Fields2Cover library for path generation.

The map must be saved as a pgm and a yaml file. So there are two ways to do it.
  
  1) Once the boundary points are saved as a CSV file publish it as a PoseStamped message in ROS, then subscribe to those poses, generate a map and save the map using map_server package. `_path_publisher_to_map.py_` (global_planner_f2c/scripts) publishes the points in a topic and `_map_generator.py_` (global_planner_f2c/scripts) creates an occupancy grid map with the boundaries marked as obstacles. Then save the map file using _map_server_ package with appropriate filename. (Crosscheck the map topic name).
 
  2) Create a map while getting the boundary points. This would save those boundary points as well but directly subscribes to those odometry messages and then generates the map using the same `_map_generator.py_` (global_planner_f2c/scripts) script. For this uncomment the commented lines of code in the file and change the topic accordingly. (NOTE: This is not preferred because it directly subscribes to odometry message, so ensure that the script starts only when the robot is at the  boundaries)

**Ublox_driver:**

GPS data from ublox devices are sent to the main PC as a topic. The nodes to run are in the _ublox_ package. To launch the nodes after confirming the fix status in the rover board (through leds), run  `roslaunch ublox_gps ublox_device.launch`. Ensure that appropriate params are set in the `_zed_f9p.yaml_` file. With the existing params we get position,heading as a topic.

**Note:** Proper remapping of topics is required while using the actual GPS, IMU hardware for localization.

**Navigation:**

  **Global planner:**
  
  Fields2Cover library is used as the plan generator for a given boundary field. Install the library as mentioned in the official documentation. Run the script `_plan_generator_without_map.py_` specifying the location of the boundary points saved as a CSV file (while mapping), the filename for PNG and CSV file to be saved. This would give the plan to cover the field given the headland width, robot width, tool width (tutorials can be found on the official documentation) which is saved as a CSV file. The plan points are discretized for 10 cm. 

  This CSV file is read, a path is appended to the list of plan points using A* algorithm and then published as a global_plan. This is written as a global_planner plugin to interface with move_base. Ensure `_f2c_planner.cpp_` (global_planner_f2c/src) reads the correct plan CSV file. 
