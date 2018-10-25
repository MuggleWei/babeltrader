#ifndef BABELTRADER_TRADE_SERVICE_H_
#define BABELTRADER_TRADE_SERVICE_H_

#include "uWS/uWS.h"
#include "rapidjson/document.h"

class TradeService
{
public:
	virtual void InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Value &msg) = 0;
	virtual void CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Value &msg) = 0;
};

#endif