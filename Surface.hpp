#ifndef SURFACE_H
#define SURFACE_H

#include <list>

#include <opencv2/opencv.hpp>

namespace SPRITS {
	class Surface
	{
		std::list <class AbstractSurfaceObserver *> observers;
	public:
		virtual ~Surface();
		virtual int size() = 0;
		virtual cv::Point getPosition(int id) = 0;
		virtual void setPosition(int id, cv::Point pos) = 0;
		void subscribe(AbstractSurfaceObserver *sobs);
		void unsubscribe(AbstractSurfaceObserver *sobs);
		void notifyObservers();
	};
	
	class AbstractSurfaceObserver
	{
		Surface *sur;
	public:
		AbstractSurfaceObserver(Surface *s);
		virtual ~AbstractSurfaceObserver();
		virtual void update() = 0;
	};
}

#endif