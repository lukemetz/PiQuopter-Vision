#include "Controller.hpp"

Controller::Controller()
{
	turnProportionalGain = 1.0f; //converts offset from camera to turn duty cycle
	throttleProportionalGain = 10.0f; // converts offset in the y direction to throttle
	sideProportionalGain = 1.0f;
	hover_throttle = 1.54;
	lower_altitude = 0;
  	//initialize all controls to neutral
	turn(1.5);
	forward(1.48);
	side(1.59);
	throttle(1.09);
	isStarted = 0;
	time_lift = 0;
}