#include <docopt.h>
#include <OpenNI.h>

#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <stdio.h>
#include <signal.h>
#include <boost/lexical_cast.hpp>
#include <spdlog/spdlog.h>

#include <Camera.hpp>
#include <Space.hpp>

#include <cameras/OpenNI.cc>
#include <cameras/VideoStream.cc>
#include <trackers/Debug.cc>
#include <trackers/ChiliTracker.cc>
#include <trackers/FingerTracker.cc>
#include <spaces/Plane.cc>
#include <publishers/WebSocket.cc>
#include <publishers/TUIO.cc>

using namespace SPRITS;

static bool stop = false;

static const char USAGE[] =
R"(Simple Pluggable Range Imaging Tracking Server.

    Usage:
      SPRITS [options]
      SPRITS [options] (OpenNI|VideoStream)

    Options:
      --help               Show this screen.
      --crop               Crop camera image.
      --debug              Enable debug window.
      --record             Enable camera recording.
      --tuio               Enable TUIO publisher.
      --verbose            Enable verbose logging.
      --websocket=<port>   Enable Websocket publisher [default port: 9002].
      --version            Show version.
)";

int main(int argc, char **argv)
{
	auto console = spdlog::stdout_logger_mt("console", true);
	try
	{
		std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "SPRITS 1.0");
		console->set_level(args["--verbose"].asBool()?spdlog::level::debug:spdlog::level::info);
		signal(SIGINT, [](int nSig) { stop = true; });
		Camera* cam = args["OpenNI"].asBool()?(Camera*)new OpenNI(args["--crop"].asBool(), args["--debug"].asBool()):(Camera*)new VideoStream(args["--crop"].asBool(), args["--debug"].asBool());
		Space<std::tuple<double, double, double>>* spc = new Plane();
		CameraObserver<std::tuple<double, double, double>>* fpsobs = new ChiliTracker(new Debug3DTracker(cam, spc, args["--record"].asBool(), NewFrameEvent::COLOR));
		std::list<SpaceObserver<std::tuple<double, double, double>>*> publishers;
		if (args["--tuio"].asBool())
			publishers.push_back(new TUIOPublisher(spc));
		if ((args["--websocket"].isBool()) && (args["--websocket"].asBool()))
			publishers.push_back(new WebSocketPublisher(spc));
		else
			publishers.push_back(new WebSocketPublisher(spc, boost::lexical_cast<int>(args["--websocket"].asString())));
		while (!stop)
			cam->update();
		for (auto const& pub : publishers)
			delete pub;
		delete fpsobs;
		delete cam;
	} catch (std::exception& e)
	{
		spdlog::get("console")->critical("ERROR: {}", e.what());
	}
	spdlog::drop_all();
	exit(0);
}
