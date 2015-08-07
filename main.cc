#include <cameras/OpenNI.cc>
#include <cameras/VideoStream.cc>
#include <trackers/Debug.cc>
#include <trackers/ChiliTracker.cc>
#include <manifolds/Plane.cc>
#include <publishers/WebSocket.cc>

#include <Carbon/Carbon.h>

#include <functional>
#include <map>
#include <vector>
#include <utility>

#include <Camera.hpp>
#include <Manifold.hpp>

using namespace SPRITS;

class DebugTracker : public Debug<std::tuple<double, double, double>>
{
public:
	DebugTracker(Camera *cam, Manifold<std::tuple<double, double, double>> *man, const NewFrameEvent& event) : Debug<std::tuple<double, double, double>>(cam, man, 1, event) { }
	
	template<typename E, typename... Events>
	DebugTracker(Camera *cam, Manifold<std::tuple<double, double, double>> *man, E event, Events... events) : Debug<std::tuple<double, double, double>>(cam, man, 1, event, events...) { }
};

Boolean isPressed( unsigned short inKeyCode )
{
	unsigned char keyMap[16];
	GetKeys((BigEndianUInt32*) &keyMap);
	return (0 != ((keyMap[ inKeyCode >> 3] >> (inKeyCode & 7)) & 1));
}
	
int main(int argc, char **argv)
{
	try
	{
		Camera* cam = new OpenNI();
		//Camera* cam = new VideoStream();
		Manifold<std::tuple<double, double, double>>* man = new Plane();
		ManifoldObserver<std::tuple<double, double, double>>* mo = new WebSocket(man);
		CameraObserver<std::tuple<double, double, double>>* co = new ChiliTracker(new DebugTracker(cam, man, NewFrameEvent::COLOR));
		while (!isPressed(0x35))
			cam->update();
		delete mo;
		delete co;
		delete cam;
	} catch (std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
