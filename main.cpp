/****************************************************************************************
POE Quadcopter control code
The base for this code comes from a heavily modified Aruco example.
Original example copyright bellow.
*****************************************************************************************/
/*****************************************************************************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************************************************************************/
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

//Boost based libraries for server
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "Controller.hpp"

using namespace cv;
using namespace aruco;

string TheInputVideo; //Name of input video
string TheIntrinsicFile; //Configuration file for camera
float TheMarkerSize=-1; //Marker size in meters
int ThePyrDownLevel;
bool gui = 0; //Using the gui or not
MarkerDetector MDetector; //An Aruco marker detector used to find markers in image
VideoCapture TheVideoCapturer; //OpenCV Image capturer
vector<Marker> TheMarkers; //Array of found markers
Mat TheInputImage,TheInputImageCopy; //Two buffers to hold the images
CameraParameters TheCameraParameters; //Camera Parameters
Controller *controller; //Instance of our Controller object


void cvTackBarEvents(int pos,void*); //Used for to change threshold params

bool readCameraParameters(string TheIntrinsicFile,CameraParameters &CP,Size size);

pair<double,double> AvrgTime(0,0) ;//determines the average time required for detection
//The two threshold amount params.
double ThresParam1,ThresParam2; 
int iThresParam1,iThresParam2;
//Wait for keys to be pressed
int waitTime=0;

//Read the input arguments and change parameters.
bool readArguments ( int argc,char **argv )
{
    if (argc<2) {
        cerr<<"Invalid number of arguments"<<endl;
        cerr<<"Usage: (in.avi|live) [intrinsics.yml] [marker size in m] [gui 0 or 1]"<<endl;
        return false;
    }
    //Input type
    TheInputVideo=argv[1];
    if (argc>=3)
        TheIntrinsicFile=argv[2]; //Camera parameters
    if (argc>=4)
        TheMarkerSize=atof(argv[3]); //Marker size in meters
    if (argc>=5)
        gui = atoi(argv[4]); //Using gui or not
    return true;
}

using boost::asio::ip::tcp;

const int max_length = 1024;
typedef boost::shared_ptr<tcp::socket> socket_ptr;
//Function that manages each individual connection to the socket.
//This function is run in threads.
void session(socket_ptr sock, Controller *controller) {
    try {
    //listen for incoming data in an infinite loop
        char data[max_length];
        for (;;) {
            printf("got data %s\n", data );

            //Parse the command type and run it
            if (data[0] == 'C') {
                printf("command found %s", &data[1]);
                controller->command(data);
            }
            else if (data[0]=='s' || data[0]=='S'){ //Stop if we get anything that even looks like stop
                printf("Stop command found");
                controller->command((char *)"C1000");//set throttle to 0
            }

            //Read some data
            boost::system::error_code error;
            size_t length = sock->read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof) {
                printf("Connection closed by client \n");
                controller->sock.reset();
                break; // Connection closed cleanly by peer.
            }
            else if (error)
                throw boost::system::system_error(error); // Some other error.
            //Two ways so send back data, not currently used
            //boost::asio::write(*sock, boost::asio::buffer(data, length));
            //sock->send(boost::asio::buffer("data back \n", 10));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

//The main tcp server function.
void server_loop(Controller *controller)
{
    //startup the io_service and tcp acceptor.
    boost::asio::io_service io_service;
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), 8080));
    //Keep connections open and then shove a socket in when a connection has been found.
    for (;;) {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        //For now the controller will only have one socket/ client.
        controller->sock = sock;
        //Launch the session thead for that connection
        boost::thread t(boost::bind(session, sock, controller));
    }
}

