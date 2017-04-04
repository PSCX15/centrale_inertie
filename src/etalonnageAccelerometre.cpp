
#include <string>
#include <vector>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <ros/duration.h>
#include <sstream>

#include "centrale_inertie/CI_msg.h"
#include "std_msgs/Bool.h"

using std::vector;



vector<float> vecX;
vector<float> vecY;
vector<float> vecZ;


int count = 0;

vector<ros::Rate> rate;
float duree;


ros::Time begin;
ros::Time end;


float xMoy;
float yMoy;
float zMoy;

float xV;
float yV;
float zV;

volatile bool stop;

void Callback(const centrale_inertie::CI_msg::ConstPtr& msg)
{

	if(!stop){
	end = ros::Time::now();
	
	duree = (end - begin).toSec() ; 
	
	begin = ros::Time::now();
	
	float X = msg->x;
	float Y = msg->y;
	float Z = msg->z;
	

	vecX.push_back(X);
	vecY.push_back(Y);
	vecZ.push_back(Z);
	
	
	
	
	//ROS_INFO("%f",duree);
	//ROS_INFO("Moyenne acc : x=%f ; y=%f ; z=%f",vecAcc[0],vecAcc[1],vecAcc[2]);
	
	//ROS_INFO("vitesse : x=%f ; y=%f ; z=%f",vecVitX[count],vecVitY[count],vecVitZ[count]);
	
	count++;
	}
}


int faireMoyenne(int countFinal){
	
	stop = true;
	
	float totalX = 0;
 	float totalY = 0;
 	float totalZ = 0;
	float ecartX = 0;
	float ecartY = 0;
	float ecartZ = 0;
 	int supprimer = 0;
 	
	bool fini = false;
	while(!fini){
	
		totalX = 0;
 		totalY = 0;
 		totalZ = 0;
 		fini = true;
 	
 		if(vecZ.size()==vecX.size() && vecX.size()==vecY.size()){
 			int longueur  = vecX.size();
	 		for(unsigned int i=0; i<vecX.size(); i++)
			{
	    	totalX += vecX[i];
	    	totalY += vecY[i];
	    	totalZ += vecZ[i];
	 		}
	 		totalX = totalX/longueur;
	 		totalY = totalY/longueur;
	 		totalZ = totalZ/longueur;
	 		
	 		ecartX = 0;
	 		ecartY = 0;
	 		ecartZ = 0;
	 		
	 		for(unsigned int i=0; i<vecX.size(); i++)
			{
	    	ecartX += pow(vecX[i]-totalX,2);
	    	ecartY += pow(vecY[i]-totalY,2);
	    	ecartZ += pow(vecZ[i]-totalZ,2);
	 		} 
	 		
	 		ecartX /= longueur;
	 		ecartY /= longueur;
	 		ecartZ /= longueur;
	 		
	 		ecartX = pow(ecartX,0.5);
	 		ecartY = pow(ecartY,0.5);
	 		ecartZ = pow(ecartZ,0.5);
	 		
	 		vector<float> nvecX = *(new vector<float>());
	 		
	 		vector<float> nvecY = *(new vector<float>());
	 		
	 		vector<float> nvecZ = *(new vector<float>());
	 		
	 		
	 		
	 		for(unsigned int i=0; i<vecX.size(); i++)
			{
				if(abs(vecX[i]-totalX)>4*ecartX || abs(vecY[i]-totalY)>4*ecartY || abs(vecZ[i]-		totalZ)>4*ecartZ){
					fini = false;
				}
				else{
					nvecX.push_back(vecX[i]);
					nvecY.push_back(vecY[i]);
					nvecZ.push_back(vecZ[i]);
				}
			}
			
			vecX=nvecX;
			vecY=nvecY;
			vecZ=nvecZ;
			
			supprimer = nvecX.size();
		}
		else{
			
		
		} 		
	}
	
	xV = ecartX*ecartX;
	yV = ecartY*ecartY;
	zV = ecartZ*ecartZ;
	
	xMoy = totalX;
	yMoy = totalY;
	zMoy = totalZ;
	
	return(countFinal-supprimer);
	
}


int main(int argc, char **argv)
{	
	//vecX = new vector<float>();
	//vecY = new vector<float>();
	//vecZ = new vector<float>();
	stop = false;
	
  ros::init(argc, argv, "EtallonageAcc");
  
  ros::NodeHandle n;
  
  ros::Rate loop_rate(10);
  rate.push_back(loop_rate);
  
	ros::Subscriber sub_gyr = n.subscribe("/CI/Acc", 1000, Callback);
	
	ros::Publisher pub_mag = n.advertise<std_msgs::Bool>("/Correction/Changed",1000);
	
	ROS_INFO("Le robot doit rester immobile parfaitement!! Vous avez 20 secondes");
	ros::Duration duree;
	
	ros::Time debut = ros::Time::now();
	while(ros::ok()){
		
		ros::spinOnce();
		
		
		if(ros::Time::now()>debut+ros::Duration(20.0)){
			
			stop = true;
			
			int countFinal = count;
			int deduis = faireMoyenne(countFinal);
			
			ROS_INFO("Moyenne faite sur %i valeurs en excluant %i valeurs absurdes",countFinal,deduis);
		
			float XCorrection;
			if(!n.getParam("/Correction/Acc/x",XCorrection)){
				XCorrection = 0;
			}
			n.setParam("/Correction/Acc/x",XCorrection+xMoy);
			
			float YCorrection;
			if(!n.getParam("/Correction/Acc/y",YCorrection)){
				YCorrection = 0;
			}
			n.setParam("/Correction/Acc/y",YCorrection+yMoy);
			
			float ZCorrection;
			if(!n.getParam("/Correction/Acc/z",ZCorrection)){
				ZCorrection = 0;
			}
			n.setParam("/Correction/Acc/z",ZCorrection+zMoy);
			
			n.setParam("/Correction/Acc/xV",xV);
			n.setParam("/Correction/Acc/yV",yV);
			n.setParam("/Correction/Acc/zV",zV);
			
			
			std_msgs::Bool msg;
			msg.data = true;
			pub_mag.publish(msg);
			
			ros::Duration(2.0).sleep();
			
			ros::shutdown();
		}
		
	}

  return 0;

}


