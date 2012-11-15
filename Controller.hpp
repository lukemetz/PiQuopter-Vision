#pragma once
#define PI 3.1415


#include <aruco/aruco.h>

class Controller
{
public:
	//Control constants
	float turnProportionalGain;
	float elivationProportionalGain;
	float sideProportionalGain;

	//Other constants
	float lowerAltitudeDutyCycle;


	Controller();
	~Controller();
	void controlMarker(aruco::Marker &marker);

	//output functions
	void turn(float dutycycle);
	void elivation(float dutycycle);
	void forward(float dutycycle);
	void side(float dutycycle);

	bool isPerpendicular(aruco::Marker &marker);
	cv::Mat getDelta(aruco::Marker &marker);
private:

};