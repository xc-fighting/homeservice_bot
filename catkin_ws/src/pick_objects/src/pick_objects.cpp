#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

// Define a client for to send goal requests to the move_base server through a SimpleActionClient
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv){
  // Initialize the simple_navigation_goals node
  ros::init(argc, argv, "pick_objects");


  ros::NodeHandle handle;
  ros::Publisher goalPublisher = handle.advertise<geometry_msgs::Pose>("/target",15);

  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);

  // Wait 5 sec for move_base action server to come up
  while(!ac.waitForServer(ros::Duration(5.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }

  move_base_msgs::MoveBaseGoal goal;

  // set up the frame parameters
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();

  // Define a position and orientation for the robot to reach
  goal.target_pose.pose.position.x = 4.0;
  goal.target_pose.pose.position.y = 4.0;
  goal.target_pose.pose.orientation.w = 1.0;

   // Send the goal position and orientation for the robot to reach
  ROS_INFO("Sending goal");
  ac.sendGoal(goal);

  // Wait an infinite time for the results
  ac.waitForResult();

  // Check if the robot reached its goal
  if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
  {
     ROS_INFO("get to the pick up zone");
     ROS_INFO("sleep for 5 second and go to next");
     ros::Duration(5.0).sleep();
  }
  else
  {
      ROS_INFO("fail to get to pick up zone");
  }

  move_base_msgs::MoveBaseGoal next;
  // set up the frame parameters
  next.target_pose.header.frame_id = "map";
  next.target_pose.header.stamp = ros::Time::now();

  // Define a position and orientation for the robot to reach
  next.target_pose.pose.position.x = 0.0;
  next.target_pose.pose.position.y = 0.0;
  next.target_pose.pose.orientation.w = 1.0;

  ac.sendGoal(next);
  // Wait an infinite time for the results
  ac.waitForResult();

  // Check if the robot reached its goal
  if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
  {
     ROS_INFO("get to the drop off zone");
     ROS_INFO("sleep for 5 second and go to next");
     ros::Duration(5.0).sleep();
  }
  else
  {
      ROS_INFO("fail to get to drop off zone");
  }
    

  return 0;
}
