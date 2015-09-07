#ifndef VIDEOSTREAM_CC
#define VIDEOSTREAM_CC

#include <Camera.hpp>

#include <opencv2/highgui/highgui.hpp>

using namespace SPRITS;

class VideoStream : public Camera
{
	cv::VideoCapture *capture;
	cv::Mat3b inputImage;
protected:
	void setCalibration(std::tuple<int, int> origin, std::tuple<int, int> target)
	{
		std::cout << "Received calibration data." << std::endl;
	}
	void resetCalibration()
	{
		std::cout << "Reset calibration data." << std::endl;
	}
public:
	VideoStream() : Camera(true)
	{
		std::cout << "Starting VideoCapture... ";
		capture = new cv::VideoCapture(0);
		if (!capture->isOpened())
			throw std::runtime_error("Unable to initialise video capture!");
		capture->set(CV_CAP_PROP_FRAME_WIDTH, 320);
		capture->set(CV_CAP_PROP_FRAME_HEIGHT, 240);
		std::cout << "OK!" << std::endl;
	}
	~VideoStream()
	{
		std::cout << "Stopping VideoCapture... ";
		std::cout.setstate(std::ios_base::failbit);
		delete capture;
		std::cout.clear();
		std::cout << "OK!" << std::endl;
	}
	void update()
	{
		if (capture->read(inputImage))
			notify(NewFrameEvent::COLOR, inputImage);
		Camera::update();
	}
};

#endif