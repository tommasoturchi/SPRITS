#ifndef OPENNI_CC
#define OPENNI_CC

#include <Camera.hpp>

#include <OpenNI.h>

using namespace SPRITS;

struct smode
{
	unsigned int resX, resY, FPS;
	openni::PixelFormat format;
	smode(int resX, int resY, int FPS, openni::PixelFormat format) : resX(resX), resY(resY), FPS(FPS), format(format) {};
	const char *getFormatName()
	{
		switch (format)
		{
			case openni::PIXEL_FORMAT_DEPTH_1_MM:
			return "1 mm";
			case openni::PIXEL_FORMAT_DEPTH_100_UM:
			return "100 um";
			case openni::PIXEL_FORMAT_SHIFT_9_2:
			return "Shifts 9.2";
			case openni::PIXEL_FORMAT_SHIFT_9_3:
			return "Shifts 9.3";
			case openni::PIXEL_FORMAT_RGB888:
			return "RGB 888";
			case openni::PIXEL_FORMAT_YUV422:
			return "YUV 422";
			case openni::PIXEL_FORMAT_YUYV:
			return "YUYV";
			case openni::PIXEL_FORMAT_GRAY8:
			return "Grayscale 8-bit";
			case openni::PIXEL_FORMAT_GRAY16:
			return "Grayscale 16-bit";
			case openni::PIXEL_FORMAT_JPEG:
			return "JPEG";
			default:
			return "Unknown";
		}
	}
};

std::ostream& operator<<(std::ostream& os, smode *s)
{
	return os << s->resX << " x " << s->resY << " @ " << s->FPS << " (" << s->getFormatName() << ")";
}

class OpenNI : public Camera
{
	template<typename T>
	struct Listener : public openni::VideoStream::NewFrameListener
	{
		std::function<void(T, openni::VideoFrameRef)> cb;
		virtual void onNewFrame(openni::VideoStream &stream)
		{
			openni::VideoFrameRef frame;
			stream.readFrame(&frame);
			T img(frame.getHeight(), frame.getWidth());
			for (int y = 0; y < img.rows; ++y)
				memcpy(img.ptr(y), ((uint8_t *)frame.getData()) + y*frame.getStrideInBytes(), img.cols*img.elemSize());
			if (cb) cb(img, frame);
		}
	};
	
	Listener<cv::Mat3b> colorListener;
	Listener<cv::Mat1s> depthListener;
	openni::Device device;
	openni::VideoStream colorStream, depthStream;
	cv::Size colorRes, depthRes;
	
	static std::vector<smode*> getSupportedModes(openni::SensorType sensor)
	{
		openni::Device dev;
		openni::OpenNI::initialize();
		if (dev.open(openni::ANY_DEVICE) != openni::STATUS_OK)
		{
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't open any device!");
		}
		std::vector<smode*> modes;
		const openni::Array<openni::VideoMode> &supportedModes = dev.getSensorInfo(sensor)->getSupportedVideoModes();
		for (int i = 0; i < supportedModes.getSize(); ++i)
			modes.push_back(new smode(supportedModes[i].getResolutionX(), supportedModes[i].getResolutionY(), supportedModes[i].getFps(), supportedModes[i].getPixelFormat()));
		dev.close();
		openni::OpenNI::shutdown();
		return modes;
	}
public:
	OpenNI() : OpenNI(4, 9) { };
	
	OpenNI(int dmode, int cmode)
	{
		std::cout << "Starting OpenNI... ";
		openni::OpenNI::initialize();
		if (device.open(openni::ANY_DEVICE) != openni::STATUS_OK)
		{
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't open any device!");
		}
		device.setDepthColorSyncEnabled(true);
		device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		if (colorStream.create(device, openni::SENSOR_COLOR) != openni::STATUS_OK)
		{
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't find any color stream!");
		}
		if (depthStream.create(device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
		{
			openni::OpenNI::shutdown();
			throw std::runtime_error("Couldn't find any depth stream!");
		}

		colorListener.cb = [&](cv::Mat3b color_, openni::VideoFrameRef frame){
			const openni::RGB888Pixel *pColor = (const openni::RGB888Pixel *)frame.getData();
			for (int i = 0; i < frame.getHeight(); ++i)
				for (int j = 0; j < frame.getWidth(); ++j)
				{
					color_.at<cv::Vec3b>(i, frame.getWidth() - (j + 1))[0] = pColor[i * frame.getWidth() + j].b;
					color_.at<cv::Vec3b>(i, frame.getWidth() - (j + 1))[1] = pColor[i * frame.getWidth() + j].g;
					color_.at<cv::Vec3b>(i, frame.getWidth() - (j + 1))[2] = pColor[i * frame.getWidth() + j].r;
				}
			notify(NewFrameEvent::COLOR, color_); };
		depthListener.cb = [&](cv::Mat1s depth_, openni::VideoFrameRef frame){ notify(NewFrameEvent::DEPTH, depth_); };
		colorStream.addNewFrameListener(&colorListener);
		depthStream.addNewFrameListener(&depthListener);

		colorStream.setVideoMode(device.getSensorInfo(openni::SENSOR_COLOR)->getSupportedVideoModes()[cmode]);
		colorStream.start();
		depthStream.setVideoMode(device.getSensorInfo(openni::SENSOR_DEPTH)->getSupportedVideoModes()[dmode]);
		depthStream.start();
		
		colorRes = cv::Size(colorStream.getVideoMode().getResolutionX(), colorStream.getVideoMode().getResolutionY());
		depthRes = cv::Size(depthStream.getVideoMode().getResolutionX(), depthStream.getVideoMode().getResolutionY());
		
		std::cout << "OK!" << std::endl;
	}
	
	~OpenNI()
	{
		std::cout << "Stopping OpenNI... ";
		colorStream.stop();
		colorStream.destroy();
		depthStream.stop();
		depthStream.destroy();
		device.close();
		openni::OpenNI::shutdown();
		std::cout << "OK!" << std::endl;
	}
	
	static std::vector<smode*> getSupportedColorModes()
	{
		return getSupportedModes(openni::SENSOR_COLOR);
	}
	
	static std::vector<smode*> getSupportedDepthModes()
	{
		return getSupportedModes(openni::SENSOR_DEPTH);
	}
};

#endif