int main(int argc,char **argv)
{
    //Create an instance of of the autonomy class.
    controller = new Controller();
    //Start up a new thread to run the server
    boost::thread server_thread(boost::bind(server_loop, controller));

    try {
        if (readArguments (argc,argv) == false) {
            return 0;
        }

        if (TheInputVideo=="live") {
            TheVideoCapturer.open(0); //USe the default camera 0.
            waitTime=10;
        }
        else  TheVideoCapturer.open(TheInputVideo);
        if (!gui) {
            TheVideoCapturer.set(CV_CAP_PROP_FRAME_WIDTH, 320);
            TheVideoCapturer.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
        }

        //TheVideoCapturer.set(CV_CAP_PROP_FPS, 5);

        //check video is open
        if (!TheVideoCapturer.isOpened()) {
            cerr<<"Could not open video"<<endl;
            return -1;
        }

        //read first image to get the dimensions
        TheVideoCapturer>>TheInputImage;

        //read camera parameters if passed
        if (TheIntrinsicFile!="") {
            TheCameraParameters.readFromXMLFile(TheIntrinsicFile);
            TheCameraParameters.resize(TheInputImage.size());
        }
        //Configure other parameters
        if (ThePyrDownLevel>0)
            MDetector.pyrDown(ThePyrDownLevel);

        //Create gui if running on laptop
        if (gui) {
            cv::namedWindow("thres",1);
            cv::namedWindow("in",1);
            MDetector.getThresholdParams( ThresParam1,ThresParam2);
            MDetector.setCornerRefinementMethod(MarkerDetector::LINES);
            iThresParam1=ThresParam1;
            iThresParam2=ThresParam2;
            cv::createTrackbar("ThresParam1", "in",&iThresParam1, 13, cvTackBarEvents);
            cv::createTrackbar("ThresParam2", "in",&iThresParam2, 13, cvTackBarEvents);
        }
        char key=0; //Current keypress in this char
        int index=0;
        double start_tick;

        //capture until press ESC or until the end of the video
        while ( key!=27 && TheVideoCapturer.grab())
        {
            float dt = ((double)getTickCount()-start_tick)/getTickFrequency();
            start_tick = (double)getTickCount();
            controller->step(dt);
            //Grab a frame
            TheVideoCapturer.retrieve( TheInputImage);

            //Buffer that we are going to send
            vector<uchar> buf;
            //Encode the image as a jpg and put into buf
            imencode(".jpg", TheInputImage, buf);
            //insert length of buffer to beginning
            unsigned long int len = buf.size();
            uchar * bufLen = (uchar *) &len;
            std::vector<uchar> len_vec(bufLen, bufLen + sizeof(unsigned long));
            buf.insert(buf.begin(), len_vec.begin(), len_vec.end());
            //If there is a socket
            if (controller->sock) {
                printf("writing over socket!!!\n");
                controller->sock->send(boost::asio::buffer(buf)); //Send the image over the socket.
            }

            index++; //number of images captured
            double tick = (double)getTickCount();//for checking the speed
            //Detection of markers in the image passed
            MDetector.detect(TheInputImage,TheMarkers,TheCameraParameters,TheMarkerSize);
            //check the speed by calculating the mean speed of all iterations
            AvrgTime.first+=((double)getTickCount()-tick)/getTickFrequency();
            AvrgTime.second++;
            //Print time between frames
            //cout<<"Time detection="<<1000*AvrgTime.first/AvrgTime.second<<" milliseconds"<<endl;
            //Copy the image
            TheInputImage.copyTo(TheInputImageCopy);

            if (gui) {
                //Draw the current marker
                for (unsigned int i=0;i<TheMarkers.size();i++) {
                    TheMarkers[i].draw(TheInputImageCopy,Scalar(0,0,255),1);
                }
                //draw a 3d cube in each marker if there is 3d info for testing purposes.
                if (  TheCameraParameters.isValid()) {
                    for (unsigned int i=0;i<TheMarkers.size();i++) {
                        CvDrawingUtils::draw3dCube(TheInputImageCopy,TheMarkers[i],TheCameraParameters);
                        CvDrawingUtils::draw3dAxis(TheInputImageCopy,TheMarkers[i],TheCameraParameters);
                    }
                }
            }

            for (unsigned int i=0;i<TheMarkers.size();i++) {
                controller->markerBasicMovement(TheMarkers[i].id);
                controller->controlMarker(TheMarkers[i]);
                break; //Only do the first one for now
            }

            if (gui) {
                //show input with augmented information and the thresholded image
                cv::imshow("in",TheInputImageCopy);
                cv::imshow("thres",MDetector.getThresholdedImage());
            }

            key=cv::waitKey(waitTime);//wait for key to be pressed
        }

    } catch (std::exception &ex) {
        cout<<"Exception :"<<ex.what()<<endl;
    }

}

void cvTackBarEvents(int pos,void*)
{
    if (iThresParam1<3) iThresParam1=3;
    if (iThresParam1%2!=1) iThresParam1++;
    if (ThresParam2<1) ThresParam2=1;
    ThresParam1=iThresParam1;
    ThresParam2=iThresParam2;
    MDetector.setThresholdParams(ThresParam1,ThresParam2);
    //recompute
    MDetector.detect(TheInputImage,TheMarkers,TheCameraParameters);
    TheInputImage.copyTo(TheInputImageCopy);
    for (unsigned int i=0;i<TheMarkers.size();i++)	TheMarkers[i].draw(TheInputImageCopy,Scalar(0,0,255),1);

    //draw a 3d cube in each marker if there is 3d info
    if (TheCameraParameters.isValid())
        for (unsigned int i=0;i<TheMarkers.size();i++)
            CvDrawingUtils::draw3dCube(TheInputImageCopy,TheMarkers[i],TheCameraParameters);

    cv::imshow("in",TheInputImageCopy);
    cv::imshow("thres",MDetector.getThresholdedImage());
}


