#ifndef BABELTRADER_WS_SERVICE_H_
#define BABELTRADER_WS_SERVICE_H_

#include "uWS/uWS.h"
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
	QuoteService *quote_;
	TradeService *trade_;
};

#endif