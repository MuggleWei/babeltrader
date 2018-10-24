#ifndef BABELTRADER_WS_SERVICE_H_
#define BABELTRADER_WS_SERVICE_H_

#include <functional>

#include "uWS/uWS.h"
#include "rapidjson/document.h"

#include "common/quote_service.h"
#include "common/trade_service.h"

class WsService
{
public:
	WsService(QuoteService *quote_service, TradeService *trade_service);

	void onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req);
	void onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length);
	void onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode);

private:
	void RegisterCallbacks();
	void Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

	void OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, int error_id, const char  *error_msg);

	void OnReqDefault(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

private:
	QuoteService *quote_;
	TradeService *trade_;

	std::map<std::string, std::function<void(uWS::WebSocket<uWS::SERVER>*, rapidjson::Document&)>> callbacks_;
	std::function<void(uWS::WebSocket<uWS::SERVER>*, rapidjson::Document&)> default_callback_;
};

#endif