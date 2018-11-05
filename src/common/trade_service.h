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
	virtual void InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) { throw std::runtime_error("'InsertOrder' not implement"); }
	virtual void CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) { throw std::runtime_error("'CancelOrder' not implement"); }
	virtual void QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &query_order) { throw std::runtime_error("'QueryOrder' not implement"); }
	virtual void QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &query_order) { throw std::runtime_error("'QueryTrade' not implement"); }
	virtual void QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &query_position) { throw std::runtime_error("'QueryPosition' not implement"); }
	virtual void QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &query_position) { throw std::runtime_error("'QueryPositionDetail' not implement"); }
	virtual void QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &query_tradeaccount) { throw std::runtime_error("'QueryTradeAccount' not implement"); }
};


}

#endif