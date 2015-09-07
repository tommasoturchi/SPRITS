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

#include <docopt.h>

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

static const char USAGE[] =
R"(Simple Pluggable Range Imaging Tracking Server

    Usage:
      SPRITS [(-c | --calibrate)] [(-d | --debug)]
      SPRITS (-h | --help)
      SPRITS --version

    Options:
      -h --help         Show this screen.
      --version         Show version.
      -c --calibrate    Enable calibration.
      -d --debug        Enable debug.
)";

int main(int argc, char **argv)
{
	try
	{
		std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "SPRITS 1.0");
		signal(SIGINT, [](int nSig) { stop = true; });
		Camera* cam = new OpenNI((bool)args["--calibrate"], (bool)args["--debug"]);
		CameraObserver<std::tuple<double, double, double>>* observer = new DebugTracker(cam, new Plane(), NewFrameEvent::COLOR);
		while (!stop)
			cam->update();
		delete observer;
		delete cam;
	} catch (std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
