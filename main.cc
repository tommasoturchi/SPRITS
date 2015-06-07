#include <Xtion.cc>
#include <Debug.cc>

#include <Carbon/Carbon.h>

class XtionObserver : public AbstractCameraObserver
{
public:
	XtionObserver(Camera *c) : AbstractCameraObserver(c) { }
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
	Camera* xt = new Xtion();
	AbstractCameraObserverDecorator* sprits = new DebugStream(new FPS(new XtionObserver(xt), 50));
	while (!isPressed(0x24))
		continue;
	delete sprits;
	delete xt;
	return 0;
}
