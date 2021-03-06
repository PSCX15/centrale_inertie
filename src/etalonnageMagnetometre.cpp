
#include <string>
#include <vector>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <ros/duration.h>
#include <sstream>

#include "centrale_inertie/CI_msg.h"
#include "std_msgs/Bool.h"

using std::vector;



vector<float> vecMagX;
vector<float> vecMagY;
vector<float> vecMagZ;


int count = 0;

vector<ros::Rate> rate;
float duree;


ros::Time begin;
ros::Time end;


float xMax;
float xMin;
float yMax;
float yMin;
float zMax;
float zMin;



void MagCallback(const centrale_inertie::CI_msg::ConstPtr& msg)
{
	end = ros::Time::now();
	
	duree = (end - begin).toSec() ; 
	
	begin = ros::Time::now();
	
	float X = msg->x;
	float Y = msg->y;
	float Z = msg->z;

	vecMagX.push_back(X);
	vecMagY.push_back(Y);
	vecMagZ.push_back(Z);
	
	if(count==0){
		xMax=X;
		xMin=X;
		
		yMax=Y;
		yMin=Y;
		
		zMax=Z;
		zMin=Z;
	}
	
	else{
		if(xMax<X)xMax=X;
		if(xMin>X)xMin=X;
		if(yMax<Y)yMax=Y;
		if(yMin>Y)yMin=Y;
		if(zMax<Z)zMax=Z;
		if(zMin>Z)zMin=Z;
	}
	
	

	//ROS_INFO("Moyenne acc : x=%f ; y=%f ; z=%f",vecAcc[0],vecAcc[1],vecAcc[2]);
	
	//ROS_INFO("vitesse : x=%f ; y=%f ; z=%f",vecVitX[count],vecVitY[count],vecVitZ[count]);
	
	count++;
	
}



int main(int argc, char **argv)
{

  ros::init(argc, argv, "EtallonageMag");
  
  ros::NodeHandle n;
  
  ros::Rate loop_rate(10);
  rate.push_back(loop_rate);
  
	ros::Subscriber sub_mag = n.subscribe("/CI/Mag/corrected", 1000, MagCallback);
	
	ros::Publisher pub_mag = n.advertise<std_msgs::Bool>("/Correction/Changed",1000);
	
	ROS_INFO("Faire bouger le magnetometre dans tous les sens svp. Vous avez 30 secondes");
	ros::Duration duree;
	
	ros::Time debut = ros::Time::now();
	while(ros::ok()){
		
		ros::spinOnce();
		
		
		if(ros::Time::now()>debut+ros::Duration(30.0)){
			float magXCorrection;
			if(!n.getParam("/Correction/Mag/x",magXCorrection)){
				magXCorrection = 0;
			}
			n.setParam("/Correction/Mag/x",magXCorrection+(xMax+xMin)/2);
			
			float magYCorrection;
			if(!n.getParam("/Correction/Mag/y",magYCorrection)){
				magYCorrection = 0;
			}
			n.setParam("/Correction/Mag/y",magYCorrection+(yMax+yMin)/2);
			
			float magZCorrection;
			if(!n.getParam("/Correction/Mag/z",magZCorrection)){
				magZCorrection = 0;
			}
			n.setParam("/Correction/Mag/z",magZCorrection+(zMax+zMin)/2);
			
			std_msgs::Bool msg;
			msg.data = true;
			pub_mag.publish(msg);
			
			ROS_INFO("c'est bon");
			ros::Duration(2.0).sleep();
			
			ros::shutdown();
		}
		
	}

  return 0;

}


