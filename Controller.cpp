#include "Controller.hpp"

#include <iostream>
#include <fstream>

#include <sstream>


Controller::Controller() 
{
	turnProportionalGain = 1.0f; //converts offset from camera to turn duty cycle
	elivationProportionalGain = -1.0f; // converts offset in the y direction to elivation
	sideProportionalGain = 1.0f;
	lowerAltitudeDutyCycle = 0.2f; //Worst case senario, slowly lower copter to the floor.
}

Controller::~Controller()
{

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
	elivation(marker.Tvec.at<float>(1,0)*elivationProportionalGain);
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
	int val = atoi(&command[2]);
	printf("val found %d \n", val);
	switch (command[1]) {
		case '0':
			turn(val/100.0);
			break;
		case '1':
			elivation(val/100.0);
			break;

		case '2':
			forward(val/100.0);
			break;

		case '3':
			side(val/100.0);
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

void Controller::elivation(float dutycycle)
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