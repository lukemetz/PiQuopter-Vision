#pragma once

#define PI 3.1415

#include <aruco/aruco.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

/*
 * The Controller class does all of the "autonomy" for the quad copter.
 * Currently the center on qr code is here as well as the basic autonomy.
 */

class Controller
{
public:
	/* Public member variables */
	//tcp socket
	boost::shared_ptr<boost::asio::ip::tcp::socket> sock;
	//Control constants used for centering on qr codes
	float turnProportionalGain;
	float throttleProportionalGain;
	float sideProportionalGain;

	//Other constants
	float lowerAltitudeDutyCycle; //Constant used to slowly lower copter to the ground.

	Controller();
	~Controller();

  	void markerBasicMovement(int markerId); //Controls for basic movement
  	//more advanced marker controls.
	void controlMarker(aruco::Marker &marker);

	//output functions
	void turn(float dutycycle);
	void throttle(float dutycycle);
	void forward(float dutycycle);
	void side(float dutycycle);

	//Commands received from the tcp socket
	void command(char *command);
	//Test to see if it is perpendicular to the marker in question
	bool isPerpendicular(aruco::Marker &marker);
	//Get the delta from perpendicular.
	cv::Mat getDelta(aruco::Marker &marker);


private:
	//Output to servo blaster to control the motors/
	void writeToServoblaster(int servo, float dutycycle);
};
