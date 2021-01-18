#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <vector>

using namespace std;

// Define a client for to send goal requests to the move_base server through a SimpleActionClient
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv){
  // Initialize the simple_navigation_goals node
  ros::init(argc, argv, "pick_objects");


  ros::NodeHandle handle;
  ros::Publisher goalPublisher = handle.advertise<move_base_msgs::MoveBaseGoal>("/goal",15);
  

  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);

  // Wait 5 sec for move_base action server to come up
  while(!ac.waitForServer(ros::Duration(5.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }

  move_base_msgs::MoveBaseGoal goal;

  // set up the frame parameters
  goal.target_pose.header.frame_id = "/map";
  goal.target_pose.header.stamp = ros::Time::now();

  

  vector<vector<float>> targets{{2.0,2.0,1.0},{4.0,4.0,1.0}};

  for(int index=0;index<targets.size();index++)
  {
     goal.target_pose.pose.position.x = targets[index][0];
     goal.target_pose.pose.position.y = targets[index][1];
     goal.target_pose.pose.orientation.w = targets[index][2];

     goalPublisher.publish(goal);
     ROS_INFO("Sending goal");
     ac.sendGoal(goal);

     ac.waitForResult();

     if(ac.getState()==actionlib::SimpleClientGoalState::SUCCEEDED)
     {
        ROS_INFO("Goal reach");
     }
     else
     {
        ROS_INFO("Goal not reach");
     }
     ros::Duration(5.0).sleep();

  }
  
  ROS_INFO("Reach the final dst");
  return 0;
}
