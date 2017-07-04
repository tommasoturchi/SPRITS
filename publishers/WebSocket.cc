#ifndef WEBSOCKET_CC
#define WEBSOCKET_CC

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <spdlog/spdlog.h>

#include <json/json.h>

#include <Space.hpp>

using namespace SPRITS;

class WebSocketPublisher : public SpaceObserver<std::tuple<double, double, double>>
{
private:
	std::thread thread_;
	websocketpp::server<websocketpp::config::asio> server_;
    std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl>> connections_;
    std::mutex mutex_;
public:
	WebSocketPublisher(Space<std::tuple<double, double, double>>* spc, int port = 9002) : SpaceObserver<std::tuple<double, double, double>>(spc) {
		spdlog::get("console")->info("Starting WebSocket Server...");
		server_.clear_access_channels(websocketpp::log::alevel::all);
		server_.init_asio();
		server_.set_reuse_addr(true);
		server_.set_open_handler(std::bind<void>([this](websocketpp::connection_hdl hdl){ std::lock_guard<std::mutex> lock(mutex_); connections_.insert(hdl); }, std::placeholders::_1));
		server_.set_close_handler(std::bind<void>([this](websocketpp::connection_hdl hdl){ std::lock_guard<std::mutex> lock(mutex_); connections_.erase(hdl); }, std::placeholders::_1));
        server_.listen(port);
		server_.start_accept();
		thread_ = std::thread([this] { server_.run(); });
		spdlog::get("console")->info("WebSocket Server started successfully!");
	}
	
	~WebSocketPublisher()
	{
		spdlog::get("console")->info("Stopping WebSocket Server...");
		server_.stop();
		thread_.join();
		spdlog::get("console")->info("WebSocket Server stopped successfully!");
	}
	
	void fire(const ElementEvent& event, int id)
	{
		Json::Value message;
		message["id"] = id;
		message["op"] = event.get_state_as_string();
		message["pos"] = Json::Value();
		message["pos"].append(std::get<0>(spc_->getElement(id)));
		message["pos"].append(std::get<1>(spc_->getElement(id)));
		message["angle"] = std::get<2>(spc_->getElement(id));
		
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it : connections_)
            server_.send(it, Json::FastWriter().write(message), websocketpp::frame::opcode::text);

        spdlog::get("console")->debug("WebSocket message {} sent.", Json::FastWriter().write(message));
	}
};

#endif