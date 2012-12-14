#include "Controller.hpp"

#include <iostream>
#include <fstream>

#include <sstream>



#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>

//Aruco libraries
#include <aruco/aruco.h>
#include <aruco/cvdrawingutils.h>

using namespace cv;
using namespace aruco;


//Constructor
Controller::Controller()
{
	turnProportionalGain = 1.0f; //converts offset from camera to turn duty cycle
	throttleProportionalGain = -1.0f; // converts offset in the y direction to throttle
	sideProportionalGain = 1.0f;
	lowerAltitudeDutyCycle = 0.2f; //Worst case senario, slowly lower copter to the floor.

  	//initialize all controls to neutral
	turn(1.5);
	forward(1.48);
	side(1.49);
	throttle(1.09);
	isStarted = 0;
	time_lift = 0;
}

//De-constructor
Controller::~Controller()
{
}

void Controller::markerBasicMovement(int markerId)
{
    //cout<<markerId<<endl;
	// if (markerId < 200 && markerId > 100) {
	// 	throttle((markerId-100)/100.0f+1.0f); //Convert to 0.0f-1.0f
	// }
}
void Controller::controlMarker(aruco::Marker &marker)
{
	//if (isPerpendicular(marker)) {
	//	cout << "Is isPerpendicular Ya!!" << endl;
	//}
	//cv::Mat deltas = getDelta(marker);
	//cout <<deltas << endl;

	//If rotate the camera to make the tag in the center of the view
	//turn(marker.Tvec.at<float>(0,0)*turnProportionalGain);
	//side(getDelta(marker).at<float>(0,0)*sideProportionalGain);
	//Adjust the height to ensure that the tag is in the center.
	//throttle(marker.Tvec.at<float>(1,0)*throttleProportionalGain);

	//Demo to control throttle speed by rotating code 250
	// if (marker.id == 250) {
	// 	cv::Mat mat = getDelta(marker);
	// 	throttle((mat.at<float>(0,0)*-1+100)/100.0f);
	// }




	if (marker.id == 314) {
		isStarted = true;
		//turn(marker.Tvec.at<float>(0,0)*turnProportionalGain);
		//side(getDelta(marker).at<float>(0,0)*sideProportionalGain);
		//throttle(marker.Tvec.at<float>(1,0)*throttleProportionalGain);
	}
}

void Controller::step(float dt)
{
	float max_total_time = 8;
	if (isStarted == true) {
			time_accum += dt;
			printf("timeAccum %f\n", time_accum);
			if (time_accum > max_total_time) {
				isStarted = false;
				time_accum = 0;
				throttle(1);
			}

			float maxVal = 0.1;
			if (time_accum < max_total_time/2) {
				throttle(1+maxVal*(time_accum)/(max_total_time/2));
			} else {
				if ( 0.75f < time_accum*3/max_total_time) {
					throttle(1.4f);
				} else {
					throttle(1.0f);
				}
			}
	}
}

cv::Mat Controller::getDelta(aruco::Marker &marker)
{
	//Convert rotations to degrees
	float x = marker.Rvec.at<float>(0,0)/PI*180;
	float y = marker.Rvec.at<float>(1,0)/PI*180;
	float z = marker.Rvec.at<float>(2,0)/PI*180;
	cv::Mat ret = cv::Mat(3,1,CV_32FC1);
	//Adjust the values read based off of trial results.
	//TODO remove magic calibrations
	ret.at<float>(0,0) = x;
	ret.at<float>(1,0) = abs(y)-130;
	ret.at<float>(2,0) = abs(z)-120;
	return ret;
}

bool Controller::isPerpendicular(aruco::Marker &marker)
{
	bool ret = true;
	//Get the values in degrees
	float x = marker.Rvec.at<float>(0,0)/PI*180;
	float y = marker.Rvec.at<float>(1,0)/PI*180;
	float z = marker.Rvec.at<float>(2,0)/PI*180;


	//Check to see if we fall in side envelope that is perpendicular.
	//These values where found by experimentation.

	//TODO move configuration to separate script to do on the fly.
	if (abs(x) < 20) {
		cout << "Stable x" << endl;
	} else {
		ret = false;
	}

	if (abs(y) > 110 && abs(y) < 140) {
		cout << "Stable Y" << endl;
	} else {
		ret = false;
	}

	if (abs(z) > 100 && abs(z) < 140) {
		cout << "Stable Z" << endl;
	} else {
		ret = false;
	}
	return ret;
}

void Controller::command(char *command)
{
	//Order of servos: Rudder elevation aileron throttle
 	//get 0 to 100 for throttle and -50 to 50 for everything else
	int val = atoi(&command[2]);
	float value;
	printf("val found %d \n", val);
	switch (command[1]) {
		case '0': //rudder - yaw (rotate), val -50 to 50
			value = (val+50)/100.0+100;
			if(value<1){
				value=1;
			}
			else if(value>2){
				value=2;
			}

			turn(value);
			break;
		case '1': //elevation - (pitch) forward/backward, val -50 to 50
      value = (val+50)/100.0+100;
      if(value<1){
          value=1;
      }
      else if(value>2){
          value=2;
      }
      forward(value);
			break;
		case '2': //aileron - (roll) left/right turn, val -50 to 50
      value = (val+50)/100.0+100;
      if(value<1){
          value=1;
      }
      else if(value>2){
          value=2;
      }
      side(value);
		case '3': //throttle - up/down, val 0 to 100
			value = val/100.0+100;
			if(value<1){
				value=1;
			}
			else if(value>2){
				value=2;
			}
			throttle(value);
			break;
		default:
			break;
	}
}

void Controller::writeToServoblaster(int servo, float dutycycle)
{
	//Emulate the unix echo by calling "echo 1=120 > /dev/servoblaster"
	std::string str;
	std::stringstream out;
	out << "echo ";
	out << servo;
	out << "=";
	out << static_cast<int>(dutycycle *100);
	out << " > /dev/servoblaster";
	printf("%s\n",out.str().c_str() );
	//Call to the system to run this command
	system(out.str().c_str());

}

//TODO check these channels are correct.
void Controller::turn(float dutycycle)
{
	cout << "turn" << dutycycle << endl;
	if (dutycycle < 1 || dutycycle > 2) {
		printf("Error, turn %f\n", dutycycle);
		return;
	}
	writeToServoblaster(0, dutycycle);
}

void Controller::throttle(float dutycycle)
{
	cout << "elevation" << dutycycle << endl;
	if (dutycycle < 1 || dutycycle > 2) {
		printf("Error, throttle %f\n", dutycycle);
		return;
	}
	writeToServoblaster(3, dutycycle);
}

void Controller::forward(float dutycycle)
{
	cout << "forward" << dutycycle << endl;
	if (dutycycle < 1 || dutycycle > 2) {
		printf("Error, forward %f\n", dutycycle);
		return;
	}
	writeToServoblaster(1, dutycycle);
}

void Controller::side(float dutycycle)
{
	cout << "side" << dutycycle << endl;
	if (dutycycle < 1 || dutycycle > 2) {
		printf("Error, side %f\n", dutycycle);
		return;
	}
	writeToServoblaster(2, dutycycle);
}
