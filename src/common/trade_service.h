#ifndef BABELTRADER_TRADE_SERVICE_H_
#define BABELTRADER_TRADE_SERVICE_H_

#include "uWS/uWS.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common_struct.h"

namespace babeltrader
{


class WsService;

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
	virtual void QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &query_product) { throw std::runtime_error("'QueryProduct' not implement"); }

public:
	// request
	void OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryTrade(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryPosition(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryProduct(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

	// response
	void BroadcastConfirmOrder(Order &order, int error_id, const char *error_msg);
	void BroadcastOrderStatus(Order &order, OrderStatusNotify &order_status_notify, int error_id, const char *error_msg);
	void BroadcastOrderDeal(Order &order, OrderDealNotify &order_deal);
	void RspOrderQry(uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry, std::vector<Order> &orders, std::vector<OrderStatusNotify> &order_status, int error_id);
	void RspTradeQry(uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry, std::vector<Order> &orders, std::vector<OrderDealNotify> &order_deal, int error_id);

	void RspPositionQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType1> &positions, int error_id);
	void RspPositionDetailQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionDetailType1> &positions, int error_id);
	void RspTradeAccountQryType1(uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry, std::vector<TradeAccountType1> &trade_accounts, int error_id);
	void RspProductQryType1(uWS::WebSocket<uWS::SERVER>* ws, ProductQuery &product_qry, std::vector<ProductType1> &product_types, int error_id);

	void RspPositionQryType2(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType2> &positions, int error_id);
	void RspTradeAccountQryType2(uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry, std::vector<TradeAccountType2> &trade_accounts, int error_id);

public:
	uWS::Hub uws_hub_;
	WsService *ws_service_;
};


}

#endif