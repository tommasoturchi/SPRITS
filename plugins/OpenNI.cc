#ifndef OPENNI_CC
#define OPENNI_CC

#include <Camera.hpp>

#include <OpenNI.h>

using namespace SPRITS;

class OpenNI : public Camera
{
	template<typename T>
	struct Listener : public openni::VideoStream::NewFrameListener { 
		std::function<void(T)> cb;
		virtual void onNewFrame(openni::VideoStream &stream) {
			openni::VideoFrameRef frame;
			stream.readFrame(&frame);
			T img(frame.getHeight(), frame.getWidth());
			for (int y = 0; y < img.rows; ++y)
				memcpy(img.ptr(y), ((uint8_t *)frame.getData()) + y*frame.getStrideInBytes(), img.cols*img.elemSize());
			if (cb) cb(img);
		}
	};
	Listener<cv::Mat3b> colorListener;
	Listener<cv::Mat1s> depthListener;
	openni::Device device;
	openni::VideoStream colorStream, depthStream;
	cv::Mat color, depth;
public:
	OpenNI() {
		std::cout << "Starting OpenNI... ";
		openni::OpenNI::initialize();
		if (device.open(openni::ANY_DEVICE) != openni::STATUS_OK) {
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't open any device!");
		}
		device.setDepthColorSyncEnabled(true);
		device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		if (colorStream.create(device, openni::SENSOR_COLOR) != openni::STATUS_OK) {
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't find any color stream!");
		}
		if (depthStream.create(device, openni::SENSOR_DEPTH) != openni::STATUS_OK) {
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't find any depth stream!");
		}

		colorListener.cb = [&](cv::Mat3b _color){ color = _color; notifyColorObservers(); };
		depthListener.cb = [&](cv::Mat1s _depth){ depth = _depth; notifyDepthObservers(); };
		colorStream.addNewFrameListener(&colorListener);
		depthStream.addNewFrameListener(&depthListener);

		colorStream.start();
		depthStream.start();
		
		std::cout << "OK!" << std::endl;
	}
	~OpenNI() {
		std::cout << "Stopping OpenNI... ";
		colorStream.stop();
		colorStream.destroy();
		depthStream.stop();
		depthStream.destroy();
		device.close();
		openni::OpenNI::shutdown();
		std::cout << "OK!" << std::endl;
	}
	cv::Mat getColorFrame() {
		return color;
	}
	cv::Mat getDepthFrame() {
		return depth;
	}
};

#endif