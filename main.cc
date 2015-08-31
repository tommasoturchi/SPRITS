#include <cameras/OpenNI.cc>
#include <cameras/VideoStream.cc>
#include <trackers/Debug.cc>
#include <trackers/ChiliTracker.cc>
#include <trackers/FingerTracker.cc>
#include <manifolds/Plane.cc>
#include <publishers/WebSocket.cc>
#include <publishers/TUIO.cc>

#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <stdio.h>
#include <signal.h>

#include <Camera.hpp>
#include <Manifold.hpp>

using namespace SPRITS;

class DebugTracker : public Debug<std::tuple<double, double, double>>
{
public:
	DebugTracker(Camera *cam, Manifold<std::tuple<double, double, double>> *man, const NewFrameEvent& event) : Debug<std::tuple<double, double, double>>(cam, man, 10, event) { }
	
	template<typename E, typename... Events>
	DebugTracker(Camera *cam, Manifold<std::tuple<double, double, double>> *man, E event, Events... events) : Debug<std::tuple<double, double, double>>(cam, man, 10, event, events...) { }
};

static bool stop = false;

int main(int argc, char **argv)
{
	try
	{
		signal(SIGINT, [](int nSig) { stop = true; });
		Camera* cam = new OpenNI();
		Manifold<std::tuple<double, double, double>>* manifold = new Plane();
		//ManifoldObserver<std::tuple<double, double, double>>* publisher = new WebSocket(manifold);
		CameraObserver<std::tuple<double, double, double>>* guiobserver = new DebugWindow(cam, manifold);
		//CameraObserver<std::tuple<double, double, double>>* observer = new ChiliTracker(new DebugTracker(cam, manifold, NewFrameEvent::COLOR));
		//CameraObserver<std::tuple<double, double, double>>* fingerobserver = new FingerTracker(observer);
		while (!stop)
			cam->update();
		//delete observer;
		delete guiobserver;
		//delete publisher;
		delete manifold;
		delete cam;
	} catch (std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
