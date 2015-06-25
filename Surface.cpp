#ifndef SURFACE_CPP
#define SURFACE_CPP

#include <list>

#include <opencv2/opencv.hpp>

#include <Surface.hpp>

namespace SPRITS {
	Surface::~Surface() {
		observers.clear();
	};
	void Surface::subscribe(AbstractSurfaceObserver *sobs) {
		observers.push_back(sobs);
	};
	void Surface::unsubscribe(AbstractSurfaceObserver *sobs) {
		observers.remove(sobs);
	};
	void Surface::notifyObservers()  {
		for (auto it = observers.begin(); it != observers.end(); ++it)
			(*it)->update();
	};
	
	AbstractSurfaceObserver::AbstractSurfaceObserver(Surface *s) : sur(s) {
		sur->subscribe(this);
	};
	AbstractSurfaceObserver::~AbstractSurfaceObserver() {
		sur->unsubscribe(this);
	};
}

#endif