#include <OpenNI.cc>
#include <Debug.cc>

#include <Carbon/Carbon.h>

class OpenNIObserver : public AbstractCameraObserver
{
public:
	OpenNIObserver(Camera *c) : AbstractCameraObserver(c) { }
	void updateColor() {
		return;
	}
	void updateDepth() {
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
		Camera* xt = new OpenNI();
		AbstractCameraObserverDecorator* sprits = new DebugStream(new FPS(new OpenNIObserver(xt), 50));
		while (!isPressed(0x24))
			continue;
		delete sprits;
		delete xt;
	} catch (std::exception& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	exit(0);
}
