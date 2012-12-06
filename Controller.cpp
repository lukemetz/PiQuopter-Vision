#include "Controller.hpp"

#include <iostream>
#include <fstream>

#include <sstream>


Controller::Controller()
{
	turnProportionalGain = 1.0f; //converts offset from camera to turn duty cycle
	throttleProportionalGain = -1.0f; // converts offset in the y direction to throttle
	sideProportionalGain = 1.0f;
	lowerAltitudeDutyCycle = 0.2f; //Worst case senario, slowly lower copter to the floor.

  //initialize all controls to neutral
	turn(1.5);
  side(1.5);
  forward(1.5);
  throttle(1);
}

Controller::~Controller()
{

}

void Controller::markerBasicMovement(int markerId)
{
    //cout<<markerId<<endl;
    switch(markerId){
      case 412:
        turn(50);
        break;
      case 400:
        turn(0);
        break;
      case 300:
        throttle(0);
        break;
      default:
        break;

    }
}
void Controller::controlMarker(aruco::Marker &marker)
{
	//marker.Rxyz
	if (isPerpendicular(marker)) {
		cout << "Is isPerpendicular Ya!!" << endl;
	}
	cv::Mat deltas = getDelta(marker);
	cout <<deltas << endl;

	//If rotate the camera to make the tag in the center of the view
	turn(marker.Tvec.at<float>(0,0)*turnProportionalGain);
	side(getDelta(marker).at<float>(0,0)*sideProportionalGain);
	//Adjust the height to ensure that the tag is in the center.
	throttle(marker.Tvec.at<float>(1,0)*throttleProportionalGain);
}

cv::Mat Controller::getDelta(aruco::Marker &marker)
{
	float x = marker.Rvec.at<float>(0,0)/PI*180;
	float y = marker.Rvec.at<float>(1,0)/PI*180;
	float z = marker.Rvec.at<float>(2,0)/PI*180;
	cv::Mat ret = cv::Mat(3,1,CV_32FC1);
	//TODO remove magic calibrations
	ret.at<float>(0,0) = x;
	ret.at<float>(1,0) = abs(y)-130;
	ret.at<float>(2,0) = abs(z)-120;
	return ret;
}

bool Controller::isPerpendicular(aruco::Marker &marker)
{
	bool ret = true;
		float x = marker.Rvec.at<float>(0,0)/PI*180;
	float y = marker.Rvec.at<float>(1,0)/PI*180;
	float z = marker.Rvec.at<float>(2,0)/PI*180;

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
	//order Rudder elevation aileron throttle
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
	//ofstream servoblaster;
	//servoblaster.open ("/dev/servoblaster");
	//servoblaster << servo << "=" << static_cast<int>(dutycycle * 100);
	//servoblaster << "0=120";
	//servoblaster.close();
	std::string str;
	std::stringstream out;
	out << "echo ";
	out << servo;
	out << "=";
	out << static_cast<int>(dutycycle *100);
	out << " > /dev/servoblaster";

	printf("%s\n",out.str().c_str() );
	system(out.str().c_str());

}

void Controller::turn(float dutycycle)
{
	cout << "turn" << dutycycle << endl;
	writeToServoblaster(0, dutycycle);
}

void Controller::throttle(float dutycycle)
{
	cout << "elevation" << dutycycle << endl;
	writeToServoblaster(1, dutycycle);
}

void Controller::forward(float dutycycle)
{
	cout << "forward" << dutycycle << endl;
	writeToServoblaster(2, dutycycle);
}

void Controller::side(float dutycycle)
{
	cout << "side" << dutycycle << endl;
	writeToServoblaster(3, dutycycle);
}
