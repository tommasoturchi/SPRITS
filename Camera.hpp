#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <list>

#include <opencv2/opencv.hpp>

#include <CameraObservers.hpp>

namespace SPRITS {
	class Camera
	{
		std::list <class AbstractCameraColorObserver *> cobservers;
		std::list <class AbstractCameraDepthObserver *> dobservers;
	public:
		virtual ~Camera();
		virtual cv::Mat getColorFrame() = 0;
		virtual cv::Mat getDepthFrame() = 0;
		void subscribe(class AbstractCameraColorObserver *cobs);
		void subscribe(class AbstractCameraDepthObserver *cobs);
		void unsubscribe(class AbstractCameraColorObserver *cobs);
		void unsubscribe(class AbstractCameraDepthObserver *cobs);
		void notifyColorObservers();
		void notifyDepthObservers();
	};
}

#endif