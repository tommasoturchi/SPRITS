#include <docopt.h>
#include <OpenNI.h>

#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <stdio.h>
#include <signal.h>
#include <boost/lexical_cast.hpp>

#include <Camera.hpp>
#include <Manifold.hpp>

#include <cameras/OpenNI.cc>
#include <cameras/VideoStream.cc>
#include <trackers/Debug.cc>
#include <trackers/ChiliTracker.cc>
#include <trackers/FingerTracker.cc>
#include <manifolds/Plane.cc>
#include <publishers/WebSocket.cc>
#include <publishers/TUIO.cc>

using namespace SPRITS;

static bool stop = false;

static const char USAGE[] =
R"(Simple Pluggable Range Imaging Tracking Server

    Usage:
      SPRITS [options]

    Options:
      -h --help          Show this screen.
      -c --calibrate     Enable calibration.
      -d --debug         Enable debug window.
      -f S --fps=S       Print FPS message every S seconds [default: 10].
      -t --tuio          Enable TUIO output.
      -w --websocket     Enable Websocket output.
      -v --version       Show version.
)";

int main(int argc, char **argv)
{
	try
	{
		std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "SPRITS 1.0");
		signal(SIGINT, [](int nSig) { stop = true; });
		//Camera* cam = new OpenNI(args["--calibrate"].asBool(), args["--debug"].asBool());
		Camera* cam = new VideoStream();
		Manifold<std::tuple<double, double, double>>* man = new Plane();
		CameraObserver<std::tuple<double, double, double>>* fpsobs = new ChiliTracker(new Debug3DTracker(cam, man, boost::lexical_cast<int>(args["--fps"].asString()), NewFrameEvent::COLOR, NewFrameEvent::DEPTH));
		std::list<ManifoldObserver<std::tuple<double, double, double>>*> publishers;
		if (args["--tuio"].asBool())
			publishers.push_back(new TUIOPublisher(man));
		if (args["--websocket"].asBool())
			publishers.push_back(new WebSocketPublisher(man));
		while (!stop)
			cam->update();
		for (auto const& pub : publishers)
			delete pub;
		delete fpsobs;
		delete cam;
	} catch (std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
