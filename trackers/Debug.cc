#include <Camera.hpp>
#include <Manifold.hpp>

#include <time.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

#include <boost/thread.hpp>
#include <thread>

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
	
class DebugWindow : public CameraObserver<std::tuple<double, double, double>>, public ManifoldObserver<std::tuple<double, double, double>>
{
private:
	Fl_Window *window;
	Fl_Box *colorBox;
	std::map<int, std::tuple<double, double, double>> ids;
public:
	DebugWindow(Camera *cam, Manifold<std::tuple<double, double, double>> *man) : CameraObserver<std::tuple<double, double, double>>(cam, man, NewFrameEvent::COLOR), ManifoldObserver<std::tuple<double, double, double>>(man)
	{
		window = new Fl_Window(0, 0, 640, 480, "Debug");
		window->begin();
		colorBox = new Fl_Box(0, 0, 640, 480);
		window->end();
		Fl::visual(FL_RGB);
	}
	
	~DebugWindow() {
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		if (window)
		{
			cv::Mat outImage;
			outImage = frame;
			for (auto id : ids)
				cv::circle(outImage, cv::Point((int)(std::get<0>(id.second)*640), (int)(std::get<1>(id.second)*480)), 8, cv::Scalar( 255, 0, 0 ), 1, 8 );
			IplImage ipltemp;
			if (event == NewFrameEvent::COLOR)
			{
				ipltemp = outImage;
			}
			else if (event == NewFrameEvent::DEPTH)
			{
				cv::Mat normalizedImage;
				double min, max;
				cv::minMaxIdx(frame, &min, &max);
				outImage.convertTo(normalizedImage, CV_8UC1, 255 / (max-min), -min*255 / (max-min)); 
				ipltemp = normalizedImage;
			}
			IplImage* colorIplImage = cvCreateImage(cvSize(frame.cols, frame.rows), 8, frame.channels());
			cvCopy(&ipltemp, colorIplImage);
			Fl_RGB_Image* colorFlImage = new Fl_RGB_Image((unsigned char*)colorIplImage->imageData, frame.cols, frame.rows, frame.channels(), colorIplImage->widthStep);
			Fl::lock();
			colorBox->image(colorFlImage);
			colorBox->redraw();
			window->redraw();
			window->show();
			Fl::wait();
			Fl::unlock();
			Fl::awake();
			cvReleaseImage(&colorIplImage);
			colorFlImage->uncache();
			Fl::flush();
			Fl::wait();
		}
	}
	
	void fire(const ElementEvent& event, int id)
	{
		switch (event.get_state()) {
			case ADD:
			case UPDATE:
			ids[id] = ManifoldObserver<std::tuple<double, double, double>>::man_->getElement(id);
			break;
			case REMOVE:
			ids.erase(id);
			break;
		}
	}
};