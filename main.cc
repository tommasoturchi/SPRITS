#include <plugins/OpenNI.cc>
#include <plugins/Debug.cc>

#include <Carbon/Carbon.h>

#include <map>

#include <Camera.hpp>

class MapSurface : public SPRITS::Surface {
	std::map<int, cv::Point> sur;
public:
	MapSurface() { }
	int size() {
		return sur.size();
	}
	cv::Point getPosition(int id) {
		return sur[id];
	}
	void setPosition(int id, cv::Point pos) {
		sur[id] = pos;
	}
};

class MyCameraColorObserver : public SPRITS::AbstractCameraColorObserver {
public:
	MyCameraColorObserver(SPRITS::Camera *c, SPRITS::Surface *s) : AbstractCameraColorObserver(c, s) { }
	void update() {
		return;
	}
};

Boolean isPressed( unsigned short inKeyCode )
{
	unsigned char keyMap[16];
	GetKeys((BigEndianUInt32*) &keyMap);
	return (0 != ((keyMap[ inKeyCode >> 3] >> (inKeyCode & 7)) & 1));
}
	
int main(int argc, char **argv)
{
	try {
		SPRITS::Camera* xt = new OpenNI();
		Surface* s = new MapSurface();
		AbstractCameraColorObserverDecorator* sprits = new ColorDebugStream(new ColorFPS(new MyCameraColorObserver(xt, s), 50), 320, 240);
		while (!isPressed(0x24))
			continue;
		delete sprits;
		delete s;
		delete xt;
	} catch (std::exception& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
