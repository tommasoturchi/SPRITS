#ifndef TUIO_CC
#define TUIO_CC

#include <TUIO/TuioServer.h>
#include <TUIO/WebSockSender.h>

#include <Manifold.hpp>

using namespace SPRITS;

class TUIOPublisher : public ManifoldObserver<std::tuple<double, double, double>>
{
private:
	TUIO::TuioServer *server;
	TUIO::TuioObject *objects[1024];
	TUIO::OscSender *ws_sender;
public:
	TUIOPublisher(Manifold<std::tuple<double, double, double>>* man) : ManifoldObserver<std::tuple<double, double, double>>(man) {
		std::cout << "Starting TUIO Server... OK!" << std::endl;
		server = new TUIO::TuioServer();
	    ws_sender = new TUIO::WebSockSender(8080);
	   	server->addOscSender(ws_sender);
	}
	
	~TUIOPublisher()
	{
		delete server;
		delete ws_sender;
	}
	
	void fire(const ElementEvent& event, int id)
	{
		server->initFrame(TUIO::TuioTime::getSessionTime());
		switch (event.get_state()) {
			case ADD:
			objects[id] = server->addTuioObject(id, std::get<0>(man_->getElement(id)), std::get<1>(man_->getElement(id)), std::get<2>(man_->getElement(id)));
			std::cout << "[NEW TAG]: id " << id << std::endl;
			break;
			case UPDATE:
			server->updateTuioObject(objects[id], std::get<0>(man_->getElement(id)), std::get<1>(man_->getElement(id)), std::get<2>(man_->getElement(id)));
			std::cout << "[UPDATE TAG]: id " << id << " " << std::get<0>(man_->getElement(id)) << " " << std::get<1>(man_->getElement(id)) << " " << std::get<2>(man_->getElement(id)) << std::endl;
			break;
			case REMOVE:
			server->removeTuioObject(objects[id]);
			objects[id] = NULL;
			std::cout << "[REMOVE TAG]: id " << id << std::endl;
			break;
		}
		server->commitFrame();
	}
};

#endif