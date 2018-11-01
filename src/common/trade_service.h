#ifndef BABELTRADER_TRADE_SERVICE_H_
#define BABELTRADER_TRADE_SERVICE_H_

#include "uWS/uWS.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common_struct.h"

namespace babeltrader
{


class TradeService
{
public:
	virtual void InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) = 0;
	virtual void CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) = 0;
	virtual void QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &query_order) = 0;
};


}

#endif