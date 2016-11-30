#include <Camera.hpp>
#include <Space.hpp>

#include <time.h>
#include <sys/time.h>
#include <spdlog/spdlog.h>

using namespace SPRITS;

#define TIMEOUT 10

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
public:
	Debug(Camera *cam, Space<T> *spc, const NewFrameEvent& event) : CameraObserver<T>(cam, spc, event)
	{
		counters_[event] = FPSCounter();
	}
	
	template<typename E, typename... Events>
	Debug(Camera *cam, Space<T> *spc, E event, Events... events) : CameraObserver<T>(cam, spc, event, events...)
	{
		for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
		{
			counters_[event] = FPSCounter();
		}
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		if (start_.find(event) == start_.end())
			start_[event] = clock();
		counters_[event].tick();
		clock_t end = clock();
		if ((end - start_[event])/(double)CLOCKS_PER_SEC > TIMEOUT)
		{
			spdlog::get("console")->info("{} sensor recording at {} FPS.", (event==NewFrameEvent::COLOR?"Color":"Depth"), counters_[event].getFPS());
			start_[event] = end;
		}
	}
};

class Debug3DTracker : public Debug<std::tuple<double, double, double>>
{
public:
	Debug3DTracker(Camera *cam, Space<std::tuple<double, double, double>> *spc, const NewFrameEvent& event) : Debug<std::tuple<double, double, double>>(cam, spc, event) { }
	
	template<typename E, typename... Events>
	Debug3DTracker(Camera *cam, Space<std::tuple<double, double, double>> *spc, E event, Events... events) : Debug<std::tuple<double, double, double>>(cam, spc, event, events...) { }
};