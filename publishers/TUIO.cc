#ifndef TUIO_CC
#define TUIO_CC

#include <TUIO/TuioServer.h>
#include <TUIO/WebSockSender.h>
#include <spdlog/spdlog.h>

#include <Space.hpp>

using namespace SPRITS;

class TUIOPublisher : public SpaceObserver<std::tuple<double, double, double>>
{
private:
	TUIO::TuioServer *server;
	TUIO::TuioObject *objects[1024];
	TUIO::OscSender *ws_sender;
public:
	TUIOPublisher(Space<std::tuple<double, double, double>>* spc) : SpaceObserver<std::tuple<double, double, double>>(spc) {
		spdlog::get("console")->info("Starting TUIO Server...");
		server = new TUIO::TuioServer();
		ws_sender = new TUIO::WebSockSender(8080);
		server->addOscSender(ws_sender);
		server->setVerbose(false);
		spdlog::get("console")->info("TUIO Server started successfully!");
	}
	
	~TUIOPublisher()
	{
		spdlog::get("console")->info("Stopping TUIO Server...");
		delete server;
		delete ws_sender;
		spdlog::get("console")->info("TUIO Server stopped successfully!");
	}
	
	void fire(const ElementEvent& event, int id)
	{
		server->initFrame(TUIO::TuioTime::getSessionTime());
		switch (event.get_state()) {
			case ADD:
			objects[id] = server->addTuioObject(id, std::get<0>(spc_->getElement(id)), std::get<1>(spc_->getElement(id)), std::get<2>(spc_->getElement(id)));
			spdlog::get("console")->debug("[NEW TAG]: id {}", id);
			break;
			case UPDATE:
			server->updateTuioObject(objects[id], std::get<0>(spc_->getElement(id)), std::get<1>(spc_->getElement(id)), std::get<2>(spc_->getElement(id)));
			spdlog::get("console")->debug("[UPDATE TAG]: id {} {} {} {}", id, std::get<0>(spc_->getElement(id)), std::get<1>(spc_->getElement(id)), std::get<2>(spc_->getElement(id)));
			break;
			case REMOVE:
			server->removeTuioObject(objects[id]);
			objects[id] = NULL;
			spdlog::get("console")->debug("[REMOVE TAG]: id {}", id);
			break;
		}
		server->commitFrame();
	}
};

#endif