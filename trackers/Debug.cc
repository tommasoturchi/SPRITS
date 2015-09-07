#include <Camera.hpp>
#include <Manifold.hpp>

#include <time.h>

using namespace SPRITS;

struct FPSCounter
{
	double lastTime, currentTime, fps;
	struct timeval time;
	void tick()
	{
		gettimeofday(&time, NULL);
		currentTime = time.tv_sec + time.tv_usec*1e-6;
		fps = 1/(currentTime - lastTime);
		lastTime = currentTime;
	}
	double getFPS()
	{
		return fps;
	}
};

template<typename T> class Debug : public CameraObserver<T>
{
private:
	std::map<NewFrameEvent, FPSCounter> counters_;
	std::map<NewFrameEvent, clock_t> start_;
	int seconds_;
public:
	Debug(Camera *cam, Manifold<T> *man, int seconds, const NewFrameEvent& event) : CameraObserver<T>(cam, man, event)
	{
		seconds_ = seconds;
		counters_[event] = FPSCounter();
		start_[event] = clock();
	}
	
	template<typename E, typename... Events>
	Debug(Camera *cam, Manifold<T> *man, int seconds, E event, Events... events) : CameraObserver<T>(cam, man, event, events...)
	{
		seconds_ = seconds;
		for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
		{
			counters_[event] = FPSCounter();
			start_[event] = clock();
		}
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		counters_[event].tick();
		clock_t end = clock();
		if ((end - start_[event])/(double)CLOCKS_PER_SEC > seconds_)
		{
			std::cout << (event==NewFrameEvent::COLOR?"Color":"Depth") << " sensor running at " << counters_[event].getFPS() << " FPS." << std::endl;
			start_[event] = end;
		}
	}
};