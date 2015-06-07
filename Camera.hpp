#include <iostream>
#include <list>

#include <opencv2/opencv.hpp>

#ifndef CAM_H
#define CAM_H

class Camera
{
	std::list <class AbstractCameraObserver *> observers;
public:
	virtual ~Camera() = default;
	virtual cv::Mat getColorFrame() = 0;
	virtual cv::Size getColorSize() = 0;
	virtual cv::Mat getDepthFrame() = 0;
	virtual cv::Size getDepthSize() = 0;
	void attach(AbstractCameraObserver *cobs) {
		observers.push_back(cobs);
	}
	void detach(AbstractCameraObserver *cobs) {
		observers.remove(cobs);
	}
	void notifyColor();
	void notifyDepth();
};

class AbstractCameraObserver 
{
	Camera *cam;
public:
	AbstractCameraObserver(Camera *c) {
		cam = c;
		cam->attach(this);
	}
	virtual ~AbstractCameraObserver() {
		cam->detach(this);
	}
	virtual void updateColor() = 0;
	virtual void updateDepth() = 0;
	virtual void process() {
		// TODO
	}
protected:
	AbstractCameraObserver() {}
	Camera *getCamera() {
		return cam;
	}
	friend class AbstractCameraObserverDecorator;
};

class AbstractCameraObserverDecorator : public AbstractCameraObserver
{
	AbstractCameraObserver *component;
public:
	AbstractCameraObserverDecorator(AbstractCameraObserver *o) {
		cam = o->getCamera();
		component = o;
		o->getCamera()->detach(o);
		o->getCamera()->attach(this);
	}
	void updateColor() {
		component->updateColor();
	}
	void updateDepth() {
		component->updateDepth();
	}
	virtual void process() {
		component->process();
	}
protected:
	AbstractCameraObserver* getComponent() {
		return component;
	}
};

void Camera::notifyColor()  {
	for (auto it = observers.begin(); it != observers.end(); ++it) {
		(*it)->updateColor();
		(*it)->process();
	}
};

void Camera::notifyDepth()  {
	for (auto it = observers.begin(); it != observers.end(); ++it) {
		(*it)->updateDepth();
		(*it)->process();
	}
};

#endif