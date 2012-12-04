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
	void command(char *command);
	bool isPerpendicular(aruco::Marker &marker);
	cv::Mat getDelta(aruco::Marker &marker);

	void writeToServoblaster(int servo, float dutycycle);
	boost::shared_ptr<boost::asio::ip::tcp::socket> sock;
private:

};