#ifndef CAMERA_CPP
#define CAMERA_CPP

#include <iostream>
#include <list>

#include <opencv2/opencv.hpp>

#include <Camera.hpp>
#include <CameraObservers.hpp>

namespace SPRITS {
	Camera::~Camera() {
		cobservers.clear();
		dobservers.clear();
	};
	void Camera::subscribe(AbstractCameraColorObserver *cobs) {
		cobservers.push_back(cobs);
	};
	void Camera::subscribe(AbstractCameraDepthObserver *cobs) {
		dobservers.push_back(cobs);
	};
	void Camera::unsubscribe(AbstractCameraColorObserver *cobs) {
		cobservers.remove(cobs);
	};
	void Camera::unsubscribe(AbstractCameraDepthObserver *cobs) {
		dobservers.remove(cobs);
	};
	void Camera::notifyColorObservers() {
		for (auto it = cobservers.begin(); it != cobservers.end(); ++it)
			(*it)->update();
	};
	void Camera::notifyDepthObservers() {
		for (auto it = dobservers.begin(); it != dobservers.end(); ++it)
			(*it)->update();
	};
}

#endif