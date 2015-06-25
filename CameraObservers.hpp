#ifndef CAMERAOBSERVERS_H
#define CAMERAOBSERVERS_H

#include <Camera.hpp>
#include <Surface.hpp>

namespace SPRITS {
	class AbstractCameraObserver
	{
	public:
		explicit AbstractCameraObserver(class Camera *c, class Surface *s);
	protected:
		class Camera *cam;
		class Surface *sur;
		friend class AbstractCameraColorObserverDecorator;
		friend class AbstractCameraDepthObserverDecorator;
	};
	
	class AbstractCameraColorObserver : public AbstractCameraObserver
	{
	public:
		AbstractCameraColorObserver(class Camera *c, class Surface *s);
		~AbstractCameraColorObserver();
		virtual void update() = 0;
	protected:
		void delegate();
		friend class AbstractCameraColorObserverDecorator;
	};
	
	class AbstractCameraColorObserverDecorator : public AbstractCameraColorObserver
	{
		AbstractCameraColorObserver *component;
	public:
		AbstractCameraColorObserverDecorator(AbstractCameraColorObserver *o);
		virtual void update();
	};
	
	class AbstractCameraDepthObserver : public AbstractCameraObserver
	{
	public:
		AbstractCameraDepthObserver(class Camera *c, class Surface *s);
		~AbstractCameraDepthObserver();
		virtual void update() = 0;
	protected:
		void delegate();
		friend class AbstractCameraDepthObserverDecorator;
	};
	
	class AbstractCameraDepthObserverDecorator : public AbstractCameraDepthObserver
	{
		AbstractCameraDepthObserver *component;
	public:
		AbstractCameraDepthObserverDecorator(AbstractCameraDepthObserver *o);
		virtual void update();
	};
}

#endif