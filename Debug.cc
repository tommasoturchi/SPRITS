#include <Camera.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

class FPS : public AbstractCameraObserverDecorator
{
	double lastTimeColor, lastTimeDepth, currentTimeColor, currentTimeDepth, fpsColor, fpsDepth;
	struct timeval timeColor, timeDepth;
	int turn, frequency;
public:
	FPS(AbstractCameraObserver *o, int f) : AbstractCameraObserverDecorator(o) {
		frequency = f;
		gettimeofday(&timeColor, NULL);
		gettimeofday(&timeDepth, NULL);
		lastTimeColor = timeColor.tv_sec + timeColor.tv_usec*1e-6;
		lastTimeDepth = timeDepth.tv_sec + timeDepth.tv_usec*1e-6;
	}
	void updateColor() {
		gettimeofday(&timeColor, NULL);
		currentTimeColor = timeColor.tv_sec + timeColor.tv_usec*1e-6;
		fpsColor = 1/(currentTimeColor - lastTimeColor);
		lastTimeColor = currentTimeColor;
		if (turn++ % frequency == 0)
			std::cout << fpsColor << " fps/color & " << fpsDepth << " fps/depth" << std::endl;
		getComponent()->updateColor();
	}
	void updateDepth() {
		gettimeofday(&timeDepth, NULL);
		currentTimeDepth = timeDepth.tv_sec + timeDepth.tv_usec*1e-6;
		fpsDepth = 1/(currentTimeDepth - lastTimeDepth);
		lastTimeDepth = currentTimeDepth;
		if (turn++ % frequency == 0)
			std::cout << fpsColor << " fps/color & " << fpsDepth << " fps/depth" << std::endl;
		getComponent()->updateDepth();
	}
	void process() {
		getComponent()->process();
	}
};

class DebugStream : public AbstractCameraObserverDecorator
{
	Fl_Window *window;
	Fl_Box *colorBox, *depthBox;
	IplImage *colorIplImage, *depthIplImage;
	Fl_RGB_Image *colorFlImage, *depthFlImage;
public:
	DebugStream(AbstractCameraObserver *o) : AbstractCameraObserverDecorator(o) {
		window = new Fl_Window(0, 0, getCamera()->getColorSize().width + getCamera()->getDepthSize().width, getCamera()->getColorSize().height, "Debug");
		window->begin();
	    colorBox = new Fl_Box(0, 0, getCamera()->getColorSize().width, getCamera()->getColorSize().height);
		depthBox = new Fl_Box(getCamera()->getColorSize().width, 0, getCamera()->getDepthSize().width, getCamera()->getDepthSize().height);
		window->end();
		window->show();
	}
	~DebugStream() {
		delete window;
	}
	void updateColor() {
		cv::Mat image = getCamera()->getColorFrame();
		colorIplImage = cvCreateImage(cvSize(image.cols, image.rows), 8, 3);
		IplImage ipltemp = image;
		cvCopy(&ipltemp, colorIplImage);
		colorFlImage = new Fl_RGB_Image((unsigned char*)colorIplImage->imageData, getCamera()->getColorSize().width, getCamera()->getColorSize().height, 3, colorIplImage->widthStep);
		Fl::lock();
		colorBox->image(colorFlImage);
		colorBox->redraw();
		Fl::wait();
		Fl::unlock();
		getComponent()->updateColor();
	}
	void updateDepth() {
		cv::Mat image = getCamera()->getDepthFrame(), normalizedImage;
		cv::normalize(image, normalizedImage, 0, 255, CV_MINMAX, CV_8UC3); 
		depthIplImage = cvCreateImage(cvSize(normalizedImage.cols, normalizedImage.rows), 8, 1);
		IplImage ipltemp = normalizedImage;
		cvCopy(&ipltemp, depthIplImage);
		depthFlImage = new Fl_RGB_Image((unsigned char*)depthIplImage->imageData, getCamera()->getDepthSize().width, getCamera()->getDepthSize().height, 1, depthIplImage->widthStep);
		Fl::lock();
		depthBox->image(depthFlImage);
		depthBox->redraw();
		Fl::wait();
		Fl::unlock();
		getComponent()->updateDepth();
	}
};