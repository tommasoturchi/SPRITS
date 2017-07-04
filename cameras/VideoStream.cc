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
	boost::tuple<int, int> cropOrigin;
	boost::tuple<int, int> cropTarget;
	bool cropped;
protected:
	void setCropping(boost::tuple<int, int> origin, boost::tuple<int, int> target)
	{
		cropOrigin = origin;
		cropTarget = target;
		cropped = true;
		//capture->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		//capture->set(CV_CAP_PROP_FRAME_HEIGHT, 720);
		spdlog::get("console")->debug("Cropping data received.");
	}
	void resetCropping()
	{
		cropped = false;
		spdlog::get("console")->debug("Cropping data reset requested.");
	}
public:
	VideoStream(bool crop = false, bool debug = false) : Camera(crop, debug)
	{
		spdlog::get("console")->info("Opening VideoCapture device...");
		capture = new cv::VideoCapture(0);
		if (!capture->isOpened())
			throw std::runtime_error("Unable to initialise video capture!");
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
		{
			if (cropped)
			{
				cv::Rect roi;
		    	roi.x = boost::get<0>(cropOrigin);
		    	roi.y = boost::get<1>(cropOrigin);
		    	roi.width = boost::get<0>(cropTarget) - boost::get<0>(cropOrigin);
		    	roi.height = boost::get<1>(cropTarget) - boost::get<1>(cropOrigin);
		    	inputImage = inputImage(roi);
		    }
			notify(NewFrameEvent::COLOR, inputImage);
		}
		Camera::update();
	}
};

#endif