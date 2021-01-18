#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Bool.h>


using namespace std;

class MarkerNode
{
    public:
        MarkerNode();
        ~MarkerNode();
        void init();
        void start();

    private:
        ros::NodeHandle handle;
        ros::Publisher markerPublisher;
        ros::Subscriber goalSubscriber;
        ros::Subscriber odomSubscriber;

        void odomCallBack( const nav_msgs::Odometry::ConstPtr &msg );
        void goalCallBack( const move_base_msgs::MoveBaseGoal &msg );


        visualization_msgs::Marker marker;
        geometry_msgs::Pose robotPos;
        geometry_msgs::Pose markerPos;


        bool place_marker;
        bool pick_up;
        bool drop_off;
        bool start_loop;

        bool closeToMarker( geometry_msgs::Pose robot_pose, geometry_msgs::Pose marker_pose );
};

MarkerNode::MarkerNode()
{
    ROS_INFO("Init the Marker Node");
    start_loop = true;
    place_marker = true;
    pick_up = true;
    drop_off = false;
}

void MarkerNode::init()
{
    markerPublisher = handle.advertise<visualization_msgs::Marker>( "visualization_marker", 10 );
    goalSubscriber = handle.subscribe( "/goal", 1, &MarkerNode::goalCallBack, this );
    odomSubscriber = handle.subscribe( "/odom", 10, &MarkerNode::odomCallBack, this );


    marker.header.frame_id = "/map";
    marker.header.stamp = ros::Time::now();

    marker.ns = "basic_shapes";
    marker.id = 0;

    marker.type = visualization_msgs::Marker::CUBE;

    marker.scale.x = 0.4;
    marker.scale.y = 0.4;
    marker.scale.z = 0.4;

    marker.color.r = 1.0f;
    marker.color.g = 0.0f;
    marker.color.b = 0.0f;
    marker.color.a = 1.0f;

    marker.lifetime = ros::Duration();


}

void MarkerNode::start()
{
     while( true )
     {
        marker.pose = markerPos;
       
        if( marker.pose.position.x == 0 && marker.pose.position.y == 0 )
        {
            start_loop = true;
        }
        else
        {
            start_loop = false;
        }
        
        if( ( place_marker == true ) && ( start_loop == false ) )
        {
            place_marker = false;
            marker.action = visualization_msgs::Marker::ADD;
            markerPublisher.publish( marker );
        }

        if( closeToMarker( robotPos, marker.pose ) && start_loop == false )
        {
            if( pick_up ) 
            { 
                ros::Duration(5.0).sleep();
                ROS_INFO( "Picked up" );
                marker.action = visualization_msgs::Marker::DELETE;
                markerPublisher.publish( marker );
                pick_up = false;
            } 
            if( drop_off ) 
            {
                ROS_INFO( "Dropping off" );
                marker.action = visualization_msgs::Marker::ADD;
                markerPublisher.publish( marker );
                ros::Duration(10.0).sleep();
                break;
            }
        }
        ros::spinOnce();
     }
}

MarkerNode::~MarkerNode()
{
   ROS_INFO("Release the Marker Node");
}

void MarkerNode::odomCallBack( const nav_msgs::Odometry::ConstPtr &msg )
{
    this->robotPos = msg->pose.pose;
}

void MarkerNode::goalCallBack( const move_base_msgs::MoveBaseGoal &msg )
{
    markerPos = msg.target_pose.pose;
    ROS_INFO( "goalCallBack: x=%f, y=%f, w=%f", markerPos.position.x, markerPos.position.y, markerPos.orientation.w);
    if( pick_up == false )
    {
       drop_off = true;
    }
}




bool MarkerNode::closeToMarker( geometry_msgs::Pose robot_pose, geometry_msgs::Pose marker_pose )
{
    float dx = robotPos.position.x - markerPos.position.x;
    float dy = robotPos.position.y - markerPos.position.y;
    float dist = sqrt(pow(dx, 2) + pow(dy, 2));
    float delta = abs(robotPos.orientation.w - markerPos.orientation.w);
    if( dist <= 0.3 && delta < M_PI/180 )
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

int main( int argc, char** argv )
{
    ros::init( argc, argv, "add_markers" );
    MarkerNode node;
    node.init();
    node.start();
    return 0;
}