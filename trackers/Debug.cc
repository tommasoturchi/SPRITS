#include <Camera.hpp>

#include <time.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

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
	clock_t start = clock();
	int seconds_;
public:
	Debug(Camera *cam, Manifold<T> *man, int seconds, const NewFrameEvent& event) : CameraObserver<T>(cam, man, event)
	{
		seconds_ = seconds;
		counters_[event] = FPSCounter();
	}
	
	template<typename E, typename... Events>
	Debug(Camera *cam, Manifold<T> *man, int seconds, E event, Events... events) : CameraObserver<T>(cam, man, event, events...)
	{
		seconds_ = seconds;
		for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
			counters_[event] = FPSCounter();
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		counters_[event].tick();
		clock_t end = clock();
		if ((end - start)/(double)CLOCKS_PER_SEC > seconds_)
		{
			std::cout << counters_[event].getFPS() << (event==NewFrameEvent::COLOR?" fps/color":" fps/depth") << std::endl;
			start = end;
		}
	}
};

/*class FPS
{
	double lastTime, currentTime, fps;
	struct timeval time;
	int turn;
	std::string sname;
public:
	FPS(int f, std::string s) : frequency(f), sname(s) {
		gettimeofday(&time, NULL);
		lastTime = time.tv_sec + time.tv_usec*1e-6;
	}
	void tick() {
		gettimeofday(&time, NULL);
		currentTime = time.tv_sec + time.tv_usec*1e-6;
		fps = 1/(currentTime - lastTime);
		lastTime = currentTime;
		if (turn++ % frequency == 0)
			std::cout << fps << " fps/" << sname << std::endl;
	}
protected:
	int frequency;
};

class ColorFPS : public AbstractCameraObserverDecorator, public FPS
{
public:
	ColorFPS(AbstractCameraObserver *o, int f) : AbstractCameraObserverDecorator(o), FPS(f, "color") { }
	void update() {
		tick();
		AbstractCameraObserverDecorator::update();
	}
};

class DebugStream
{
	int width, height;
	std::string wname;
public:
	DebugStream(std::string wname) : wname(wname) {
		cv::namedWindow(wname, cv::WINDOW_AUTOSIZE);
		window = new Fl_Window(0, 0, w, h, wname.c_str());
		window->begin();
		box = new Fl_Box(0, 0, w, h);
		window->end();
		window->show();
		iplImage = cvCreateImage(cvSize(w, h), 8, 3);
		//FlImage = new Fl_RGB_Image((unsigned char*)iplImage->imageData, w, h, 3, iplImage->widthStep);
		//Fl::awake();
	}
	~DebugStream() {
		//delete box;
		//delete window;
	}
	void normalize(cv::Mat image) {
		//cv::Mat normalizedImage;
		//cv::normalize(image, normalizedImage, 0, 255, CV_MINMAX, CV_8UC3); 
		//show(normalizedImage, 1);
	}
	void show(cv::Mat image) {
		//cvReleaseImage(&iplImage);
		//iplImage = cvCreateImage(cvSize(image.cols, image.rows), 8, 3);
		//IplImage ipltemp = image;
		//cvCopy(&ipltemp, iplImage);
		//FlImage->uncache();
		//FlImage = new Fl_RGB_Image((unsigned char*)iplImage->imageData, width, height, channels, iplImage->widthStep);
		//Fl::lock();
		//box->image(FlImage);
		//box->redraw();
		//Fl::wait();
		//Fl::unlock();
		//Fl::awake();
	    cv::imshow(wname, image);
	    cv::waitKey(1); 
	}
};

class ColorDebugStream : public AbstractCameraObserverDecorator, public DebugStream
{
public:
	ColorDebugStream(AbstractCameraObserver *o) : AbstractCameraObserverDecorator(o), DebugStream("ColorStream") { }
	void update() {
		show(cam->getColorFrame());
		AbstractCameraObserverDecorator::update();
	}
};*/