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
	websocketpp::server<websocketpp::config::asio> server_;
    std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl>> connections_;
    std::mutex mutex_;
public:
	WebSocket(Manifold<std::tuple<double, double, double>>* man) : ManifoldObserver<std::tuple<double, double, double>>(man) {
		std::cout << "Starting WebSocket Server... ";
		server_.clear_access_channels(websocketpp::log::alevel::all);
		server_.init_asio();
		server_.set_reuse_addr(true);
		server_.set_open_handler(std::bind<void>([this](websocketpp::connection_hdl hdl){ std::lock_guard<std::mutex> lock(mutex_); connections_.insert(hdl); }, std::placeholders::_1));
		server_.set_close_handler(std::bind<void>([this](websocketpp::connection_hdl hdl){ std::lock_guard<std::mutex> lock(mutex_); connections_.erase(hdl); }, std::placeholders::_1));
        server_.listen(9002);
		server_.start_accept();
		thread_ = std::thread([this] {
			server_.run();
		});
		std::cout << "OK!" << std::endl;
	}
	
	~WebSocket()
	{
		std::cout << "Stopping WebSocket Server... ";
		server_.stop();
		thread_.join();
		std::cout << "OK!" << std::endl;
	}
	
	void fire(const ElementEvent& event, int id)
	{
		Json::Value message;
		message["id"] = id;
		message["op"] = event.get_state_as_string();
		message["pos"] = Json::Value();
		message["pos"].append(std::get<0>(man_->getElement(id)));
		message["pos"].append(std::get<1>(man_->getElement(id)));
		message["angle"] = std::get<2>(man_->getElement(id));
		
        std::lock_guard<std::mutex> lock(mutex_);    
        for (auto it : connections_)
            server_.send(it, Json::FastWriter().write(message), websocketpp::frame::opcode::text);
	}
};

#endif