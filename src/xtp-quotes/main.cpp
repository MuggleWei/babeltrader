#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

#include "uWS/uWS.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "ws_service.h"
#include "http_service.h"

void runService(uWS::Hub &h)
{
	WsService ws_service;
	HttpService http_service;

	// ws
	h.onConnection([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
		ws_service.onConnection(ws, req);
	});
	h.onMessage([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
		ws_service.onMessage(ws, message, length, opCode);
	});
	h.onDisconnection([&ws_service](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	});

	// rest
	h.onHttpRequest([&http_service](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
		http_service.onMessage(res, req, data, length, remainingBytes);
	});

	if (!h.listen("127.0.0.1", 3000, nullptr, uS::ListenOptions::REUSE_PORT)) {
		std::cout << "Failed to listen" << std::endl;
	}
	h.run();
}

int main(int argc, char *argv[])
{
	uWS::Hub h;
	auto loop_thread = std::thread([&h] {
		runService(h);
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	auto broadcast_thread = std::thread([&h] {
		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		writer.StartObject();
		writer.Key("msg");
		writer.String("notify");

		writer.Key("data");
		writer.StartObject();
		writer.Key("msg");
		writer.String("it's uWebSockets");
		writer.EndObject();

		writer.EndObject();

		std::string msg = s.GetString();
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << msg << std::endl;
			h.getDefaultGroup<uWS::SERVER>().broadcast(msg.c_str(), msg.size(), uWS::OpCode::TEXT);
		}
	});

	loop_thread.join();
	broadcast_thread.join();	

	return 0;
}