#ifndef WEBSOCKET_CC
#define WEBSOCKET_CC

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <json/json.h>

#include <Manifold.hpp>

using namespace SPRITS;

class WebSocket : public ManifoldObserver<std::tuple<double, double, double>>
{
private:
	std::thread thread_;
public:
	WebSocket(Manifold<std::tuple<double, double, double>>* man) : ManifoldObserver<std::tuple<double, double, double>>(man) {
		std::cout << "Starting WebSocket Server... ";
		thread_ = std::thread([this] { this->start(); });
		std::cout << "OK!" << std::endl;
	}
	
	~WebSocket()
	{
		thread_.join();
	}
	
	void start()
	{
		websocketpp::server<websocketpp::config::asio> server_;
		server_.init_asio();
		server_.listen(9002);
		server_.start_accept();
		server_.run();
		std::cout << "test" << std::endl;
	}
	
	void fire(const ElementEvent& event, int id)
	{
		Json::Value message;
		message["id"] = id;
		switch (event)
		{
			case ElementEvent::ADD:
			message["op"] = "ADD";
			break;
			case ElementEvent::UPDATE:
			message["op"] = "UPDATE";
			break;
			case ElementEvent::REMOVE:
			message["op"] = "REMOVE";
			break;
		}
		Json::FastWriter fastWriter;
		//for (auto conn : server_.get_connections()) server_.send(conn, fastWriter.write(message));
		std::cout << fastWriter.write(message) << std::endl;
	}
};

#endif