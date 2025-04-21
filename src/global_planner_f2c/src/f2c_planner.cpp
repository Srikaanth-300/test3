#include <ros/ros.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <costmap_2d/costmap_2d.h>
#include <nav_core/base_global_planner.h>
#include <geometry_msgs/PoseStamped.h>
#include <angles/angles.h>
#include <base_local_planner/world_model.h>
#include <base_local_planner/costmap_model.h>
#include <global_planner_f2c/f2c_planner.h>
#include <nav_msgs/Path.h>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <pluginlib/class_list_macros.h>

using std::string;

namespace F2C_planner {

 GlobalPlanner::GlobalPlanner() {}
 void GlobalPlanner::initialize(std::string name, costmap_2d::Costmap2DROS* costmap_ros){
    ros::NodeHandle nh;
    global_plan_pub = nh.advertise<nav_msgs::Path>("global_plan", 10);
    a_star_check = nh.advertise<nav_msgs::Path>("a_star_plan", 10);
    geometry_msgs::PoseStamped robot_pose;
    costmap_ = costmap_ros;
    std::string csv_file_path = "/home/srikaanth/harvtech_ws/src/navigation/csv_files/circle.csv";
    std::ifstream file(csv_file_path);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        double x, y, z = 0.0;
        if (std::getline(ss, value, ' ')) x = std::stod(value);
        if (std::getline(ss, value, ' ')) y = std::stod(value);
        if (std::getline(ss, value, ' ')) z = std::stod(value); 
        geometry_msgs::PoseStamped pose;
        pose.header.stamp = ros::Time::now();
        pose.header.frame_id = "map";
        pose.pose.position.x = x;
        pose.pose.position.y = y;
        pose.pose.position.z = z;
        pose.pose.orientation.w = 1.0;
        global_plan.push_back(pose);
        //ROS_INFO("x: %f ,y: %f, z: %f", x, y, z);
    }
    file.close();

    // // Read the interrupt_file (this can be used further to read data from a file where the last point was reached before the system being shut)
    // std::string interrupt_file_path = "/home/srikaanth/bison_ws/src/Fields2Cover/interrupt_coordinates.txt";
    // std::ifstream file2(interrupt_file_path);
    // if (std::getline(file2, line) && line == "true"){
    //     if(std::getline(file2, line)){
    //         std::stringstream ss1(line);
    //         std::string goals;
    //         if (std::getline(ss1, goals, ',')) interrupt_x = std::stod(goals);
    //         if (std::getline(ss1, goals, ',')) interrupt_y = std::stod(goals);
    //         ROS_INFO("The read files have values %f, %f ", interrupt_x, interrupt_y);
    //         // To be used to check where is the start point of the robot
    //         interrupted = true;
    //         //std::cout << "The interrupt variable has the value: " << interrupted << std::endl;
    //     }
    // }
    // file2.close();

    nav_msgs::Path path_msg;
    path_msg.header.stamp = ros::Time::now();
    path_msg.header.frame_id = "map";
    path_msg.poses = global_plan;
    
    global_plan_pub.publish(path_msg);

    ROS_INFO("Number of waypoints in the plan: %lu", global_plan.size());
    

