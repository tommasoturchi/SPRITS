#ifndef VIDEOSTREAM_CC
#define VIDEOSTREAM_CC

#include <Camera.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <spdlog/spdlog.h>

using namespace SPRITS;

class VideoStream : public Camera
{
	cv::VideoCapture *capture;
	cv::Mat3b inputImage;
protected:
	void setCropping(boost::tuple<int, int> origin, boost::tuple<int, int> target)
	{
		spdlog::get("console")->debug("Cropping data received.");
	}
	void resetCropping()
	{
		spdlog::get("console")->debug("Cropping data reset requested.");
	}
public:
	VideoStream() : Camera(true)
	{
		spdlog::get("console")->info("Opening VideoCapture device...");
		capture = new cv::VideoCapture(0);
		if (!capture->isOpened())
			throw std::runtime_error("Unable to initialise video capture!");
		capture->set(CV_CAP_PROP_FRAME_WIDTH, 320);
		capture->set(CV_CAP_PROP_FRAME_HEIGHT, 240);
		spdlog::get("console")->info("VideoCapture device opened successfully!");
	}
	~VideoStream()
	{
		spdlog::get("console")->info("Closing VideoCapture device...");
		std::cout.setstate(std::ios_base::failbit);
		delete capture;
		std::cout.clear();
		spdlog::get("console")->info("VideoCapture device closed successfully!");
	}
	void update()
	{
		if (capture->read(inputImage))
			notify(NewFrameEvent::COLOR, inputImage);
		Camera::update();
	}
};

#endif