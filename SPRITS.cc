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

static bool stop = false;

static const char USAGE[] =
R"(Simple Pluggable Range Imaging Tracking Server

    Usage:
      SPRITS [(-c | --calibrate)] [(-dw | --debugwindow)] (-dm=s | --debugmessage=s)
      SPRITS (-h | --help)
      SPRITS --version

    Options:
      -h --help             Show this screen.
      --version             Show version.
      -c --calibrate        Enable calibration.
      -dw --debugwindow     Enable debug window.
      -dm --debugmessage=s  Print debug message every s seconds [default: 10].
)";

int main(int argc, char **argv)
{
	try
	{
		std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "SPRITS 1.0");
		signal(SIGINT, [](int nSig) { stop = true; });
		Camera* cam = new OpenNI(args["--calibrate"].asBool(), args["--debug"].asBool());
		CameraObserver<std::tuple<double, double, double>>* observer = new Debug3DTracker(cam, new Plane(), args["--debugmessage"].asLong(), NewFrameEvent::COLOR);
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
