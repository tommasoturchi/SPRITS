#include <Camera.hpp>
#include <CameraObservers.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

using namespace SPRITS;

class FPS
{
	double lastTime, currentTime, fps;
	struct timeval time;
	int turn;
public:
	FPS(int f) : frequency(f) {
		gettimeofday(&time, NULL);
		lastTime = time.tv_sec + time.tv_usec*1e-6;
	}
	void tick() {
		gettimeofday(&time, NULL);
		currentTime = time.tv_sec + time.tv_usec*1e-6;
		fps = 1/(currentTime - lastTime);
		lastTime = currentTime;
		if (turn++ % frequency == 0)
			std::cout << fps << " fps/color" << std::endl;
	}
protected:
	int frequency;
};

class ColorFPS : public AbstractCameraColorObserverDecorator, public FPS
{
public:
	ColorFPS(AbstractCameraColorObserver *o, int f) : AbstractCameraColorObserverDecorator(o), FPS(f) { }
	void update() {
		tick();
		AbstractCameraColorObserverDecorator::update();
	}
};

class DepthFPS : public AbstractCameraDepthObserverDecorator, public FPS
{
public:
	DepthFPS(AbstractCameraDepthObserver *o, int f) : AbstractCameraDepthObserverDecorator(o), FPS(f) { }
	void update() {
		tick();
		AbstractCameraDepthObserverDecorator::update();
	}
};

class DebugStream
{
	Fl_Window *window;
	Fl_Box *box;
	IplImage *iplImage;
	Fl_RGB_Image *FlImage;
	int width, height;
public:
	DebugStream(int w, int h, std::string wname) : width(w), height(h) {
		window = new Fl_Window(0, 0, w, h, wname.c_str());
		window->begin();
		box = new Fl_Box(0, 0, w, h);
		window->end();
		window->show();
	}
	~DebugStream() {
		delete box;
		delete window;
	}
	void show(cv::Mat image) {
		iplImage = cvCreateImage(cvSize(image.cols, image.rows), 8, 3);
		IplImage ipltemp = image;
		cvCopy(&ipltemp, iplImage);
		FlImage = new Fl_RGB_Image((unsigned char*)iplImage->imageData, width, height, 3, iplImage->widthStep);
		Fl::lock();
		box->image(FlImage);
		box->redraw();
		Fl::wait();
		Fl::unlock();
	}
};

class ColorDebugStream : public AbstractCameraColorObserverDecorator, public DebugStream
{
public:
	ColorDebugStream(AbstractCameraColorObserver *o, int w, int h) : AbstractCameraColorObserverDecorator(o), DebugStream(w, h, "ColorStream") { }
	void update() {
		show(cam->getColorFrame());
		AbstractCameraColorObserverDecorator::update();
	}
};

class DepthDebugStream : public AbstractCameraDepthObserverDecorator, public DebugStream
{
public:
	DepthDebugStream(AbstractCameraDepthObserver *o, int w, int h) : AbstractCameraDepthObserverDecorator(o), DebugStream(w, h, "DepthStream") { }
	void update() {
		show(cam->getDepthFrame());
		AbstractCameraDepthObserverDecorator::update();
	}
};