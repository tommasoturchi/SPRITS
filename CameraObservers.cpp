#ifndef CAMERAOBSERVERS_CPP
#define CAMERAOBSERVERS_CPP

#include <CameraObservers.hpp>

namespace SPRITS {
	AbstractCameraObserver::AbstractCameraObserver(Camera *c, Surface *s) : cam(c), sur(s) { };
	
	AbstractCameraColorObserver::AbstractCameraColorObserver(Camera *c, Surface *s) : AbstractCameraObserver(c, s)
	{
		cam->subscribe(this);
	};
	AbstractCameraColorObserver::~AbstractCameraColorObserver() {
		cam->unsubscribe(this);
	};
	void AbstractCameraColorObserver::delegate() {
		cam->unsubscribe(this);
	};
	
	AbstractCameraDepthObserver::AbstractCameraDepthObserver(Camera *c, Surface *s) : AbstractCameraObserver(c, s)
	{
		cam->subscribe(this);
	};
	AbstractCameraDepthObserver::~AbstractCameraDepthObserver() {
		cam->unsubscribe(this);
	};
	void AbstractCameraDepthObserver::delegate() {
		cam->unsubscribe(this);
	};
	
	AbstractCameraColorObserverDecorator::AbstractCameraColorObserverDecorator(AbstractCameraColorObserver *o) : AbstractCameraColorObserver(o->cam, o->sur), component(o) {
		component->delegate();
	};
	void AbstractCameraColorObserverDecorator::update() {
		component->update();
	};
	
	AbstractCameraDepthObserverDecorator::AbstractCameraDepthObserverDecorator(AbstractCameraDepthObserver *o) : AbstractCameraDepthObserver(o->cam, o->sur), component(o) {
		component->delegate();
	};
	void AbstractCameraDepthObserverDecorator::update() {
		component->update();
	};
}

#endif