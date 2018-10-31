#ifndef BABELTRADER_WS_SERVICE_H_
#define BABELTRADER_WS_SERVICE_H_

#include <functional>
#include <thread>
#include <set>

#include "uWS/uWS.h"
#include "rapidjson/document.h"
#include "muggle/cpp/tunnel/tunnel.hpp"

#include "common/quote_service.h"
#include "common/trade_service.h"

namespace babeltrader
{

class WsService
{
private:
	struct WsTunnelMsg
	{
		WsTunnelMsg(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &&doc)
			: ws_(ws)
			, doc_(std::move(doc))
		{}

		uWS::WebSocket<uWS::SERVER> *ws_;
		rapidjson::Document doc_;
	};

public:
	WsService(QuoteService *quote_service, TradeService *trade_service);

	void onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req);
	void onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length);
	void onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode);

	int PutMsg(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &&doc);

	void SendMsgToClient(uWS::WebSocket<uWS::SERVER> *ws, const char *msg);

private:
	void MessageLoop();

	void RegisterCallbacks();
	void Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

	void OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, int error_id, const char  *error_msg);
	void OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc, int error_id, const char  *error_msg);

	void OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

private:
	QuoteService *quote_;
	TradeService *trade_;

	std::map<std::string, std::function<void(uWS::WebSocket<uWS::SERVER>*, rapidjson::Document&)>> callbacks_;
	std::function<void(uWS::WebSocket<uWS::SERVER>*, rapidjson::Document&)> default_callback_;

	muggle::Tunnel<WsTunnelMsg> msg_tunnel_;

	std::mutex ws_mtx_;
	std::set<uWS::WebSocket<uWS::SERVER>*> ws_set_;
};


}

#endif