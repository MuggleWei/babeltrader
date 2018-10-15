#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

#include "uWS/uWS.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "muggle/muggle_cc.h"

#include "conf.h"
#include "ws_service.h"
#include "http_service.h"
#include "ctp_quote_spi.h"

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
	// load config
	CTPQuoteConf conf;
	auto ret = LoadConfig("./config/ctp_conf.json", conf);
	if (!ret) {
		return -1;
	}

	// init CTP api
	CThostFtdcMdApi *quote_api = CThostFtdcMdApi::CreateFtdcMdApi();
	std::cout << "CTP quotes API version:" << quote_api->GetApiVersion() << std::endl;

	char addr[256] = { 0 };
	strncpy(addr, conf.addr.c_str(), sizeof(addr) - 1);

	CTPQuoteHandler spi(quote_api, conf);
	quote_api->RegisterSpi(&spi);
	quote_api->RegisterFront(addr);
	quote_api->Init();

	// run service loop
	uWS::Hub h;
	auto loop_thread = std::thread([&h] {
		runService(h);
	});
	
	loop_thread.join();

	// quote_api->Join();
	// quote_api->Release();
	
	return 0;
}