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

	void BroadcastConfirmOrder(uWS::Hub &hub, Order &order, int error_id, const char *error_msg);
	void BroadcastOrderStatus(uWS::Hub &hub, Order &order, OrderStatusNotify &order_status_notify, int error_id, const char *error_msg);
	void BroadcastOrderDeal(uWS::Hub &hub, Order &order, OrderDealNotify &order_deal);
	void RspOrderQry(uWS::WebSocket<uWS::SERVER>* ws, OrderQuery &order_qry, std::vector<Order> &orders, std::vector<OrderStatusNotify> &order_status, int error_id);
	void RspTradeQry(uWS::WebSocket<uWS::SERVER>* ws, TradeQuery &trade_qry, std::vector<Order> &orders, std::vector<OrderDealNotify> &order_deal, int error_id);

	void RspPositionQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType1> &positions, int error_id);
	void RspPositionDetailQryType1(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionDetailType1> &positions, int error_id);
	void RspTradeAccountQryType1(uWS::WebSocket<uWS::SERVER>* ws, TradeAccountQuery &tradeaccount_qry, std::vector<TradeAccountType1> &trade_accounts, int error_id);
	void RspProductQryType1(uWS::WebSocket<uWS::SERVER>* ws, ProductQuery &product_qry, std::vector<ProductType1> &product_types, int error_id);

	void RspPositionQryType2(uWS::WebSocket<uWS::SERVER>* ws, PositionQuery &position_qry, std::vector<PositionSummaryType2> &positions, int error_id);

private:
	void MessageLoop();

	void RegisterCallbacks();
	void Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

	void OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, int error_id, const char  *error_msg);
	void OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc, int error_id, const char  *error_msg);

	void OnReqInsertOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqCancelOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryOrder(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryTrade(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryPosition(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);
	void OnReqQueryProduct(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc);

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