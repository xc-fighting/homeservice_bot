#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Bool.h>
#include <vector>

using namespace std;

class AddMarkers
{
    public:
        AddMarkers();
        ~AddMarkers();

    private:
        ros::NodeHandle n;
        ros::Publisher marker_pub;
        ros::Subscriber goal_sub;
        ros::Subscriber odom_sub;
        ros::Subscriber amcl_sub;
        ros::Subscriber reached_sub;
        void odomCallBack( const nav_msgs::Odometry::ConstPtr &msg );
        void goalCallBack( const move_base_msgs::MoveBaseGoal &msg );
        void amclPoseCallBack( const geometry_msgs::PoseWithCovariance &msg );
        void reachedCallBack( const std_msgs::Bool::ConstPtr &msg ); 
        vector<vector<float>> marker_locations{{ 1.0, 3.0, 1.0 },{ 4.0, 0.0, 1.0 }};

        visualization_msgs::Marker marker;
        geometry_msgs::Pose robot_pose_;
        geometry_msgs::Pose marker_pose_;
        geometry_msgs::Pose amcl_marker_pose_;
        bool reached_goal;
        bool start = true;
        bool pick_up = true;
        bool searching = true;
        bool changed = false;
        bool atOrigin = true;

        bool closeToMarker( geometry_msgs::Pose robot_pose, geometry_msgs::Pose marker_pose );
};

AddMarkers::AddMarkers()
{

    marker_pub = n.advertise<visualization_msgs::Marker>( "visualization_marker", 10 );
    goal_sub = n.subscribe( "/goal", 1, &AddMarkers::goalCallBack, this );
    odom_sub = n.subscribe( "/odom", 10, &AddMarkers::odomCallBack, this );
    amcl_sub = n.subscribe( "amcl_pose", 10, &AddMarkers::amclPoseCallBack, this );
    reached_sub = n.subscribe( "/reached", 10, &AddMarkers::reachedCallBack, this );

    marker.header.frame_id = "map";
    marker.header.stamp = ros::Time::now();

    marker.ns = "basic_shapes";
    marker.id = 0;

    marker.type = visualization_msgs::Marker::SPHERE;

    marker.scale.x = 0.5;
    marker.scale.y = 0.5;
    marker.scale.z = 0.5;

    marker.color.r = 0.0f;
    marker.color.g = 1.0f;
    marker.color.b = 1.0f;
    marker.color.a = 1.0f;

    marker.lifetime = ros::Duration();

    while( true )
    {
        marker.pose = marker_pose_;
        if( marker.pose.position.x == 0 && marker.pose.position.y == 0 )
        {
            atOrigin = true;
        }
        else
        {
            atOrigin = false;
        }
        
        if( start && !atOrigin )
        {
            start = false;
            marker.action = visualization_msgs::Marker::ADD;
            marker_pub.publish( marker );
        }
        // cout << reached_goal << endl;
        if( closeToMarker( robot_pose_, marker.pose ) && !atOrigin )
        // if( reached_goal && !atOrigin )
        {
            if( pick_up ) { // Picked up
                sleep(5);
                ROS_INFO( "Picked up" );
                marker.action = visualization_msgs::Marker::DELETE;
                marker_pub.publish( marker );
                pick_up = false;
            } 
            if( changed ) {
                ROS_INFO( "Dropping off" );
                marker.action = visualization_msgs::Marker::ADD;
                marker_pub.publish( marker );
                sleep(10);
                break;
            }
        }
        ros::spinOnce();
    }
}

AddMarkers::~AddMarkers()
{

}

void AddMarkers::odomCallBack( const nav_msgs::Odometry::ConstPtr &msg )
{
    robot_pose_ = msg->pose.pose;
}

void AddMarkers::goalCallBack( const move_base_msgs::MoveBaseGoal &msg )
{
    marker_pose_ = msg.target_pose.pose;
    ROS_INFO( "goalCallBack: x=%f, y=%f, w=%f", marker_pose_.position.x, marker_pose_.position.y, marker_pose_.orientation.w);
    if( !pick_up )
        changed = true;
}

void AddMarkers::amclPoseCallBack( const geometry_msgs::PoseWithCovariance &msg)
{
    amcl_marker_pose_ = msg.pose;
    ROS_INFO( "At amcl callback: %f, %f, %f", amcl_marker_pose_.position.x, amcl_marker_pose_.position.y, amcl_marker_pose_.orientation.w );
}

void AddMarkers::reachedCallBack( const std_msgs::Bool::ConstPtr &msg )
{
    reached_goal = msg->data;
    ROS_INFO("reachedCallBack: %d", reached_goal);
}

bool AddMarkers::closeToMarker( geometry_msgs::Pose robot_pose, geometry_msgs::Pose marker_pose )
{
    float x_diff = robot_pose.position.x - marker_pose.position.x;
    float y_diff = robot_pose.position.y - marker_pose.position.y;
    float position_distance = sqrt( pow(x_diff, 2) + pow(y_diff, 2) );
    float angle_diff = abs(robot_pose.orientation.w - marker_pose.orientation.w);
    // ROS_INFO("robot_pose.position: %f %f %f",  robot_pose.position.x, robot_pose.position.y, robot_pose.orientation.w);
    // ROS_INFO("marker_pose.position: %f %f %f",  marker_pose.position.x, marker_pose.position.y, marker_pose.orientation.w );
    // ROS_INFO("differences: %f <= 0.3,  %f <= %f \n",  position_distance, angle_diff, M_PI/180);
    if( position_distance <= 0.3 && angle_diff < M_PI/180 )
    {
        return true;
    }
    return false;
}

int main( int argc, char** argv )
{
    ros::init( argc, argv, "add_markers" );
    AddMarkers addmarker;
    return 0;
}