    ROS_INFO("F2C_planner plugin has been initiated.");

 }
 bool GlobalPlanner::makePlan(const geometry_msgs::PoseStamped& start, const geometry_msgs::PoseStamped& goal, std::vector<geometry_msgs::PoseStamped>& plan){
    plan = global_plan;
    if (!plan.empty()){
        geometry_msgs::PoseStamped robot_pose;
        if (costmap_->getRobotPose(robot_pose)) {
            ROS_INFO("Robot pose: x = %f, y = %f", robot_pose.pose.position.x, robot_pose.pose.position.y);
            geometry_msgs::PoseStamped first_waypoint;
            first_waypoint = plan[0];
            // The below part is for later use as mentioned above.
            // //std::cout << "The interrupt variable inside the make_plan has the value: " << interrupted << std::endl;
            // if (interrupted){
            //     //geometry_msgs::PoseStamped first_waypoint;
            //     first_waypoint.header.stamp = ros::Time::now();
            //     first_waypoint.header.frame_id = "map";
            //     first_waypoint.pose.position.x = interrupt_x;
            //     first_waypoint.pose.position.y = interrupt_y;
            //     first_waypoint.pose.position.z = 0.0;
            //     first_waypoint.pose.orientation.w = 1.0;
            //     ROS_INFO("The first wapoints are x: %f, y: %f", interrupt_x, interrupt_y);
            // }
            // else{
            //    // geometry_msgs::PoseStamped
            //    // ROS_INFO("The first part of the plan has points x: %f, y: %f", plan[0].pose.position.x, plan[0].pose.position.y);
            //     first_waypoint = plan[0];
            //    // ROS_INFO("The first wapoints are x: %f, y: %f", first_waypoint.pose.position.x, first_waypoint.pose.position.y);
            // }
            unsigned int mx, my;
            if (costmap_->getCostmap()->worldToMap(robot_pose.pose.position.x, robot_pose.pose.position.y, mx, my)){     // convert coordinates of start from global to cells
                ROS_INFO("The grid values of the start pose's are : %u, %u", mx, my);
                std::vector<Node> a_star_path = runAStar(mx, my,first_waypoint.pose.position.x, first_waypoint.pose.position.y);
                // Convert A* path to PoseStamped and add to plan
                std::vector<geometry_msgs::PoseStamped> a_star_plan;
                for (const auto& node : a_star_path) {
                    double world_x, world_y;
                    costmap_->getCostmap()->mapToWorld(node.x, node.y, world_x, world_y);
                    geometry_msgs::PoseStamped pose;
                    pose.header.stamp = ros::Time::now();
                    pose.header.frame_id = "map";
                    pose.pose.position.x = world_x;
                    pose.pose.position.y = world_y;
                    pose.pose.orientation.w = 1.0;  // Assume neutral orientation
                    a_star_plan.push_back(pose);
                }
                a_star_plan.insert(a_star_plan.end(), global_plan.begin(), global_plan.end());
                plan = a_star_plan;
                nav_msgs::Path path_msg;
                path_msg.header.stamp = ros::Time::now();
                path_msg.header.frame_id = "map";
                path_msg.poses = plan;
                global_plan_pub.publish(path_msg);
            }
            else{
                ROS_INFO("The coordinates are of starting point is of bound");
            }
        } 
        else {
            ROS_WARN("Could not retrieve robot pose.");
        }
    }
    else {
    ROS_WARN("Plan vector is empty!");}
    return true;    
    }
 
 // A* Pathfinding function
 std::vector<Node> GlobalPlanner::runAStar(unsigned int start_x, unsigned int start_y, double goal_x, double goal_y) {
    
    //using NodePtr = std::shared_ptr<Node>;  // Use shared pointers for safety
    std::priority_queue<NodePtr, std::vector<NodePtr>, CompareNode> openSet;
    std::vector<std::vector<bool>> closed_list(costmap_->getCostmap()->getSizeInCellsX(), std::vector<bool>(costmap_->getCostmap()->getSizeInCellsY(), false));
    
    std::unordered_map<std::pair<unsigned int, unsigned int>, NodePtr, Hash> nodeLookup; // Stores the nodes 

    std::unordered_map<unsigned int, std::unordered_map<unsigned int, double>> g_cost; // Stores the minimum cost to reach a node
    
    nav_msgs::Path nodes_path;
    nodes_path.header.stamp = ros::Time::now();
    nodes_path.header.frame_id = "map";
    
    unsigned int mx, my;

    if (costmap_->getCostmap()->worldToMap(goal_x, goal_y, mx, my)){   // Transform goal coordinates into cells
        ROS_INFO("The grid values of the goal pose's are : %u, %u", mx, my);
        ROS_INFO("The grid values of the start pose's are : %u, %u", start_x, start_y);
        ROS_INFO("Costmap size: %u x %u", costmap_->getCostmap()->getSizeInCellsX(), costmap_->getCostmap()->getSizeInCellsY());
        /*if (!isValid(start_x, start_y)) {
            ROS_WARN("Start position (%f, %f) is inside an obstacle or outside the map!", start_x, start_y);
        }
        if (!isValid(mx, my)) {
            ROS_WARN("Goal position (%f, %f) is inside an obstacle or outside the map!", mx, my);  //Commented out because the initial conditions are already handled 
        }*/

        /*std::unordered_map<std::string, Node> closed_list;
        std::unordered_map<int, Node> visited;*/
        NodePtr start = std::make_shared<Node>();
        start->x = start_x;
        start->y = start_y;
        start->cost = 0;
        start->heuristic = heuristic(start_x, start_y, mx, my);
        start->parent = nullptr;

        openSet.push(start);

        nodeLookup[{start_x, start_y}] = start;

        g_cost[start_x][start_y] = 0;

        //ROS_INFO("Initial openSet size: %lu", openSet.size());
        ROS_INFO("Initial openSet start values: %u, %u, %f, %f, %f ", start->x, start->y, start->cost, start->heuristic, start->parent);

        std::vector<std::pair<double, double>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        //visited[start_x * 1000 + start_y] = start;

        while (!openSet.empty()) {
            NodePtr current = openSet.top();
            //ROS_INFO("Initial openSet size: %lu", openSet.size());
            openSet.pop();
            //ROS_INFO("Initial openSet size: %lu", openSet.size());
            ros::Rate rate(0.5);
            //ROS_INFO("The chosen one with lowest cost is x: %u, y: %u, cost: %f", current->x, current->y, current->cost);
            // If goal is reached, reconstruct path

            //ROS_INFO("The current value being processed is x:% u, y: %u", current->x, current->y);
            // Visualization for A* 
            geometry_msgs::PoseStamped node_pose;
            double viz_x, viz_y;
            /*costmap_->getCostmap()->mapToWorld(current->x, current->y, viz_x, viz_y);
            node_pose.header.stamp = ros::Time::now();
            node_pose.header.frame_id = "map";
            node_pose.pose.position.x = viz_x;
            node_pose.pose.position.y = viz_y;
            node_pose.pose.orientation.w = 1.0;

            nodes_path.poses.push_back(node_pose);
            a_star_check.publish(nodes_path);*/

            if (current->x == mx && current->y == my) {
                std::vector<Node> path;
                NodePtr temp = current;
                while (temp) {
                    path.push_back(*temp);
                    temp = temp->parent;
                }
                std::reverse(path.begin(), path.end());  // Reverse the path
                ROS_INFO("Reached the target");
                bool reached_start_point = true;
                return path;
            }

            closed_list[current->x][current->y] = true;

            unsigned int local_x, local_y; 
            //costmap_->getCostmap()->worldToMap(current.x, current.y, local_x, local_y);
            //ROS_INFO("%u, %u", current->x, current->y);
            for (const auto& dir : directions) {
                unsigned int new_x = current->x + dir.first;
                unsigned int new_y = current->y + dir.second;

                //ROS_INFO("The first neighbor is %u, %u", new_x, new_y);
                if (!isValid(new_x, new_y) || closed_list[new_x][new_y]){
                    //ROS_INFO("Skipping this point because its invalid or alreaady visited");
                    continue;
                }
                double new_cost = current->cost + hypot(dir.first, dir.second);
                //Node neighbor = {new_x, new_y, new_cost, heuristic(new_x, new_y, mx, my), current};

                /*if (isValid(new_x,new_y))
                 closed_list[new_x][new_y] = true; // Mark as visited
                else{
                    ROS_INFO("Skipping this point because its invalid");
                    continue;
                }*/
                // If this path to the neighbor is better, update it
            
                if (g_cost.find(new_x) == g_cost.end() || g_cost[new_x].find(new_y) == g_cost[new_x].end() || new_cost < g_cost[new_x][new_y]) {
                    //ROS_INFO("The present cost is %f, but the cost to this other path is %u", new_cost, g_cost[new_x][new_y]);
                    g_cost[new_x][new_y] = new_cost;

                    if (nodeLookup.find({new_x, new_y}) != nodeLookup.end()) {
                    // Node exists: update cost and parent
                    NodePtr existingNode = nodeLookup[{new_x, new_y}];
                    existingNode->cost = new_cost;
                    existingNode->parent = current;
                    openSet.push(existingNode);
                    //ROS_INFO("Modifying existing path");
                    }
                    else{
                    NodePtr neighbor = std::make_shared<Node>();
                    neighbor->x = new_x;
                    neighbor->y = new_y;
                    neighbor->cost = new_cost;
                    neighbor->heuristic = heuristic(new_x, new_y, mx, my);
                    neighbor->parent = current;
                    openSet.push(neighbor);
                   //ROS_INFO("The neighbors being pushed has the f cost: %f , and the point is x: %u, y: %u", (neighbor->heuristic + new_cost), new_x, new_y);
                    }
                    //ROS_INFO("This is the new coordinate x: %u, y: %u, cost: %f , heuristic: %f", new_x, new_y, new_cost, neighbor->heuristic);
                   // ROS_INFO("The size of openset is %lu inside the if loop", openSet.size());



                   // Visualization for A* 
                    costmap_->getCostmap()->mapToWorld(new_x, new_y, viz_x, viz_y);
                    node_pose.header.stamp = ros::Time::now();
                    node_pose.header.frame_id = "map";
                    node_pose.pose.position.x = viz_x;
                    node_pose.pose.position.y = viz_y;
                    node_pose.pose.orientation.w = 1.0;
        
                    nodes_path.poses.push_back(node_pose);
                    a_star_check.publish(nodes_path);
                }
                //rate.sleep();

                //openSet.push(neighbor);
                //ROS_INFO("The size of openset is %lu inside the for loop", openSet.size());
            }


            /*// Explore neighbors
            for (int i = 0; i < 8; ++i) {
                int nx = current.x + dx[i];
                int ny = current.y + dy[i];

                double new_cost = current.cost + 1.0;  // Assuming equal movement cost for now
                int hash = nx * 1000 + ny;

                // If this node has been visited with a lower cost, skip it
                if (visited.find(hash) != visited.end() && visited[hash].cost <= new_cost) {
                    continue;
                }

                Node neighbor;
                neighbor.x = nx;
                neighbor.y = ny;
                neighbor.cost = new_cost;
                neighbor.heuristic = heuristic(nx, ny, goal_x, goal_y);
                neighbor.parent = std::make_shared<Node>(current);  // Set the parent

                openSet.push(neighbor);
                visited[hash] = neighbor;
            }*/
        }
        /*    // Initialize start node
        Node* start = new Node(start_x, start_y, 0, heuristic(start_x, start_y, goal_x, goal_y));
        openSet.push(*start);
        visited[start_x * 1000 + start_y] = start;
            while (!openSet.empty()) {
            Node current = openSet.top();
            openSet.pop();
                // If goal is reached, reconstruct path
            if (current.x == goal_x && current.y == goal_y) {
                std::vector<Node> path;
                Node* temp = &current;
                while (temp) {
                    path.push_back(*temp);
                    temp = temp->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
                // Expand neighbors
            for (int i = 0; i < 8; i++) {
                int nx = current.x + dx[i];
                int ny = current.y + dy[i];
                    // Assume the cost of moving is 1 for now
                double new_cost = current.cost + 1.0;
                    // If the node has been visited with a lower cost, skip it
                int hash = nx * 1000 + ny;
                if (visited.find(hash) != visited.end() && visited[hash]->cost <= new_cost) {
                    continue;
                }
                    Node* neighbor = new Node(nx, ny, new_cost, heuristic(nx, ny, goal_x, goal_y), new Node(current));
                openSet.push(*neighbor);
                visited[hash] = neighbor;
            }
        }*/
            // If no path is found, return an empty vector
        ROS_WARN("A* search failed to find a path.");
    }
    else{
        ROS_INFO("The goal point is out of bounds");
    }
    return {};
 }
 double GlobalPlanner::heuristic(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
 }
 bool GlobalPlanner::isValid(unsigned int x, unsigned int y) {

    unsigned char cost = costmap_->getCostmap()->getCost(x, y);
    return cost < costmap_2d::INSCRIBED_INFLATED_OBSTACLE;  // Avoid occupied areas
 }


}

PLUGINLIB_EXPORT_CLASS(F2C_planner::GlobalPlanner, nav_core::BaseGlobalPlanner)
